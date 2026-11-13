#include "struct_marshal.h"

#include <climits>
#include "alloc/general_allocation.h"
#include "gc/gc_roots.h"
#include "metadata/module_def.h"
#include "utils/encode_conv.h"
#include "utils/hashmap.h"
#include "utils/rt_vector.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/marshal.h"
#include "vm/object.h"
#include "vm/runtime.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace vm
{
namespace
{
using metadata::RtClass;
using metadata::RtElementType;
using metadata::RtMarshalNativeType;
struct NativeLayout;
enum class Kind : uint8_t
{
    Copy,
    Boolean,
    VariantBoolean,
    AnsiChar,
    Structure,
    String,
    FixedString,
    FixedArray
};
enum class Encoding : uint8_t
{
    Ansi,
    Unicode,
    Utf8,
    BStr
};

struct ValuePlan
{
    Kind kind;
    Encoding encoding;
    uint32_t size;
    uint32_t alignment;
    uint32_t managed_size;
    uint32_t count;
    const RtClass* klass;
    const NativeLayout* nested;
};

struct FieldPlan
{
    const metadata::RtFieldInfo* field;
    uint32_t native_offset;
    ValuePlan value;
    ValuePlan element;
};

struct NativeLayout
{
    utils::Vector<FieldPlan> fields;
    ValuePlan value{};
    RtErr error = RtErr::None;
    bool building = true;
    bool identity = false;
    bool resources = false;
    mutable bool constructor_resolved = false;
    mutable const metadata::RtMethodInfo* default_constructor = nullptr;
};

// Metadata has process lifetime. Nodes (and references to them) survive rehash.
// Like the runtime metadata caches, access is on the runtime owner thread.
utils::HashMap<const RtClass*, NativeLayout>& layouts()
{
    static utils::HashMap<const RtClass*, NativeLayout> cache;
    return cache;
}

class ObjectRoot
{
    RtObject* _object;

  public:
    explicit ObjectRoot(RtObject* obj) : _object(obj)
    {
        gc::GcRoots::register_slot(&_object);
    }

    ~ObjectRoot()
    {
        gc::GcRoots::unregister_slot(&_object);
    }
};

struct Buffer
{
    uint8_t* data;

    explicit Buffer(uint64_t size) : data(size <= SIZE_MAX ? static_cast<uint8_t*>(alloc::GeneralAllocation::calloc(static_cast<size_t>(size), 1)) : nullptr)
    {
    }

    ~Buffer()
    {
        alloc::GeneralAllocation::free(data);
    }
};

template <class T>
T load(const void* ptr)
{
    T value;
    std::memcpy(&value, ptr, sizeof(T));
    return value;
}

template <class T>
void store(void* ptr, T value)
{
    std::memcpy(ptr, &value, sizeof(T));
}

Encoding charset(const RtClass* klass)
{
    const auto format = klass->flags & static_cast<uint32_t>(metadata::RtTypeAttribute::StringFormatMask);
    if (format == static_cast<uint32_t>(metadata::RtTypeAttribute::UnicodeClass))
        return Encoding::Unicode;
#if LEANCLR_PLATFORM_WIN
    if (format == static_cast<uint32_t>(metadata::RtTypeAttribute::AutoClass))
        return Encoding::Unicode;
#endif
    return Encoding::Ansi;
}

bool has_resources(const ValuePlan& v)
{
    return v.kind == Kind::String || (v.kind == Kind::Structure && v.nested->resources);
}

bool is_identity(const ValuePlan& v)
{
    return v.size == v.managed_size && (v.kind == Kind::Copy || (v.kind == Kind::Structure && v.nested->identity));
}

RtResult<const NativeLayout*> get_layout(const RtClass* klass, unsigned depth = 0);

RtResultVoid make_value(const metadata::RtTypeSig* type, RtMarshalNativeType native, Encoding encoding, ValuePlan& v, unsigned depth)
{
    if (type->by_ref || depth > 64)
        RET_ERR(RtErr::Argument);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, klass, Class::get_class_from_typesig(type));
    RET_ERR_ON_FAIL(Class::initialize_fields(klass));
    v = {};
    v.kind = Kind::Copy;
    v.encoding = encoding;
    v.klass = klass;
    v.managed_size = Class::is_reference_type(klass) ? static_cast<uint32_t>(PTR_SIZE) : klass->instance_size_without_header;
    auto element = Class::is_enum_type(klass) ? Class::get_enum_element_type(klass) : type->ele_type;
    const bool default_native = native == RtMarshalNativeType::NotInitialized;
    switch (element)
    {
    case RtElementType::Boolean:
        v.kind = Kind::Boolean;
        if (default_native || native == RtMarshalNativeType::Boolean)
            v.size = 4;
        else if (native == RtMarshalNativeType::I1 || native == RtMarshalNativeType::U1)
            v.size = 1;
        else if (native == RtMarshalNativeType::VariantBool)
        {
            v.kind = Kind::VariantBoolean;
            v.size = 2;
        }
        else
            RET_ERR(RtErr::Argument);
        break;
    case RtElementType::Char:
        if ((default_native && encoding != Encoding::Unicode) || native == RtMarshalNativeType::I1 || native == RtMarshalNativeType::U1)
        {
            v.kind = Kind::AnsiChar;
            v.size = 1;
        }
        else if (default_native || native == RtMarshalNativeType::I2 || native == RtMarshalNativeType::U2)
            v.size = 2;
        else
            RET_ERR(RtErr::Argument);
        break;
    case RtElementType::I1:
    case RtElementType::U1:
        if (!default_native && native != RtMarshalNativeType::I1 && native != RtMarshalNativeType::U1)
            RET_ERR(RtErr::Argument);
        v.size = 1;
        break;
    case RtElementType::I2:
    case RtElementType::U2:
        if (!default_native && native != RtMarshalNativeType::I2 && native != RtMarshalNativeType::U2)
            RET_ERR(RtErr::Argument);
        v.size = 2;
        break;
    case RtElementType::I4:
    case RtElementType::U4:
        if (!default_native && native != RtMarshalNativeType::I4 && native != RtMarshalNativeType::U4 && native != RtMarshalNativeType::Error)
            RET_ERR(RtErr::Argument);
        v.size = 4;
        break;
    case RtElementType::I8:
    case RtElementType::U8:
        if (!default_native && native != RtMarshalNativeType::I8 && native != RtMarshalNativeType::U8)
            RET_ERR(RtErr::Argument);
        v.size = 8;
        break;
    case RtElementType::R4:
        if (!default_native && native != RtMarshalNativeType::R4)
            RET_ERR(RtErr::Argument);
        v.size = 4;
        break;
    case RtElementType::R8:
        if (!default_native && native != RtMarshalNativeType::R8)
            RET_ERR(RtErr::Argument);
        v.size = 8;
        break;
    case RtElementType::I:
    case RtElementType::U:
    case RtElementType::Ptr:
    case RtElementType::FnPtr:
        if (!default_native && native != RtMarshalNativeType::Int && native != RtMarshalNativeType::UInt && native != RtMarshalNativeType::Ptr)
            RET_ERR(RtErr::Argument);
        v.size = static_cast<uint32_t>(PTR_SIZE);
        break;
    case RtElementType::String:
        v.kind = Kind::String;
        if (native == RtMarshalNativeType::LPWStr)
            v.encoding = Encoding::Unicode;
        else if (native == RtMarshalNativeType::LPStr)
            v.encoding = Encoding::Ansi;
        else if (native == RtMarshalNativeType::LPUTF8Str)
            v.encoding = Encoding::Utf8;
        else if (native == RtMarshalNativeType::BStr)
            v.encoding = Encoding::BStr;
        else if (native == RtMarshalNativeType::FixedSysString)
            v.kind = Kind::FixedString;
        else if (!default_native && native != RtMarshalNativeType::LPTStr)
            RET_ERR(RtErr::NotSupported);
        v.size = static_cast<uint32_t>(PTR_SIZE);
        break;
    default:
        if (!Class::is_value_type(klass))
            RET_ERR(RtErr::Argument);
        if (!default_native && native != RtMarshalNativeType::Struct && native != RtMarshalNativeType::NestedStruct)
            RET_ERR(RtErr::NotSupported);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const NativeLayout*, nested, get_layout(klass, depth + 1));
        v.kind = Kind::Structure;
        v.nested = nested;
        v.size = nested->value.size;
        v.alignment = nested->value.alignment;
        RET_VOID_OK();
    }
    v.alignment = v.size;
    RET_VOID_OK();
}

RtResultVoid build_layout(const RtClass* klass, NativeLayout& result, unsigned depth)
{
    if (depth > 64)
        RET_ERR(RtErr::Argument);
    RET_ERR_ON_FAIL(Class::initialize_fields(const_cast<RtClass*>(klass)));
    if (klass->generic_container || klass->by_val->ele_type == RtElementType::GenericInst || Class::is_nullable_type(klass))
        RET_ERR(RtErr::Argument);
    if (Class::is_enum_type(klass))
        RET_ERR(RtErr::Argument); // Enum fields are supported, standalone enum layouts are not.
    auto element = klass->by_val->ele_type;
    if ((element >= RtElementType::Boolean && element <= RtElementType::R8) || element == RtElementType::I || element == RtElementType::U ||
        element == RtElementType::Ptr)
    {
        RET_ERR_ON_FAIL(make_value(klass->by_val, RtMarshalNativeType::NotInitialized, Encoding::Ansi, result.value, depth));
        result.identity = is_identity(result.value);
        RET_VOID_OK();
    }
    const auto layout_kind = klass->flags & static_cast<uint32_t>(metadata::RtTypeAttribute::LayoutMask);
    if (layout_kind != static_cast<uint32_t>(metadata::RtTypeAttribute::SequentialLayout) &&
        layout_kind != static_cast<uint32_t>(metadata::RtTypeAttribute::ExplicitLayout))
        RET_ERR(RtErr::Argument);
    const bool explicit_layout = Class::is_explicit_layout(klass);
    const auto class_layout = klass->image->get_class_layout_data(klass->token);
    const uint32_t packing = class_layout && class_layout->packing ? class_layout->packing : 8;
    if (packing > 128 || (packing & (packing - 1)))
        RET_ERR(RtErr::TypeLoad);
    uint32_t end = 0, alignment = 1;
    bool identity = !Class::get_has_references(klass);
    const auto& corlib = Class::get_corlib_types();
    if (klass->parent && klass->parent != corlib.cls_object && klass->parent != corlib.cls_valuetype)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const NativeLayout*, parent, get_layout(klass->parent, depth + 1));
        for (const auto& f : parent->fields)
            result.fields.push_back(f);
        end = parent->value.size;
        alignment = parent->value.alignment;
        identity = identity && parent->identity;
        result.resources = parent->resources;
    }
    for (uint16_t i = 0; i < klass->field_count; ++i)
    {
        const auto* field = klass->fields + i;
        if (!Field::is_instance(field))
            continue;
        FieldPlan f{};
        f.field = field;
        metadata::RtMarshalSpec spec{};
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, has_spec, Marshal::get_marshal_spec(field, spec));
        auto native = has_spec ? spec.native_type : RtMarshalNativeType::NotInitialized;
        if (field->type_sig->ele_type == RtElementType::SZArray && native == RtMarshalNativeType::FixedArray)
        {
            f.value.kind = Kind::FixedArray;
            f.value.managed_size = static_cast<uint32_t>(PTR_SIZE);
            f.value.count = spec.fixed_array.size;
            if (!f.value.count || f.value.count > INT_MAX)
                RET_ERR(RtErr::Argument);
            RET_ERR_ON_FAIL(make_value(field->type_sig->data.element_type, spec.fixed_array.array_element_type, charset(klass), f.element, depth + 1));
            if (f.element.kind == Kind::FixedString)
                RET_ERR(RtErr::Argument);
            uint64_t size = uint64_t(f.element.size) * f.value.count;
            if (size > INT_MAX)
                RET_ERR(RtErr::Argument);
            f.value.size = static_cast<uint32_t>(size);
            f.value.alignment = f.element.alignment;
        }
        else
        {
            RET_ERR_ON_FAIL(make_value(field->type_sig, native, charset(klass), f.value, depth + 1));
            if (f.value.kind == Kind::FixedString)
            {
                f.value.count = spec.fixed_sys_string.size;
                uint64_t size = uint64_t(f.value.count) * (f.value.encoding == Encoding::Unicode ? 2 : 1);
                if (!f.value.count || size > INT_MAX)
                    RET_ERR(RtErr::Argument);
                f.value.size = static_cast<uint32_t>(size);
                f.value.alignment = f.value.encoding == Encoding::Unicode ? 2 : 1;
            }
        }
        const uint32_t align = std::min(packing, f.value.alignment);
        alignment = std::max(alignment, align);
        uint64_t offset = (uint64_t(end) + align - 1) & ~(uint64_t(align) - 1);
        if (explicit_layout)
        {
            auto declared_offset = klass->image->get_field_offset(field->token);
            if (!declared_offset)
                RET_ERR(RtErr::TypeLoad);
            offset = declared_offset.value();
        }
        if (offset + f.value.size > INT_MAX)
            RET_ERR(RtErr::Argument);
        f.native_offset = static_cast<uint32_t>(offset);
        end = std::max(end, f.native_offset + f.value.size);
        identity = identity && is_identity(f.value) && f.native_offset == field->offset;
        result.resources = result.resources || has_resources(f.value) || (f.value.kind == Kind::FixedArray && has_resources(f.element));
        result.fields.push_back(f);
    }
    // Aliased native reference-owning fields would have ambiguous ownership.
    // Explicit blittable unions are supported; overlapping conversions are not.
    if (explicit_layout && !identity)
        for (size_t i = 0; i < result.fields.size(); ++i)
            for (size_t j = 0; j < i; ++j)
            {
                const auto& a = result.fields[i];
                const auto& b = result.fields[j];
                if (uint64_t(a.native_offset) < uint64_t(b.native_offset) + b.value.size &&
                    uint64_t(b.native_offset) < uint64_t(a.native_offset) + a.value.size)
                    RET_ERR(RtErr::Argument);
            }
    uint64_t size = (uint64_t(end) + alignment - 1) & ~(uint64_t(alignment) - 1);
    if (class_layout)
        size = std::max(size, uint64_t(class_layout->size));
    size = std::max(size, uint64_t(1));
    if (size > INT_MAX)
        RET_ERR(RtErr::Argument);
    result.value = {Kind::Structure, charset(klass), static_cast<uint32_t>(size), alignment, klass->instance_size_without_header, 0, klass, &result};
    result.identity = identity && size == klass->instance_size_without_header;
    RET_VOID_OK();
}

RtResult<const NativeLayout*> get_layout(const RtClass* klass, unsigned depth)
{
    if (!klass)
        RET_ERR(RtErr::ArgumentNull);
    auto inserted = layouts().try_emplace(klass);
    auto& entry = inserted.first->second;
    if (inserted.second)
    {
        auto built = build_layout(klass, entry, depth);
        entry.error = built.is_err() ? built.unwrap_err() : RtErr::None;
        entry.building = false;
        // These errors carry transient allocation/exception state and must not be cached.
        if (entry.error == RtErr::OutOfMemory || entry.error == RtErr::ManagedException)
        {
            const auto error = entry.error;
            layouts().erase(klass);
            RET_ERR(error);
        }
    }
    if (entry.building)
        RET_ERR(RtErr::Argument);
    if (entry.error != RtErr::None)
        RET_ERR(entry.error);
    RET_OK(&entry);
}

void destroy_value(const ValuePlan& plan, uint8_t* native);

void destroy_layout(const NativeLayout& plan, uint8_t* native)
{
    if (!plan.resources)
        return;
    for (const auto& f : plan.fields)
    {
        auto* p = native + f.native_offset;
        if (f.value.kind == Kind::FixedArray)
            for (uint32_t i = 0; i < f.value.count; ++i)
                destroy_value(f.element, p + i * f.element.size);
        else
            destroy_value(f.value, p);
    }
}

void destroy_value(const ValuePlan& plan, uint8_t* native)
{
    if (plan.kind == Kind::String)
    {
        auto* ptr = load<uint8_t*>(native);
        if (ptr)
            alloc::GeneralAllocation::free(plan.encoding == Encoding::BStr ? ptr - 4 : ptr);
        store<void*>(native, nullptr);
    }
    else if (plan.kind == Kind::Structure)
        destroy_layout(*plan.nested, native);
}

RtResultVoid write_value(const ValuePlan& plan, const uint8_t* managed, uint8_t* native);
RtResultVoid read_value(const ValuePlan& plan, const uint8_t* native, uint8_t* managed);

RtResultVoid write_layout(const NativeLayout& plan, const uint8_t* managed, uint8_t* native)
{
    if (plan.identity)
    {
        std::memcpy(native, managed, plan.value.size);
        RET_VOID_OK();
    }
    for (const auto& f : plan.fields)
    {
        const auto* m = managed + f.field->offset;
        auto* n = native + f.native_offset;
        if (f.value.kind != Kind::FixedArray)
        {
            RET_ERR_ON_FAIL(write_value(f.value, m, n));
            continue;
        }
        auto* arr = load<RtArray*>(m);
        if (!arr || Array::get_array_length(arr) < static_cast<int32_t>(f.value.count))
            RET_ERR(RtErr::Argument);
        const auto* a = static_cast<const uint8_t*>(Array::get_array_data_start_as_ptr_void(arr));
        for (uint32_t i = 0; i < f.value.count; ++i)
            RET_ERR_ON_FAIL(write_value(f.element, a + i * f.element.managed_size, n + i * f.element.size));
    }
    RET_VOID_OK();
}

RtResultVoid read_layout(const NativeLayout& plan, const uint8_t* native, uint8_t* managed)
{
    if (plan.identity)
    {
        std::memcpy(managed, native, plan.value.size);
        RET_VOID_OK();
    }
    for (const auto& f : plan.fields)
    {
        auto* m = managed + f.field->offset;
        const auto* n = native + f.native_offset;
        if (f.value.kind != Kind::FixedArray)
        {
            RET_ERR_ON_FAIL(read_value(f.value, n, m));
            continue;
        }
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
            RtArray*, arr, LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(f.element.klass, static_cast<int32_t>(f.value.count), "StructMarshal::read_layout"));
        gc::GarbageCollector::write_barrier(reinterpret_cast<RtObject**>(m), arr);
        auto* a = static_cast<uint8_t*>(Array::get_array_data_start_as_ptr_void(arr));
        for (uint32_t i = 0; i < f.value.count; ++i)
            RET_ERR_ON_FAIL(read_value(f.element, n + i * f.element.size, a + i * f.element.managed_size));
    }
    RET_VOID_OK();
}

// Bound conversion by the supplied lengths, including truncated native UTF-8
// and unpaired managed surrogates. The general unchecked converters cannot be
// used for a fixed native field, which may end in a partial multibyte sequence.
void encode_utf8(const Utf16Char* chars, size_t length, uint8_t* output, size_t& bytes)
{
    bytes = 0;
    for (size_t i = 0; i < length; ++i)
    {
        uint32_t c = chars[i];
        if (c >= 0xD800 && c <= 0xDBFF)
        {
            if (i + 1 < length && chars[i + 1] >= 0xDC00 && chars[i + 1] <= 0xDFFF)
                c = 0x10000 + ((c - 0xD800) << 10) + (chars[++i] - 0xDC00);
            else
                c = 0xFFFD;
        }
        else if (c >= 0xDC00 && c <= 0xDFFF)
            c = 0xFFFD;
        if (c < 0x80)
            output[bytes++] = static_cast<uint8_t>(c);
        else if (c < 0x800)
        {
            output[bytes++] = static_cast<uint8_t>(0xC0 | (c >> 6));
            output[bytes++] = static_cast<uint8_t>(0x80 | (c & 63));
        }
        else if (c < 0x10000)
        {
            output[bytes++] = static_cast<uint8_t>(0xE0 | (c >> 12));
            output[bytes++] = static_cast<uint8_t>(0x80 | ((c >> 6) & 63));
            output[bytes++] = static_cast<uint8_t>(0x80 | (c & 63));
        }
        else
        {
            output[bytes++] = static_cast<uint8_t>(0xF0 | (c >> 18));
            output[bytes++] = static_cast<uint8_t>(0x80 | ((c >> 12) & 63));
            output[bytes++] = static_cast<uint8_t>(0x80 | ((c >> 6) & 63));
            output[bytes++] = static_cast<uint8_t>(0x80 | (c & 63));
        }
    }
    output[bytes] = 0;
}

void decode_utf8(const uint8_t* bytes, size_t length, Utf16Char* output, size_t& chars)
{
    chars = 0;
    for (size_t i = 0; i < length;)
    {
        uint32_t c = bytes[i++];
        if (c < 0x80)
        {
            output[chars++] = static_cast<Utf16Char>(c);
            continue;
        }
        const unsigned continuation = c >= 0xC2 && c <= 0xDF ? 1 : c >= 0xE0 && c <= 0xEF ? 2 : c >= 0xF0 && c <= 0xF4 ? 3 : 0;
        if (!continuation)
        {
            output[chars++] = 0xFFFD;
            continue;
        }
        const uint32_t lead = c;
        c &= (1u << (6 - continuation)) - 1;
        unsigned j = 0;
        for (; j < continuation && i < length; ++j)
        {
            const auto next = bytes[i];
            if ((next & 0xC0) != 0x80 || (j == 0 && ((lead == 0xE0 && next < 0xA0) || (lead == 0xED && next >= 0xA0) || (lead == 0xF0 && next < 0x90) ||
                                                     (lead == 0xF4 && next >= 0x90))))
                break;
            ++i;
            c = (c << 6) | (next & 63);
        }
        if (j != continuation)
            c = 0xFFFD;
        if (c < 0x10000)
            output[chars++] = static_cast<Utf16Char>(c);
        else
        {
            c -= 0x10000;
            output[chars++] = static_cast<Utf16Char>(0xD800 + (c >> 10));
            output[chars++] = static_cast<Utf16Char>(0xDC00 + (c & 1023));
        }
    }
    output[chars] = 0;
}

void encode_ansi(const Utf16Char* chars, size_t length, uint8_t* output, size_t& bytes)
{
#if LEANCLR_PLATFORM_WIN
    utils::EncodeConv::utf16_to_ansi(chars, length, reinterpret_cast<char*>(output), bytes);
#else
    encode_utf8(chars, length, output, bytes);
#endif
}

void decode_ansi(const uint8_t* bytes, size_t length, Utf16Char* output, size_t& chars)
{
#if LEANCLR_PLATFORM_WIN
    utils::EncodeConv::ansi_to_utf16(reinterpret_cast<const char*>(bytes), length, output, chars);
#else
    decode_utf8(bytes, length, output, chars);
#endif
}

RtResultVoid write_string(const ValuePlan& plan, RtString* str, uint8_t* native)
{
    const uint32_t length = str ? static_cast<uint32_t>(String::get_length(str)) : 0;
    const auto* chars = str ? String::get_chars_ptr(str) : nullptr;
    const bool wide = plan.encoding == Encoding::Unicode || plan.encoding == Encoding::BStr;
    if (plan.kind == Kind::FixedString)
    {
        if (!str)
            RET_VOID_OK();
        if (wide)
        {
            std::memcpy(native, chars, std::min(length, plan.count - 1) * sizeof(Utf16Char));
            RET_VOID_OK();
        }
        const uint64_t capacity = uint64_t(length) * 4 + 1;
        Buffer encoded(capacity);
        if (!encoded.data)
            RET_ERR(RtErr::OutOfMemory);
        size_t bytes = 0;
        encode_ansi(chars, length, encoded.data, bytes);
        std::memcpy(native, encoded.data, std::min(bytes, size_t(plan.count - 1)));
        RET_VOID_OK();
    }
    if (!str)
    {
        store<void*>(native, nullptr);
        RET_VOID_OK();
    }
    uint64_t capacity = wide ? (uint64_t(length) + 1) * 2 : uint64_t(length) * 4 + 1;
    const size_t prefix = plan.encoding == Encoding::BStr ? 4 : 0;
    if (capacity > SIZE_MAX - prefix)
        RET_ERR(RtErr::OutOfMemory);
    auto* allocated = static_cast<uint8_t*>(alloc::GeneralAllocation::calloc(static_cast<size_t>(capacity) + prefix, 1));
    if (!allocated)
        RET_ERR(RtErr::OutOfMemory);
    auto* data = allocated + prefix;
    if (wide)
        std::memcpy(data, chars, size_t(length) * 2);
    else
    {
        size_t bytes = 0;
        if (plan.encoding == Encoding::Utf8)
            encode_utf8(chars, length, data, bytes);
        else
            encode_ansi(chars, length, data, bytes);
    }
    if (prefix)
        store<uint32_t>(allocated, length * 2);
    store<void*>(native, data);
    RET_VOID_OK();
}

RtResultVoid read_string(const ValuePlan& plan, const uint8_t* native, uint8_t* managed)
{
    const uint8_t* data = plan.kind == Kind::FixedString ? native : load<const uint8_t*>(native);
    if (!data)
    {
        gc::GarbageCollector::write_barrier(reinterpret_cast<RtObject**>(managed), nullptr);
        RET_VOID_OK();
    }
    const bool wide = plan.encoding == Encoding::Unicode || plan.encoding == Encoding::BStr;
    size_t length = 0;
    if (plan.encoding == Encoding::BStr)
        length = load<uint32_t>(data - 4) / 2;
    else
    {
        const size_t max_length = plan.kind == Kind::FixedString ? plan.count : static_cast<size_t>(INT_MAX);
        while (length < max_length && (wide ? load<Utf16Char>(data + length * 2) != 0 : data[length] != 0))
            ++length;
    }
    if (length > INT_MAX)
        RET_ERR(RtErr::Argument);
    RtString* str;
    if (wide)
    {
        // Native fields may be unaligned (Pack=1); never dereference native UTF-16 directly.
        Buffer aligned((uint64_t(length) + 1) * 2);
        if (!aligned.data)
            RET_ERR(RtErr::OutOfMemory);
        std::memcpy(aligned.data, data, length * 2);
        str = String::create_string_from_utf16chars(reinterpret_cast<const Utf16Char*>(aligned.data), static_cast<int32_t>(length));
    }
    else
    {
        Buffer decoded((uint64_t(length) + 1) * 2);
        if (!decoded.data)
            RET_ERR(RtErr::OutOfMemory);
        size_t chars = 0;
        if (plan.encoding == Encoding::Utf8)
            decode_utf8(data, length, reinterpret_cast<Utf16Char*>(decoded.data), chars);
        else
            decode_ansi(data, length, reinterpret_cast<Utf16Char*>(decoded.data), chars);
        str = String::create_string_from_utf16chars(reinterpret_cast<const Utf16Char*>(decoded.data), static_cast<int32_t>(chars));
    }
    if (!str)
        RET_ERR(RtErr::OutOfMemory);
    gc::GarbageCollector::write_barrier(reinterpret_cast<RtObject**>(managed), str);
    RET_VOID_OK();
}

RtResultVoid write_value(const ValuePlan& plan, const uint8_t* managed, uint8_t* native)
{
    switch (plan.kind)
    {
    case Kind::Copy:
        std::memcpy(native, managed, plan.size);
        break;
    case Kind::Boolean:
        if (plan.size == 1)
            store<uint8_t>(native, *managed ? 1 : 0);
        else
            store<int32_t>(native, *managed ? 1 : 0);
        break;
    case Kind::VariantBoolean:
        store<int16_t>(native, *managed ? -1 : 0);
        break;
    case Kind::AnsiChar:
    {
        char encoded[5];
        size_t count = 0;
        auto c = load<Utf16Char>(managed);
        encode_ansi(&c, 1, reinterpret_cast<uint8_t*>(encoded), count);
        *native = static_cast<uint8_t>(encoded[0]);
        break;
    }
    case Kind::Structure:
        return write_layout(*plan.nested, managed, native);
    case Kind::String:
    case Kind::FixedString:
        return write_string(plan, load<RtString*>(managed), native);
    default:
        RET_ERR(RtErr::Argument);
    }
    RET_VOID_OK();
}

RtResultVoid read_value(const ValuePlan& plan, const uint8_t* native, uint8_t* managed)
{
    switch (plan.kind)
    {
    case Kind::Copy:
        std::memcpy(managed, native, plan.size);
        break;
    case Kind::Boolean:
        *managed = (plan.size == 1 ? *native != 0 : load<int32_t>(native) != 0) ? 1 : 0;
        break;
    case Kind::VariantBoolean:
        *managed = load<int16_t>(native) != 0 ? 1 : 0;
        break;
    case Kind::AnsiChar:
    {
        Utf16Char decoded[2];
        size_t count = 0;
        decode_ansi(native, 1, decoded, count);
        store<Utf16Char>(managed, count ? decoded[0] : 0);
        break;
    }
    case Kind::Structure:
        return read_layout(*plan.nested, native, managed);
    case Kind::String:
    case Kind::FixedString:
        return read_string(plan, native, managed);
    default:
        RET_ERR(RtErr::Argument);
    }
    RET_VOID_OK();
}
} // namespace

RtResult<int32_t> StructMarshal::size_of(const RtClass* klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const NativeLayout*, plan, get_layout(klass));
    RET_OK(static_cast<int32_t>(plan->value.size));
}

RtResult<intptr_t> StructMarshal::offset_of(const RtClass* klass, const char* name)
{
    if (!name)
        RET_ERR(RtErr::ArgumentNull);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const NativeLayout*, plan, get_layout(klass));
    for (const auto& f : plan->fields)
        if (std::strcmp(f.field->name, name) == 0)
            RET_OK(static_cast<intptr_t>(f.native_offset));
    RET_ERR(RtErr::Argument);
}

RtResultVoid StructMarshal::to_native(RtObject* obj, void* destination, bool delete_old)
{
    if (!obj || !destination)
        RET_ERR(RtErr::ArgumentNull);
    ObjectRoot root(obj);
    return to_native_data(obj->klass, reinterpret_cast<const uint8_t*>(obj) + RT_OBJECT_HEADER_SIZE, destination, delete_old);
}

RtResultVoid StructMarshal::to_native_data(const RtClass* klass, const void* value, void* destination, bool delete_old)
{
    if (!value || !destination)
        RET_ERR(RtErr::ArgumentNull);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const NativeLayout*, plan, get_layout(klass));
    const auto* managed = static_cast<const uint8_t*>(value);
    if (plan->identity)
    {
        std::memcpy(destination, managed, plan->value.size);
        RET_VOID_OK();
    }
    if (plan->value.kind == Kind::Boolean || plan->value.kind == Kind::VariantBoolean || plan->value.kind == Kind::AnsiChar)
    {
        // Scalar conversions cannot fail or own resources, so no transactional
        // staging allocation (or delete-old cleanup) is necessary.
        return write_value(plan->value, managed, static_cast<uint8_t*>(destination));
    }
    // Build into zeroed staging storage. A failure frees all fields built so far,
    // preserving the destination and its old resources even when delete_old=true.
    Buffer staging(plan->value.size);
    if (!staging.data)
        RET_ERR(RtErr::OutOfMemory);
    auto written = write_value(plan->value, managed, staging.data);
    if (written.is_err())
    {
        destroy_layout(*plan, staging.data);
        return written.unwrap_err();
    }
    if (delete_old)
        destroy_layout(*plan, static_cast<uint8_t*>(destination));
    std::memcpy(destination, staging.data, plan->value.size);
    RET_VOID_OK();
}

RtResultVoid StructMarshal::from_native(const void* source, RtObject* obj)
{
    if (!source || !obj)
        RET_ERR(RtErr::ArgumentNull);
    ObjectRoot root(obj);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const NativeLayout*, plan, get_layout(obj->klass));
    return read_value(plan->value, static_cast<const uint8_t*>(source), reinterpret_cast<uint8_t*>(obj) + RT_OBJECT_HEADER_SIZE);
}

RtResult<RtObject*> StructMarshal::from_native(const void* source, const RtClass* klass)
{
    if (!klass)
        RET_ERR(RtErr::ArgumentNull);
    if (!source)
        RET_OK(nullptr);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const NativeLayout*, plan, get_layout(klass));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, obj, LEANCLR_NEWOBJ_INTERNAL(klass, "StructMarshal::from_native"));
    ObjectRoot root(obj);
    if (plan->value.kind == Kind::Structure)
    {
        if (!plan->constructor_resolved)
        {
            RET_ERR_ON_FAIL(Class::initialize_all(const_cast<RtClass*>(klass)));
            plan->default_constructor = Class::get_method_for_name(klass, ".ctor", 0, false);
            plan->constructor_resolved = true;
        }
        if (plan->default_constructor)
        {
            RET_ERR_ON_FAIL(Runtime::invoke_object_arguments_with_run_cctor(plan->default_constructor, obj, nullptr, 0));
        }
        else if (Class::is_reference_type(klass))
            RET_ERR(RtErr::MissingMethod);
    }
    RET_ERR_ON_FAIL(read_value(plan->value, static_cast<const uint8_t*>(source), reinterpret_cast<uint8_t*>(obj) + RT_OBJECT_HEADER_SIZE));
    RET_OK(obj);
}

RtResultVoid StructMarshal::destroy(void* native, const RtClass* klass)
{
    if (!native)
        RET_ERR(RtErr::ArgumentNull);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const NativeLayout*, plan, get_layout(klass));
    destroy_layout(*plan, static_cast<uint8_t*>(native));
    RET_VOID_OK();
}
} // namespace vm
} // namespace leanclr

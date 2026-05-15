#include "marshal.h"

#include "alloc/general_allocation.h"
#include "rt_string.h"
#include "rt_array.h"
#include "class.h"
#include "field.h"
#include "object.h"
#include "type.h"
#include "metadata/aot_module.h"
#include "utils/string_util.h"
#include "utils/string_builder.h"
#include "platform/kernel32.h"

namespace leanclr
{
namespace vm
{

void* leanclr::vm::Marshal::alloc_hglobal(size_t size)
{
    return alloc::GeneralAllocation::malloc(size);
}

void* Marshal::re_alloc_hglobal(void* ptr, size_t size)
{
    return alloc::GeneralAllocation::realloc(ptr, size);
}

void Marshal::free_hglobal(void* ptr)
{
    alloc::GeneralAllocation::free(ptr);
}

void* Marshal::alloc_co_task_mem(int32_t size)
{
    return alloc_hglobal(static_cast<size_t>(size));
}

void* Marshal::re_alloc_co_task_mem(void* ptr, int32_t size)
{
    return re_alloc_hglobal(ptr, static_cast<size_t>(size));
}

void Marshal::free_co_task_mem(void* ptr)
{
    free_hglobal(ptr);
}

void* Marshal::alloc_co_task_mem_size(size_t size)
{
    return alloc_hglobal(size);
}

vm::RtString* Marshal::ptr_to_string_ansi(void* ptr)
{
    return String::create_string_from_utf8cstr(reinterpret_cast<const char*>(ptr));
}

vm::RtString* Marshal::ptr_to_string_ansi_len(void* ptr, int32_t len)
{
    return String::create_string_from_utf8chars(reinterpret_cast<const char*>(ptr), len);
}

vm::RtString* Marshal::ptr_to_string_uni(void* ptr)
{
    int32_t utf16_length = utils::StringUtil::get_utf16chars_length(reinterpret_cast<const Utf16Char*>(ptr));
    return String::create_string_from_utf16chars(reinterpret_cast<const uint16_t*>(ptr), utf16_length);
}

vm::RtString* Marshal::ptr_to_string_uni_len(void* ptr, int32_t len)
{
    return String::create_string_from_utf16chars(reinterpret_cast<const uint16_t*>(ptr), len);
}

void* Marshal::string_to_hglobal_ansi(const Utf16Char* chars, int32_t len)
{
    utils::StringBuilder utf8_str;
    utils::StringUtil::utf16_to_utf8(chars, static_cast<size_t>(len), utf8_str);
    return const_cast<char*>(utils::StringUtil::strdup(utf8_str.as_cstr()));
}

void* Marshal::string_to_hglobal_uni(const Utf16Char* chars, int32_t len)
{
    return (void*)utils::StringUtil::strdup_utf16_with_null_terminator(chars, static_cast<size_t>(len));
}

/// BStr is a Unicode character string that is a length-prefixed(4 bytes) double byte.
vm::RtString* Marshal::ptr_to_string_bstr(void* ptr)
{
    if (ptr == nullptr)
    {
        RET_OK(nullptr);
    }
    const Utf16Char* bstr = reinterpret_cast<const Utf16Char*>(ptr);
    uint32_t len = *reinterpret_cast<const uint32_t*>((const uint8_t*)bstr - sizeof(uint32_t));
    assert((uintptr_t)bstr % 2 == 0);
    // null terminator
    assert(bstr[len] == 0);
    return String::create_string_from_utf16chars(bstr, len);
}

Utf16Char* Marshal::alloc_bstr(size_t utf16char_count)
{
    // make sure alignment to 4 because this str is used as bstr which is a length-prefixed(4 bytes) double byte. its length should alignment to 4
    return (Utf16Char*)alloc::GeneralAllocation::calloc(utf16char_count + 2 /* length is 4 bytes */ + 1 /* null terminator */, sizeof(Utf16Char));
}

void* Marshal::buffer_to_bstr(const Utf16Char* chars, size_t len)
{
    Utf16Char* bstr = alloc_bstr(len);
    *reinterpret_cast<int32_t*>(bstr) = static_cast<int32_t>(len);
    Utf16Char* str_start = bstr + 2;
    std::memcpy(str_start, chars, len * sizeof(Utf16Char));
    str_start[len] = 0;
    // return address of the string start
    return str_start;
}

void Marshal::free_bstr(void* ptr)
{
    alloc::GeneralAllocation::free((byte*)ptr - 4);
}

void* Marshal::alloc_native_array(size_t element_count, size_t element_size)
{
    return alloc::GeneralAllocation::calloc(element_count, element_size);
}

void Marshal::free_array(void* ptr)
{
    alloc::GeneralAllocation::free(ptr);
}

static RtResultVoid structure_to_ptr_impl(vm::RtObject* obj, void* ptr)
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

static RtResultVoid ptr_to_structure_impl(void* ptr, vm::RtObject* obj)
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

static RtResultVoid destroy_structure_impl(void* ptr, const metadata::RtClass* klass)
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResultVoid Marshal::ptr_to_structure(void* ptr, vm::RtObject* obj)
{
    return ptr_to_structure_impl(ptr, obj);
}

RtResult<vm::RtObject*> Marshal::ptr_to_structure_type(void* ptr, vm::RtReflectionType* ref_type)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(ref_type->type_handle));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, Object::new_object(klass));
    RET_ERR_ON_FAIL(ptr_to_structure_impl(ptr, obj));
    RET_OK(obj);
}

RtResultVoid Marshal::structure_to_ptr(vm::RtObject* obj, void* ptr, bool delete_old)
{
    if (delete_old)
    {
        RET_ERR_ON_FAIL(destroy_structure_impl(ptr, obj->klass));
    }
    return structure_to_ptr_impl(obj, ptr);
}

RtResultVoid Marshal::destroy_structure(void* ptr, vm::RtReflectionType* ref_type)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(ref_type->type_handle));
    RET_ERR_ON_FAIL(Class::initialize_all(klass));
    return destroy_structure_impl(ptr, klass);
}

RtResult<int32_t> Marshal::sizeof_type(vm::RtReflectionType* ref_type)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(ref_type->type_handle));
    RET_ERR_ON_FAIL(Class::initialize_fields(klass));
    int32_t size = static_cast<int32_t>(vm::Class::get_instance_size_without_object_header(klass));
    RET_OK(size);
}

RtResult<intptr_t> Marshal::offset_of(vm::RtReflectionType* ref_type, const char* field_name)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(ref_type->type_handle));
    RET_ERR_ON_FAIL(Class::initialize_fields(klass));
    const metadata::RtFieldInfo* field_info = Class::get_field_for_name(klass, field_name, false);
    if (!field_info)
    {
        RET_ERR(RtErr::FileNotFound);
    }
    int32_t offset = static_cast<int32_t>(Field::get_field_offset_excludes_object_header_for_all_type(field_info));
    RET_OK(static_cast<intptr_t>(offset));
}

RtResult<RtDelegate*> Marshal::marshal_function_pointer_to_delegate(metadata::RtNativeMethodPointer ptr, metadata::RtClass* delegate_class)
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<metadata::RtNativeMethodPointer> Marshal::get_function_pointer_for_delegate(RtDelegate* delegate)
{
    if (delegate == nullptr)
    {
        RET_OK(nullptr);
    }
    auto* md = reinterpret_cast<RtMulticastDelegate*>(delegate);
    RtDelegate* single = nullptr;
    if (md->deles != nullptr)
    {
        const int32_t len = Array::get_array_length(md->deles);
        if (len != 1)
        {
            RET_ERR(RtErr::NotSupported);
        }
        single = *Array::get_array_data_start_as<RtDelegate*>(md->deles);
    }
    else
    {
        single = &md->dele;
    }
    if (single == nullptr)
    {
        RET_OK(nullptr);
    }
    const metadata::RtMethodInfo* target_method = single->method;
    if (target_method == nullptr)
    {
        RET_ERR(RtErr::NotSupported);
    }
    const metadata::RtAotMethodMonoPInvokeCallbackData* cb =
        metadata::AotModule::find_mono_pinvoke_callback_method(target_method->parent->image, target_method->token);
    if (cb == nullptr || cb->native_method_ptr == nullptr)
    {
        RET_ERR(RtErr::NotSupported);
    }
    RET_OK(cb->native_method_ptr);
}

bool read_utf8_span(utils::BinaryReader& reader, metadata::RtMarshalUtf8Span& span)
{
    uint32_t length = 0;
    if (!reader.try_read_compressed_uint32(length))
    {
        return false;
    }
    span.data = reinterpret_cast<const char*>(reader.get_current_ptr());
    if (!reader.try_advance(length))
    {
        return false;
    }
    span.length = static_cast<size_t>(length);
    return true;
}

#define READ_UINT32_OR_SET_INVALID(variable_name, default_value) \
    if (reader.not_empty())                                      \
    {                                                            \
        uint32_t value = 0;                                      \
        if (!reader.try_read_compressed_uint32(value))           \
        {                                                        \
            RET_ASSERT_ERR(RtErr::BadImageFormat);               \
        }                                                        \
        variable_name = decltype(variable_name)(value);          \
    }                                                            \
    else                                                         \
    {                                                            \
        variable_name = default_value;                           \
    }

#define READ_UTF8_SPAN_OR_SET_INVALID(span_var) \
    if (reader.not_empty())                                      \
    {                                                            \
        if (!read_utf8_span(reader, span_var)) \
        { \
            RET_ASSERT_ERR(RtErr::BadImageFormat); \
        } \
    } else { \
        span_var.set_invalid(); \
    }

#define READ_TYPESIG_OR_SET_NULL(variable_name) \
    if (reader.not_empty())                                      \
    {                                                            \
        metadata::RtMarshalUtf8Span type_name; \
        if (!read_utf8_span(reader, type_name)) \
        { \
            RET_ASSERT_ERR(RtErr::BadImageFormat); \
        } \
        UNWRAP_OR_RET_ERR_ON_FAIL(variable_name, Type::parse_assembly_qualified_type(mod, type_name.data, type_name.length, false)); \
    }                                                            \
    else                                                         \
    {                                                            \
        variable_name = nullptr;                           \
    }
RtResult<bool> Marshal::get_marshal_spec(const metadata::RtFieldInfo* field, metadata::RtMarshalSpec& spec) noexcept
{
    if (!vm::Field::has_field_marshal(field))
    {
        RET_OK(false);
    }

    metadata::RtModuleDef* mod = field->parent->image;
    const metadata::CliImage& cli_image = mod->get_cli_image();
    uint32_t field_rid = metadata::RtToken::decode_rid(field->token);
    uint32_t fieldmarshal_parent = metadata::RtMetadata::encode_has_field_marshal_coded_index(metadata::TableType::Field, field_rid);
    std::optional<uint32_t> marshal_rid = cli_image.find_row_of_owner(metadata::TableType::FieldMarshal, 0, fieldmarshal_parent);
    if (!marshal_rid)
    {
        // impossible for valid dll file.
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    std::optional<metadata::RowFieldMarshal> marshal_row = cli_image.read_field_marshal(marshal_rid.value());
    assert(marshal_row.has_value());

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, reader, mod->get_decoded_blob_reader(marshal_row->native_type));
    uint8_t native_type = 0;
    if (!reader.try_read_byte(native_type))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    assert(sizeof(metadata::RtMarshalNativeType) == sizeof(int32_t));
    spec.native_type = static_cast<metadata::RtMarshalNativeType>(native_type);
    switch (spec.native_type)
    {
    case metadata::RtMarshalNativeType::FixedSysString:
    {
        READ_UINT32_OR_SET_INVALID(spec.fixed_sys_string.size, metadata::RT_INVALID_PARAM_INDEX_OR_ELEMENT_COUNT);
        break;
    }
    case metadata::RtMarshalNativeType::SafeArray:
    {
        READ_UINT32_OR_SET_INVALID(spec.safe_array.variant_type, metadata::RtMarshalVariantType::NotInitialized);
        READ_TYPESIG_OR_SET_NULL(spec.safe_array.udt);
        break;
    }
    case metadata::RtMarshalNativeType::FixedArray:
    {
        READ_UINT32_OR_SET_INVALID(spec.fixed_array.size, metadata::RT_INVALID_PARAM_INDEX_OR_ELEMENT_COUNT);
        READ_UINT32_OR_SET_INVALID(spec.fixed_array.array_element_type, metadata::RtMarshalNativeType::NotInitialized);
        break;
    }
    case metadata::RtMarshalNativeType::Array:
    {
        READ_UINT32_OR_SET_INVALID(spec.array.array_element_type, metadata::RtMarshalNativeType::NotInitialized);
        READ_UINT32_OR_SET_INVALID(spec.array.param_index, metadata::RT_INVALID_PARAM_INDEX_OR_ELEMENT_COUNT);
        READ_UINT32_OR_SET_INVALID(spec.array.element_count, metadata::RT_INVALID_PARAM_INDEX_OR_ELEMENT_COUNT);
        break;
    }
    case metadata::RtMarshalNativeType::CustomMarshaler:
    {
        READ_UTF8_SPAN_OR_SET_INVALID(spec.custom_marshaler.guid);
        READ_UTF8_SPAN_OR_SET_INVALID(spec.custom_marshaler.type_name);
        READ_TYPESIG_OR_SET_NULL(spec.custom_marshaler.custom_marshaler_type);
        READ_UTF8_SPAN_OR_SET_INVALID(spec.custom_marshaler.cookie);
        break;
    }
    case metadata::RtMarshalNativeType::IUnknown:
    case metadata::RtMarshalNativeType::IDispatch:
    case metadata::RtMarshalNativeType::IntF:
    {
        READ_UINT32_OR_SET_INVALID(spec.interface.iid_param_index, metadata::RT_INVALID_PARAM_INDEX_OR_ELEMENT_COUNT);
        break;
    }
    default:
        break;
    }
    if (reader.not_empty())
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RET_OK(true);
}

int32_t Marshal::get_last_win32_error()
{
    return platform::Kernel32::get_last_win32_error();
}

void Marshal::set_last_win32_error(int32_t error)
{
    platform::Kernel32::set_last_win32_error(error);
}

} // namespace vm
} // namespace leanclr

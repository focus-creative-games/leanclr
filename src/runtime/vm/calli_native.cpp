#include "calli.h"
#include "alloc/general_allocation.h"
#include "vm/class.h"
#include "vm/field.h"
#include <cstdio>
#include <cstring>

namespace leanclr
{
namespace vm
{
namespace
{
using namespace metadata;

struct Entry
{
    uint64_t hash;
    const RtCalliBridge* bridge;
};

Entry* entries = nullptr;
size_t capacity = 0, entry_count = 0;

void number(uint64_t& hash, uint32_t value)
{
    for (unsigned i = 0; i < 4; ++i)
    {
        hash = (hash ^ uint8_t(value)) * UINT64_C(1099511628211);
        value >>= 8;
    }
}

bool shape_valid(const RtCalliAbiType* t, unsigned depth = 0)
{
    if (!t || depth > 16)
        return false;
    if (t->kind == 17)
    {
        if (!t->size || !t->alignment || (t->alignment & (t->alignment - 1)) || t->alignment > 8 || !t->field_count || t->field_count > 128 || !t->fields)
            return false;
        for (uint32_t i = 0; i < t->field_count; ++i)
        {
            const auto& f = t->fields[i];
            if (!shape_valid(f.type, depth + 1) || f.type->kind == 1 || f.type->kind == 2 || f.type->kind == 3 || f.offset > t->size ||
                f.type->size > t->size - f.offset)
                return false;
        }
        return true;
    }
    if (t->field_count || t->fields)
        return false;
    switch (t->kind)
    {
    case 1:
        return t->size == 0 && t->alignment == 1;
    case 3:
    case 4:
    case 5:
        return t->size == 1 && t->alignment == 1;
    case 6:
    case 7:
        return t->size == 2 && t->alignment == 2;
    case 2:
    case 8:
    case 9:
    case 12:
        return t->size == 4 && t->alignment == alignof(int32_t);
    case 10:
    case 11:
    case 13:
        return t->size == 8 && t->alignment == alignof(int64_t);
    case 15:
        return t->size == sizeof(void*) && t->alignment == alignof(void*);
    default:
        return false;
    }
}

void shape_hash(uint64_t& hash, const RtCalliAbiType& t)
{
    number(hash, t.kind);
    number(hash, t.size);
    number(hash, t.alignment);
    number(hash, t.field_count);
    for (uint32_t i = 0; i < t.field_count; ++i)
    {
        number(hash, t.fields[i].offset);
        shape_hash(hash, *t.fields[i].type);
    }
}

uint64_t bridge_hash(const RtCalliBridge& b)
{
    uint64_t hash = UINT64_C(14695981039346656037);
    number(hash, b.convention);
    number(hash, b.parameter_count);
    shape_hash(hash, *b.return_type);
    for (uint32_t i = 0; i < b.parameter_count; ++i)
        shape_hash(hash, *b.parameters[i]);
    return hash;
}

bool equal_type(const RtCalliAbiType& a, const RtCalliAbiType& b)
{
    if (a.kind != b.kind || a.size != b.size || a.alignment != b.alignment || a.field_count != b.field_count)
        return false;
    for (uint32_t i = 0; i < a.field_count; ++i)
        if (a.fields[i].offset != b.fields[i].offset || !equal_type(*a.fields[i].type, *b.fields[i].type))
            return false;
    return true;
}

bool equal_bridge(const RtCalliBridge& a, const RtCalliBridge& b)
{
    if (a.convention != b.convention || a.parameter_count != b.parameter_count || !equal_type(*a.return_type, *b.return_type))
        return false;
    for (uint32_t i = 0; i < a.parameter_count; ++i)
        if (!equal_type(*a.parameters[i], *b.parameters[i]))
            return false;
    return true;
}

struct ShapeArena
{
    RtCalliAbiType types[128];
    RtCalliAbiField fields[128];
    uint32_t used = 0;
};

RtResult<RtCalliAbiType> runtime_type(const RtTypeSig* sig, bool allow_void, ShapeArena& arena, unsigned depth = 0)
{
    if (depth > 16 || !sig || sig->pinned ||
        (sig->num_mods && (!sig->by_ref || sig->num_mods != unsigned(bool(sig->field_or_param_attrs & 1)) + unsigned(bool(sig->field_or_param_attrs & 2)))))
        RET_ERR(RtErr::NotSupported);
    uint8_t kind = uint8_t(sig->ele_type);
    if (sig->by_ref)
    {
        auto payload = *sig;
        payload.by_ref = 0;
        payload.num_mods = 0;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtCalliAbiType, inner, runtime_type(&payload, false, arena, depth + 1));
        if (inner.kind == 2 || inner.kind == 3)
            RET_ERR(RtErr::NotSupported);
        RET_OK((RtCalliAbiType{15, sizeof(void*), alignof(void*), nullptr, 0}));
    }
    if (kind == 24 || kind == 25)
        kind = 15;
    if (kind == 17 || kind == 21)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, klass, Class::get_class_from_typesig(sig));
        RET_ERR_ON_FAIL(Class::initialize_all(klass));
        if (!Class::is_enum_type(klass))
        {
            if (!Class::is_value_type(klass) || Class::get_has_references(klass) || !(klass->flags & uint32_t(RtTypeAttribute::LayoutMask)) ||
                klass->alignment > 8 || std::strncmp(klass->namespaze, "System.Runtime.Intrinsics", 25) == 0 ||
                (std::strcmp(klass->namespaze, "System.Numerics") == 0 && std::strncmp(klass->name, "Vector", 6) == 0))
                RET_ERR(RtErr::NotSupported);
            uint32_t count = 0;
            for (uint16_t i = 0; i < klass->field_count; ++i)
                if (!Field::is_static_included_literal_and_rva(klass->fields + i))
                    ++count;
            if (!count || count > 128 - arena.used)
                RET_ERR(RtErr::NotSupported);
            const auto first = arena.used;
            arena.used += count;
            uint32_t index = first;
            for (uint16_t i = 0; i < klass->field_count; ++i)
            {
                const auto& f = klass->fields[i];
                if (Field::is_static_included_literal_and_rva(&f))
                    continue;
                if (f.flags & uint32_t(RtFieldAttribute::HasFieldMarshal))
                    RET_ERR(RtErr::NotSupported);
                UNWRAP_OR_RET_ERR_ON_FAIL(arena.types[index], runtime_type(f.type_sig, false, arena, depth + 1));
                if (arena.types[index].kind == 2 || arena.types[index].kind == 3)
                    RET_ERR(RtErr::NotSupported);
                arena.fields[index] = {f.offset, arena.types + index};
                ++index;
            }
            RET_OK((RtCalliAbiType{17, klass->instance_size_without_header, klass->alignment, arena.fields + first, count}));
        }
        kind = uint8_t(Class::get_enum_element_type(klass));
    }
    uint32_t size = 0, alignment = 1;
    switch (kind)
    {
    case 3:
#if LEANCLR_PLATFORM_WIN
        size = alignment = 1;
        break;
#else
        RET_ERR(RtErr::NotSupported);
#endif
    case 1:
        if (!allow_void || sig->by_ref)
            RET_ERR(RtErr::NotSupported);
        break;
    case 4:
    case 5:
        size = alignment = 1;
        break;
    case 6:
    case 7:
        size = alignment = 2;
        break;
    case 2:
    case 8:
    case 9:
    case 12:
        size = 4;
        alignment = alignof(int32_t);
        break;
    case 10:
    case 11:
    case 13:
        size = 8;
        alignment = alignof(int64_t);
        break;
    case 15:
        size = sizeof(void*);
        alignment = alignof(void*);
        break;
    default:
        RET_ERR(RtErr::NotSupported);
    }
    RET_OK((RtCalliAbiType{kind, size, alignment, nullptr, 0}));
}

void print_type(const RtCalliAbiType& type)
{
    std::fprintf(stderr, "%u/%u@%u", unsigned(type.kind), unsigned(type.size), unsigned(type.alignment));
    if (type.field_count)
    {
        std::fputc('{', stderr);
        for (uint32_t i = 0; i < type.field_count; ++i)
        {
            std::fprintf(stderr, "%s%u:", i ? "," : "", unsigned(type.fields[i].offset));
            print_type(*type.fields[i].type);
        }
        std::fputc('}', stderr);
    }
}

RtResult<uint8_t> convention(const RtMethodSig* sig)
{
    if (!sig || sig->generic_param_count || sig->calli_flags || (sig->flags & 0xf0) || sig->params.size() > 256)
        RET_ERR(RtErr::NotSupported);
    auto c = RtMetadata::decode_sig_type(sig->flags);
    if (c == RtSigType::Unmanaged)
        c = sig->native_call_conv ? RtSigType(sig->native_call_conv) : RtSigType::C;
    else if (sig->native_call_conv || sig->call_conv_modifier_count)
        RET_ERR(RtErr::NotSupported);
    if (c != RtSigType::C && c != RtSigType::StdCall)
        RET_ERR(RtErr::NotSupported);
#if (defined(_WIN32) && defined(_M_X64)) || (defined(__ANDROID__) && defined(__x86_64__)) || (defined(__EMSCRIPTEN__) && UINTPTR_MAX == UINT32_MAX) || \
    (defined(LEANCLR_CALLI_ABI_PROBE) && LEANCLR_CALLI_ABI_PROBE)
    // Both spellings use the platform C ABI here. Other ABIs require their own tests.
    RET_OK(uint8_t(1));
#else
    RET_ERR(RtErr::NotSupported);
#endif
}
} // namespace

RtResultVoid Calli::register_native_bridges(const metadata::RtCalliBridge* table, uint32_t count)
{
    if (count && !table)
        RET_ERR(RtErr::ArgumentNull);
    for (uint32_t i = 0; i < count; ++i)
    {
        const auto& b = table[i];
        if (b.convention != 1 || !b.invoke || b.parameter_count > 256 || (b.parameter_count && !b.parameters) || !shape_valid(b.return_type))
            RET_ERR(RtErr::BadImageFormat);
        for (uint32_t p = 0; p < b.parameter_count; ++p)
            if (!shape_valid(b.parameters[p]) || b.parameters[p]->kind == 1)
                RET_ERR(RtErr::BadImageFormat);
    }
    bool all_present = true;
    for (uint32_t i = 0; i < count; ++i)
    {
        bool found = false;
        if (capacity)
        {
            const auto hash = bridge_hash(table[i]);
            size_t slot = size_t(hash) & (capacity - 1);
            while (entries[slot].bridge)
            {
                if (entries[slot].hash == hash && equal_bridge(*entries[slot].bridge, table[i]))
                {
                    found = true;
                    break;
                }
                slot = (slot + 1) & (capacity - 1);
            }
        }
        if (!found)
        {
            all_present = false;
            break;
        }
    }
    if (all_present)
        RET_VOID_OK();
    if (count > SIZE_MAX - entry_count)
        RET_ERR(RtErr::OutOfMemory);
    size_t next = capacity ? capacity : 16;
    while (entry_count + count >= next - next / 4)
    {
        if (next > SIZE_MAX / 2 / sizeof(Entry))
            RET_ERR(RtErr::OutOfMemory);
        next *= 2;
    }
    if (count && next != capacity)
    {
        auto* replacement = alloc::GeneralAllocation::calloc_any<Entry>(next);
        if (!replacement)
            RET_ERR(RtErr::OutOfMemory);
        for (size_t i = 0; i < capacity; ++i)
            if (entries[i].bridge)
            {
                size_t slot = size_t(entries[i].hash) & (next - 1);
                while (replacement[slot].bridge)
                    slot = (slot + 1) & (next - 1);
                replacement[slot] = entries[i];
            }
        alloc::GeneralAllocation::free(entries);
        entries = replacement;
        capacity = next;
    }
    for (uint32_t i = 0; i < count; ++i)
    {
        uint64_t hash = bridge_hash(table[i]);
        size_t slot = size_t(hash) & (capacity - 1);
        while (entries[slot].bridge)
        {
            if (entries[slot].hash == hash && equal_bridge(*entries[slot].bridge, table[i]))
                break;
            slot = (slot + 1) & (capacity - 1);
        }
        if (!entries[slot].bridge)
        {
            entries[slot] = {hash, &table[i]};
            ++entry_count;
        }
    }
    RET_VOID_OK();
}

RtResult<const metadata::RtCalliBridge*> Calli::bind_native(const metadata::RtMethodSig* sig)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint8_t, conv, convention(sig));
    // Return modifiers were already decoded by the signature parser; only recognized
    // optional convention markers may be removed here. Required modifiers are rejected.
    if (!sig->return_type || sig->return_type->num_mods != sig->call_conv_modifier_count)
        RET_ERR(RtErr::NotSupported);
    auto return_sig = *sig->return_type;
    return_sig.num_mods = 0;
    ShapeArena arena{};
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtCalliAbiType, ret, runtime_type(&return_sig, true, arena));
    RtCalliAbiType args[256];
    const RtCalliAbiType* pointers[256];
    for (size_t i = 0; i < sig->params.size(); ++i)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(args[i], runtime_type(sig->params[i], false, arena));
        pointers[i] = args + i;
    }
    RtCalliBridge wanted{conv, &ret, pointers, uint32_t(sig->params.size()), nullptr, nullptr};
    const uint64_t hash = bridge_hash(wanted);
    if (capacity)
    {
        size_t slot = size_t(hash) & (capacity - 1);
        while (entries[slot].bridge)
        {
            if (entries[slot].hash == hash && equal_bridge(*entries[slot].bridge, wanted))
                RET_OK(entries[slot].bridge);
            slot = (slot + 1) & (capacity - 1);
        }
    }
    std::fprintf(stderr, "LeanCLR: missing native calli bridge (conv=%u, return=", unsigned(conv));
    print_type(ret);
    std::fprintf(stderr, ", params=");
    for (size_t i = 0; i < sig->params.size(); ++i)
    {
        if (i)
            std::fputc(',', stderr);
        print_type(args[i]);
    }
    std::fprintf(stderr, "). Generate/link calli_bridges.cpp or register a host bridge.\n");
    RET_ERR(RtErr::NotSupported);
}
} // namespace vm
} // namespace leanclr

#include "calli.h"
#include "alloc/general_allocation.h"
#include "metadata/metadata_compare.h"
#include "vm/method.h"

namespace leanclr
{
namespace vm
{
namespace
{
// Open addressing gives fallible growth and allocation-free repeated ldftn/calli.
// Only escaped managed targets occupy this process-owned metadata index.
const metadata::RtMethodInfo** targets = nullptr;
size_t capacity = 0, count = 0;

size_t slot_for(const metadata::RtMethodInfo* method, size_t size)
{
    auto key = reinterpret_cast<uintptr_t>(method) / alignof(metadata::RtMethodInfo);
    key ^= key >> 16;
    key *= uintptr_t(0x9e3779b1u);
    return static_cast<size_t>(key) & (size - 1);
}

bool contains(const metadata::RtMethodInfo* method)
{
    if (!method || !capacity)
        return false;
    auto slot = slot_for(method, capacity);
    while (targets[slot])
    {
        if (targets[slot] == method)
            return true;
        slot = (slot + 1) & (capacity - 1);
    }
    return false;
}
} // namespace

RtResultVoid Calli::register_managed_target(const metadata::RtMethodInfo* method)
{
    if (!method)
        RET_ERR(RtErr::ArgumentNull);
    if (contains(method))
        RET_VOID_OK();
    if (!capacity || count >= capacity - capacity / 4)
    {
        if (capacity > SIZE_MAX / 2 / sizeof(*targets))
            RET_ERR(RtErr::OutOfMemory);
        const size_t next = capacity ? capacity * 2 : 16;
        auto* replacement = alloc::GeneralAllocation::calloc_any<const metadata::RtMethodInfo*>(next);
        if (!replacement)
            RET_ERR(RtErr::OutOfMemory);
        for (size_t i = 0; i < capacity; ++i)
        {
            if (!targets[i])
                continue;
            auto slot = slot_for(targets[i], next);
            while (replacement[slot])
                slot = (slot + 1) & (next - 1);
            replacement[slot] = targets[i];
        }
        alloc::GeneralAllocation::free(targets);
        targets = replacement;
        capacity = next;
    }
    auto slot = slot_for(method, capacity);
    while (targets[slot])
        slot = (slot + 1) & (capacity - 1);
    targets[slot] = method;
    ++count;
    RET_VOID_OK();
}

RtResultVoid Calli::validate_managed_signature(const metadata::RtMethodSig* signature)
{
    if (!signature || !signature->return_type)
        RET_ERR(RtErr::ArgumentNull);
    if (metadata::RtMetadata::decode_sig_type(signature->flags) != metadata::RtSigType::Default || signature->generic_param_count ||
        (signature->flags & ~uint8_t(metadata::RtSigType::HasThis)) || signature->native_call_conv || signature->calli_flags ||
        signature->call_conv_modifier_count || signature->return_type->num_mods || signature->return_type->pinned)
        RET_ERR(RtErr::NotSupported);
    for (const auto* param : signature->params)
        if (!param || param->is_void() || param->num_mods || param->pinned)
            RET_ERR(RtErr::NotSupported);
    RET_VOID_OK();
}

RtResultVoid Calli::validate_managed_target(const metadata::RtMethodSig* signature, const metadata::RtMethodInfo* target)
{
    if (!target)
        RET_ERR(RtErr::NullReference);
    if (!contains(target))
        RET_ERR(RtErr::Argument);
    RET_ERR_ON_FAIL(validate_managed_signature(signature));
    if (Method::contains_not_instantiated_generic_param(target))
        RET_ERR(RtErr::NotSupported);
    if (Method::is_instance(target) != Method::has_this(signature) || target->parameter_count != signature->params.size() ||
        !metadata::MetadataCompare::is_typesig_equal_ignore_attrs(target->return_type, signature->return_type, false) ||
        !metadata::MetadataCompare::is_typesigs_equal_ignore_attrs(target->parameters, signature->params.data(), signature->params.size(), false))
        RET_ERR(RtErr::Argument);
    RET_VOID_OK();
}
} // namespace vm
} // namespace leanclr

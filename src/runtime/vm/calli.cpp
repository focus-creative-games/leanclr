#include "calli.h"
#include "alloc/general_allocation.h"
#include "metadata/metadata_compare.h"
#include "vm/method.h"
#include "vm/class.h"
#include "gc/garbage_collector.h"
#include "vm/runtime.h"
#include "metadata/module_def.h"
#include "utils/little_endian.h"
#include <new>

namespace leanclr
{
namespace vm
{
struct CalliSite
{
    metadata::RtMethodSig signature;
    uint32_t il_offset;
    const metadata::RtMethodInfo* last_target;
    const metadata::RtCalliBridge* bridge;
};

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
        (signature->flags & ~uint8_t(uint8_t(metadata::RtSigType::HasThis) | uint8_t(metadata::RtSigType::ExplicitThis))) || signature->native_call_conv ||
        signature->calli_flags || signature->call_conv_modifier_count || signature->return_type->num_mods || signature->return_type->pinned)
        RET_ERR(RtErr::NotSupported);
    if ((signature->flags & uint8_t(metadata::RtSigType::ExplicitThis)) && (!Method::has_this(signature) || signature->params.empty()))
        RET_ERR(RtErr::BadImageFormat);
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
    const bool explicit_this = (signature->flags & uint8_t(metadata::RtSigType::ExplicitThis)) != 0;
    const size_t first = explicit_this ? 1 : 0;
    if (explicit_this)
    {
        const auto* this_type = Class::is_value_type(target->parent) ? target->parent->by_ref : target->parent->by_val;
        if (!metadata::MetadataCompare::is_typesig_equal_ignore_attrs(this_type, signature->params[0], false))
            RET_ERR(RtErr::Argument);
    }
    if (Method::is_instance(target) != Method::has_this(signature) || target->parameter_count != signature->params.size() - first ||
        !metadata::MetadataCompare::is_typesig_equal_ignore_attrs(target->return_type, signature->return_type, false) ||
        !metadata::MetadataCompare::is_typesigs_equal_ignore_attrs(target->parameters, signature->params.data() + first, target->parameter_count, false))
        RET_ERR(RtErr::Argument);
    RET_VOID_OK();
}

namespace
{
struct SiteEntry
{
    const metadata::RtMethodInfo* caller;
    CalliSite* site;
};

SiteEntry* site_entries = nullptr;
size_t site_capacity = 0, site_count = 0;

size_t site_slot(const metadata::RtMethodInfo* caller, uint32_t offset, size_t size)
{
    return (slot_for(caller, size) ^ (static_cast<size_t>(offset) * size_t(0x85ebca6bu))) & (size - 1);
}

RtResult<CalliSite*> get_site(const metadata::RtMethodInfo* caller, uint32_t offset)
{
    if (site_capacity)
    {
        size_t slot = site_slot(caller, offset, site_capacity);
        while (site_entries[slot].caller)
        {
            if (site_entries[slot].caller == caller && site_entries[slot].site->il_offset == offset)
                RET_OK(site_entries[slot].site);
            slot = (slot + 1) & (site_capacity - 1);
        }
    }
    if (!site_capacity || site_count >= site_capacity - site_capacity / 4)
    {
        if (site_capacity > SIZE_MAX / 2 / sizeof(SiteEntry))
            RET_ERR(RtErr::OutOfMemory);
        const size_t next = site_capacity ? site_capacity * 2 : 16;
        auto* entries = alloc::GeneralAllocation::calloc_any<SiteEntry>(next);
        if (!entries)
            RET_ERR(RtErr::OutOfMemory);
        for (size_t i = 0; i < site_capacity; ++i)
        {
            if (!site_entries[i].caller)
                continue;
            size_t slot = site_slot(site_entries[i].caller, site_entries[i].site->il_offset, next);
            while (entries[slot].caller)
                slot = (slot + 1) & (next - 1);
            entries[slot] = site_entries[i];
        }
        alloc::GeneralAllocation::free(site_entries);
        site_entries = entries;
        site_capacity = next;
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<metadata::RtMethodBody>, body, Method::get_method_body(caller));
    if (!body || offset > body->code_size || body->code_size - offset < 5 || body->code[offset] != 0x29)
        RET_ERR(RtErr::BadImageFormat);
    const uint32_t token = utils::LittleEndian::read_u32(body->code + offset + 1);
    const metadata::RtGenericContainerContext gcc{caller->parent->generic_container, caller->generic_container};
    const auto* context = caller->generic_method ? &caller->generic_method->generic_context : nullptr;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtMethodSig, signature, caller->parent->image->read_stadalone_method_sig(token, gcc, context));
    const metadata::RtCalliBridge* bridge = nullptr;
    if (metadata::RtMetadata::decode_sig_type(signature.flags) == metadata::RtSigType::Default)
    {
        RET_ERR_ON_FAIL(Calli::validate_managed_signature(&signature));
    }
    else
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(bridge, Calli::bind_native(&signature));
    }
    // Native argument roots need complete value-type reference bitmaps before
    // the callee (including a type initializer) is allowed to collect.
    for (const auto* type : signature.params)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(type));
        RET_ERR_ON_FAIL(Class::initialize_all(klass));
    }
    if (!signature.return_type->is_void())
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(signature.return_type));
        RET_ERR_ON_FAIL(Class::initialize_all(klass));
    }
    auto* memory = alloc::GeneralAllocation::malloc_any<CalliSite>();
    if (!memory)
        RET_ERR(RtErr::OutOfMemory);
    auto* site = new (memory) CalliSite{std::move(signature), offset, nullptr, bridge};
    size_t slot = site_slot(caller, offset, site_capacity);
    while (site_entries[slot].caller)
        slot = (slot + 1) & (site_capacity - 1);
    site_entries[slot] = {caller, site};
    ++site_count;
    RET_OK(site);
}
} // namespace

RtResultVoid Calli::invoke_unmanaged(SiteCache& cache, const metadata::RtMethodInfo* caller, uint32_t offset, void* target, interp::RtStackObject* args,
                                     interp::RtStackObject* ret)
{
    if (!caller)
        RET_ERR(RtErr::ArgumentNull);
    if (cache.caller != caller || !cache.site || cache.site->il_offset != offset)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(CalliSite*, site, get_site(caller, offset));
        cache.caller = caller;
        cache.site = site;
    }
    return invoke_native(cache.site->bridge, target, args, ret);
}

void* Calli::value_receiver(const metadata::RtMethodInfo* target, void* receiver)
{
    if (!Method::is_instance(target) || !Class::is_value_type(target->parent) || !receiver)
        return receiver;
    auto* owner = gc::GarbageCollector::find_object_containing(receiver);
    // ldvirtftn on a boxed value denotes an unboxing entry. Ordinary value this
    // pointers already address the payload (including stack/field interiors).
    return owner == receiver && owner->klass == target->parent ? static_cast<void*>(owner + 1) : receiver;
}

RtResultVoid Calli::invoke_native(const metadata::RtCalliBridge* bridge, void* target, interp::RtStackObject* args, interp::RtStackObject* ret)
{
    if (!target)
        RET_ERR(RtErr::NullReference);
    if (contains(reinterpret_cast<const metadata::RtMethodInfo*>(target)))
        RET_ERR(RtErr::Argument);
    if (!bridge || !bridge->invoke)
        RET_ERR(RtErr::NotSupported);
    if ((bridge->parameter_count && !args) || (bridge->return_type->kind != 1 && !ret))
        RET_ERR(RtErr::ArgumentNull);
    bridge->invoke(target, args, ret);
    RET_VOID_OK();
}

RtResultVoid Calli::invoke_managed(SiteCache& cache, const metadata::RtMethodInfo* caller, uint32_t offset, const metadata::RtMethodInfo* target,
                                   interp::RtStackObject* args, interp::RtStackObject* ret)
{
    if (!caller)
        RET_ERR(RtErr::ArgumentNull);
    if (!target)
        RET_ERR(RtErr::NullReference);
    if (cache.caller != caller || !cache.site || cache.site->il_offset != offset)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(CalliSite*, site, get_site(caller, offset));
        cache.caller = caller;
        cache.site = site;
    }
    auto* site = cache.site;
    if (site->last_target != target)
    {
        RET_ERR_ON_FAIL(validate_managed_target(&site->signature, target));
        site->last_target = target;
    }
    if ((target->total_arg_stack_object_size && !args) || (!site->signature.return_type->is_void() && !ret))
        RET_ERR(RtErr::ArgumentNull);
    if (Method::is_instance(target) && !args[0].ptr)
        RET_ERR(RtErr::NullReference);
    if (Method::is_instance(target))
        args[0].ptr = value_receiver(target, args[0].ptr);
    return Runtime::invoke_stackobject_arguments_with_run_cctor(target, args, ret);
}
} // namespace vm
} // namespace leanclr

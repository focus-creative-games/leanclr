#include "calli.h"
#include "interp/interpreter.h"
#include "interp/machine_state.h"
#include "alloc/general_allocation.h"
#include "metadata/metadata_compare.h"
#include "vm/method.h"
#include "vm/class.h"
#include "gc/garbage_collector.h"
#include "vm/runtime.h"
#include "metadata/module_def.h"
#include "utils/little_endian.h"
#include <new>
#include <cstdio>

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
        if (!param || param->is_void() ||
            (param->num_mods &&
             (!param->by_ref || param->num_mods != unsigned(bool(param->field_or_param_attrs & 1)) + unsigned(bool(param->field_or_param_attrs & 2)))) ||
            param->pinned)
            RET_ERR(RtErr::NotSupported);
    RET_VOID_OK();
}

namespace
{
RtResult<bool> reference_conversion(const metadata::RtTypeSig* from, const metadata::RtTypeSig* to)
{
    if (from->by_ref || to->by_ref)
        RET_OK(false);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, source, Class::get_class_from_typesig(from));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, destination, Class::get_class_from_typesig(to));
    RET_ERR_ON_FAIL(Class::initialize_all(source));
    RET_ERR_ON_FAIL(Class::initialize_all(destination));
    RET_OK(Class::is_reference_type(source) && Class::is_reference_type(destination) && Class::is_assignable_from(source, destination));
}

RtResultVoid compatible_type(const metadata::RtTypeSig* from, const metadata::RtTypeSig* to)
{
    if (metadata::MetadataCompare::is_typesig_equal_ignore_attrs(from, to, false))
        RET_VOID_OK();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, convertible, reference_conversion(from, to));
    if (!convertible)
        RET_ERR(RtErr::Argument);
    RET_VOID_OK();
}
} // namespace

RtResultVoid Calli::validate_managed_binding(ManagedBinding& binding, const metadata::RtMethodInfo* target)
{
    if (binding.target && binding.target == target)
        RET_VOID_OK();
    RET_ERR_ON_FAIL(validate_managed_target(binding.signature, target));
    binding.target = target;
    RET_VOID_OK();
}

RtResultVoid Calli::validate_managed_target(const metadata::RtMethodSig* signature, const metadata::RtMethodInfo* target)
{
    if (!target)
        RET_ERR(RtErr::NullReference);
    if (!is_managed_target(target))
        RET_ERR(RtErr::Argument);
    RET_ERR_ON_FAIL(validate_managed_signature(signature));
    if (Method::contains_not_instantiated_generic_param(target))
        RET_ERR(RtErr::NotSupported);
    const bool explicit_this = (signature->flags & uint8_t(metadata::RtSigType::ExplicitThis)) != 0;
    const size_t first = explicit_this ? 1 : 0;
    if (explicit_this)
    {
        const auto* this_type = Class::is_value_type(target->parent) ? target->parent->by_ref : target->parent->by_val;
        RET_ERR_ON_FAIL(compatible_type(signature->params[0], this_type));
    }
    if (Method::is_instance(target) != Method::has_this(signature) || target->parameter_count != signature->params.size() - first)
        RET_ERR(RtErr::Argument);
    // Function pointer conversions permit reference covariance for the return and
    // contravariance for parameters. Byrefs and value representations stay exact.
    RET_ERR_ON_FAIL(compatible_type(target->return_type, signature->return_type));
    for (uint16_t i = 0; i < target->parameter_count; ++i)
        RET_ERR_ON_FAIL(compatible_type(signature->params[i + first], target->parameters[i]));
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
    auto key = reinterpret_cast<uintptr_t>(caller) / alignof(metadata::RtMethodInfo);
    key ^= key >> 16;
    key *= uintptr_t(0x9e3779b1u);
    return (static_cast<size_t>(key) ^ (static_cast<size_t>(offset) * size_t(0x85ebca6bu))) & (size - 1);
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
        auto binding = Calli::bind_native(&signature);
        if (binding.is_err())
        {
            std::fprintf(stderr, "LeanCLR calli caller: [%s]%s.%s::%s IL_%04X\n", caller->parent->image->get_name_no_ext(), caller->parent->namespaze,
                         caller->parent->name, caller->name, unsigned(offset));
            RET_ERR(binding.unwrap_err());
        }
        bridge = binding.unwrap();
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
    // Ordinary generated instance bodies assume non-null this. A calli may legally
    // pass null to a body that never accesses it; interpret this rare case instead
    // of violating the C++ optimizer's non-null contract.
    if (Method::is_instance(target) && !args[0].ptr && target->invoker_type == metadata::RtInvokerType::Aot)
    {
        interp::MethodArgumentRoots roots(target, args);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const interp::RtStackObject*, value, interp::Interpreter::execute(target, args));
        if (target->ret_stack_object_size)
            std::memcpy(ret, value, target->ret_stack_object_size * sizeof(interp::RtStackObject));
        RET_VOID_OK();
    }
    if (Method::is_instance(target))
        args[0].ptr = value_receiver(target, args[0].ptr);
    return Runtime::invoke_stackobject_arguments_with_run_cctor(target, args, ret);
}
} // namespace vm
} // namespace leanclr

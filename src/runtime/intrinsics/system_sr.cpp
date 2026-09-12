#include "system_sr.h"

#include "interp/eval_stack_op.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<vm::RtString*> SystemSR::get_resource_string(vm::RtString* key) noexcept
{
    if (key == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    RET_OK(key);
}

RtResult<vm::RtString*> SystemSR::format(vm::RtString* resource_format) noexcept
{
    if (resource_format == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    RET_OK(resource_format);
}

RtResult<bool> SystemSR::using_resource_keys() noexcept
{
    RET_OK(true);
}

/// @intrinsic: System.SR::GetResourceString(System.String)
static RtResultVoid get_resource_string_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto key = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemSR::get_resource_string(key));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @intrinsic: System.SR::Format(System.String,...)
static RtResultVoid format_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                   interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto resource_format = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemSR::format(resource_format));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @intrinsic: System.SR::UsingResourceKeys()
static RtResultVoid using_resource_keys_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemSR::using_resource_keys());
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_sr[] = {
    {"System.SR::GetResourceString(System.String)", (vm::IntrinsicFunction)&SystemSR::get_resource_string, get_resource_string_invoker},
    {"System.SR::InternalGetResourceString(System.String)", (vm::IntrinsicFunction)&SystemSR::get_resource_string, get_resource_string_invoker},
    {"System.SR::Format", (vm::IntrinsicFunction)&SystemSR::format, format_invoker},
    {"System.SR::UsingResourceKeys()", (vm::IntrinsicFunction)&SystemSR::using_resource_keys, using_resource_keys_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemSR::get_intrinsic_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_intrinsic_entries_system_sr) / sizeof(s_intrinsic_entries_system_sr[0]);
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_sr, entry_count);
}

} // namespace intrinsics
} // namespace leanclr

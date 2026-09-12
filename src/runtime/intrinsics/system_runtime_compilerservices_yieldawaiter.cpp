#include "system_runtime_compilerservices_yieldawaiter.h"

#include "interp/eval_stack_op.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<bool> SystemRuntimeCompilerServicesYieldAwaiter::get_is_completed() noexcept
{
    RET_OK(true);
}

static RtResultVoid get_is_completed_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                             interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeCompilerServicesYieldAwaiter::get_is_completed());
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_runtime_compilerservices_yieldawaiter[] = {
    {"System.Runtime.CompilerServices.YieldAwaitable/YieldAwaiter::get_IsCompleted",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesYieldAwaiter::get_is_completed, get_is_completed_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemRuntimeCompilerServicesYieldAwaiter::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_runtime_compilerservices_yieldawaiter,
                                           sizeof(s_intrinsic_entries_system_runtime_compilerservices_yieldawaiter) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

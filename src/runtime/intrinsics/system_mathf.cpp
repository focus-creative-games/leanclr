#include "system_mathf.h"

#include <cmath>

#include "interp/eval_stack_op.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<float> SystemMathF::round(float value) noexcept
{
    RET_OK(static_cast<float>(std::round(static_cast<double>(value))));
}

/// @intrinsic: System.MathF::Round(System.Single)
static RtResultVoid round_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    auto value = interp::EvalStackOp::get_param<float>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemMathF::round(value));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_mathf[] = {
    {"System.MathF::Round(System.Single)", (vm::IntrinsicFunction)&SystemMathF::round, round_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemMathF::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_mathf, sizeof(s_intrinsic_entries_system_mathf) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

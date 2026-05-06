#include "system_numerics_vector.h"
#include "interp/interp_defs.h"
#include "platform/hardware.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<bool> SystemNumericsVector::get_is_hardware_accelerated() noexcept
{
    RET_OK(pal::Hardware::is_hardware_accelerated());
}

/// @intrinsic: System.Numerics.Vector::get_IsHardwareAccelerated
static RtResultVoid get_is_hardware_accelerated_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemNumericsVector::get_is_hardware_accelerated());
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

// Intrinsic registry
static vm::IntrinsicEntry s_intrinsic_entries_system_numerics_vector[] = {
    {"System.Numerics.Vector::get_IsHardwareAccelerated", (vm::IntrinsicFunction)&SystemNumericsVector::get_is_hardware_accelerated,
     get_is_hardware_accelerated_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemNumericsVector::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_numerics_vector,
                                           sizeof(s_intrinsic_entries_system_numerics_vector) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

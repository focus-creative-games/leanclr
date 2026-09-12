#include "system_numerics_vector.h"
#include "interp/eval_stack_op.h"
#include "interp/interp_defs.h"
#include "vm/class.h"
#include "vm/type.h"

namespace leanclr
{
namespace intrinsics
{
namespace
{

RtResult<const metadata::RtTypeSig*> get_vector_element_type(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr || method->parent == nullptr)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    const metadata::RtClass* klass = method->parent;
    if (vm::Class::is_generic_inst(klass))
    {
        const metadata::RtGenericClass* generic_class = klass->by_val->data.generic_class;
        if (generic_class != nullptr && generic_class->class_inst != nullptr && generic_class->class_inst->generic_arg_count == 1)
        {
            RET_OK(generic_class->class_inst->generic_args[0]);
        }
    }

    if (method->generic_method != nullptr && method->generic_method->generic_context.class_inst != nullptr &&
        method->generic_method->generic_context.class_inst->generic_arg_count == 1)
    {
        RET_OK(method->generic_method->generic_context.class_inst->generic_args[0]);
    }

    RET_ERR(RtErr::BadImageFormat);
}

bool is_supported_intrinsics_base_type(const metadata::RtTypeSig* type_sig) noexcept
{
    if (type_sig == nullptr || type_sig->by_ref)
    {
        return false;
    }

    switch (type_sig->ele_type)
    {
    case metadata::RtElementType::I1:
    case metadata::RtElementType::U1:
    case metadata::RtElementType::I2:
    case metadata::RtElementType::U2:
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    case metadata::RtElementType::I:
    case metadata::RtElementType::U:
    case metadata::RtElementType::R4:
    case metadata::RtElementType::R8:
        return true;
    default:
        return false;
    }
}

template <int32_t VectorSize>
RtResultVoid get_count_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                               interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, count, SystemNumericsVector::get_count(method, VectorSize));
    interp::EvalStackOp::set_return(ret, count);
    RET_VOID_OK();
}

} // namespace

RtResult<bool> SystemNumericsVector::get_is_hardware_accelerated() noexcept
{
    // LeanCLR does not implement CoreCLR's SIMD instruction surface yet.
    // Report false so System.Private.CoreLib stays on its scalar paths.
    RET_OK(false);
}

RtResult<int32_t> SystemNumericsVector::get_count(const metadata::RtMethodInfo* method, int32_t vector_size) noexcept
{
    if (vector_size <= 0)
    {
        RET_ERR(RtErr::ArgumentOutOfRange);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, element_type, get_vector_element_type(method));
    if (!is_supported_intrinsics_base_type(element_type))
    {
        RET_ERR(RtErr::NotSupported);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, element_size, vm::Type::get_size_of_type(element_type));
    if (element_size == 0 || static_cast<size_t>(vector_size) % element_size != 0)
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    RET_OK(static_cast<int32_t>(static_cast<size_t>(vector_size) / element_size));
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
    {"System.Numerics.Vector::get_IsHardwareAccelerated()", (vm::IntrinsicFunction)&SystemNumericsVector::get_is_hardware_accelerated,
     get_is_hardware_accelerated_invoker},
    {"System.Numerics.Vector::get_IsHardwareAccelerated", (vm::IntrinsicFunction)&SystemNumericsVector::get_is_hardware_accelerated,
     get_is_hardware_accelerated_invoker},
    {"System.Runtime.Intrinsics.Vector64::get_IsHardwareAccelerated()",
     (vm::IntrinsicFunction)&SystemNumericsVector::get_is_hardware_accelerated, get_is_hardware_accelerated_invoker},
    {"System.Runtime.Intrinsics.Vector128::get_IsHardwareAccelerated()",
     (vm::IntrinsicFunction)&SystemNumericsVector::get_is_hardware_accelerated, get_is_hardware_accelerated_invoker},
    {"System.Runtime.Intrinsics.Vector128::get_IsHardwareAccelerated",
     (vm::IntrinsicFunction)&SystemNumericsVector::get_is_hardware_accelerated, get_is_hardware_accelerated_invoker},
    {"System.Runtime.Intrinsics.Vector256::get_IsHardwareAccelerated()",
     (vm::IntrinsicFunction)&SystemNumericsVector::get_is_hardware_accelerated, get_is_hardware_accelerated_invoker},
    {"System.Runtime.Intrinsics.Vector256::get_IsHardwareAccelerated",
     (vm::IntrinsicFunction)&SystemNumericsVector::get_is_hardware_accelerated, get_is_hardware_accelerated_invoker},
    {"System.Runtime.Intrinsics.Vector512::get_IsHardwareAccelerated()",
     (vm::IntrinsicFunction)&SystemNumericsVector::get_is_hardware_accelerated, get_is_hardware_accelerated_invoker},
    {"System.Runtime.Intrinsics.Vector512::get_IsHardwareAccelerated",
     (vm::IntrinsicFunction)&SystemNumericsVector::get_is_hardware_accelerated, get_is_hardware_accelerated_invoker},
    {"System.Runtime.Intrinsics.Vector64`1::get_Count()",
     reinterpret_cast<vm::IntrinsicFunction>(&SystemNumericsVector::get_count), get_count_invoker<8>},
    {"System.Runtime.Intrinsics.Vector64`1::get_Count", reinterpret_cast<vm::IntrinsicFunction>(&SystemNumericsVector::get_count),
     get_count_invoker<8>},
    {"System.Runtime.Intrinsics.Vector128`1::get_Count()",
     reinterpret_cast<vm::IntrinsicFunction>(&SystemNumericsVector::get_count), get_count_invoker<16>},
    {"System.Runtime.Intrinsics.Vector128`1::get_Count", reinterpret_cast<vm::IntrinsicFunction>(&SystemNumericsVector::get_count),
     get_count_invoker<16>},
    {"System.Runtime.Intrinsics.Vector256`1::get_Count()",
     reinterpret_cast<vm::IntrinsicFunction>(&SystemNumericsVector::get_count), get_count_invoker<32>},
    {"System.Runtime.Intrinsics.Vector256`1::get_Count", reinterpret_cast<vm::IntrinsicFunction>(&SystemNumericsVector::get_count),
     get_count_invoker<32>},
    {"System.Runtime.Intrinsics.Vector512`1::get_Count()",
     reinterpret_cast<vm::IntrinsicFunction>(&SystemNumericsVector::get_count), get_count_invoker<64>},
    {"System.Runtime.Intrinsics.Vector512`1::get_Count", reinterpret_cast<vm::IntrinsicFunction>(&SystemNumericsVector::get_count),
     get_count_invoker<64>},
};

utils::Span<vm::IntrinsicEntry> SystemNumericsVector::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_numerics_vector,
                                           sizeof(s_intrinsic_entries_system_numerics_vector) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

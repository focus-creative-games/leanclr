#include "system_runtime_interopservices_memorymarshal.h"

#include "interp/eval_stack_op.h"
#include "interp/interp_defs.h"
#include "vm/rt_array.h"

namespace leanclr
{
namespace intrinsics
{
namespace
{

RtResult<size_t> get_method_generic_arg_size(const metadata::RtMethodInfo* method, uint8_t index) noexcept
{
    if (method == nullptr || method->generic_method == nullptr || method->generic_method->generic_context.method_inst == nullptr)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    const metadata::RtGenericInst* method_inst = method->generic_method->generic_context.method_inst;
    if (index >= method_inst->generic_arg_count)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, type_and_size,
                                            interp::InterpDefs::get_reduce_type_and_size_by_typesig(method_inst->generic_args[index]));
    RET_OK(type_and_size.byte_size);
}

} // namespace

RtResult<void*> SystemRuntimeInteropServicesMemoryMarshal::get_array_data_reference(vm::RtArray* array) noexcept
{
    if (array == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    RET_OK(vm::Array::get_array_data_start_as_ptr_void(array));
}

RtResult<vm::RtReadOnlySpan<uint8_t>> SystemRuntimeInteropServicesMemoryMarshal::cast_span(const metadata::RtMethodInfo* method,
                                                                                           vm::RtReadOnlySpan<uint8_t> span) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, from_size, get_method_generic_arg_size(method, 0));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, to_size, get_method_generic_arg_size(method, 1));
    if (from_size == 0 || to_size == 0)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    size_t byte_length = static_cast<size_t>(span.length) * from_size;
    if (byte_length % to_size != 0)
    {
        RET_ERR(RtErr::Argument);
    }

    vm::RtReadOnlySpan<uint8_t> result{span.pointer, static_cast<int32_t>(byte_length / to_size)};
    RET_OK(result);
}

/// @intrinsic: System.Runtime.InteropServices.MemoryMarshal::GetArrayDataReference<>(T[])
/// @intrinsic: System.Runtime.InteropServices.MemoryMarshal::GetArrayDataReference(System.Array)
static RtResultVoid get_array_data_reference_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtArray* array = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, data, SystemRuntimeInteropServicesMemoryMarshal::get_array_data_reference(array));
    interp::EvalStackOp::set_return(ret, data);
    RET_VOID_OK();
}

/// @intrinsic: System.Runtime.InteropServices.MemoryMarshal::Cast<,>(System.Span`1<TFrom>)
/// @intrinsic: System.Runtime.InteropServices.MemoryMarshal::Cast<,>(System.ReadOnlySpan`1<TFrom>)
static RtResultVoid cast_span_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method,
                                      const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto span = interp::EvalStackOp::get_param<vm::RtReadOnlySpan<uint8_t>>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReadOnlySpan<uint8_t>, result,
                                            SystemRuntimeInteropServicesMemoryMarshal::cast_span(method, span));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_runtime_interopservices_memorymarshal[] = {
    {"System.Runtime.InteropServices.MemoryMarshal::GetArrayDataReference<>",
     (vm::IntrinsicFunction)&SystemRuntimeInteropServicesMemoryMarshal::get_array_data_reference, get_array_data_reference_invoker},
    {"System.Runtime.InteropServices.MemoryMarshal::GetArrayDataReference(System.Array)",
     (vm::IntrinsicFunction)&SystemRuntimeInteropServicesMemoryMarshal::get_array_data_reference, get_array_data_reference_invoker},
    {"System.Runtime.InteropServices.MemoryMarshal::Cast<,>(System.Span`1<TFrom>)", nullptr, cast_span_invoker},
    {"System.Runtime.InteropServices.MemoryMarshal::Cast<,>(System.ReadOnlySpan`1<TFrom>)", nullptr, cast_span_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemRuntimeInteropServicesMemoryMarshal::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_runtime_interopservices_memorymarshal,
                                           sizeof(s_intrinsic_entries_system_runtime_interopservices_memorymarshal) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

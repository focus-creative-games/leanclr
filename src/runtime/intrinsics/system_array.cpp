#include "system_array.h"

#include "interp/eval_stack_op.h"
#include "vm/rt_array.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<int32_t> SystemArray::get_length(vm::RtArray* arr) noexcept
{
    RET_OK(vm::Array::get_array_length(arr));
}

RtResult<int64_t> SystemArray::get_long_length(vm::RtArray* arr) noexcept
{
    RET_OK(static_cast<int64_t>(vm::Array::get_array_length(arr)));
}

RtResultVoid SystemArray::get_generic_value_impl(vm::RtArray* arr, int32_t index, void* value) noexcept
{
    if (vm::Array::is_out_of_range(arr, index))
    {
        RET_ERR(RtErr::IndexOutOfRange);
    }
    vm::Array::copy_array_data_to_no_eval_stack(arr, index, value);
    RET_VOID_OK();
}

RtResultVoid SystemArray::set_generic_value_impl(vm::RtArray* arr, int32_t index, void* value) noexcept
{
    if (vm::Array::is_out_of_range(arr, index))
    {
        RET_ERR(RtErr::IndexOutOfRange);
    }
    size_t ele_size = vm::Array::get_array_element_size(arr);
    uint8_t* dest_ptr = reinterpret_cast<uint8_t*>(const_cast<uint64_t*>(&arr->first_data)) + ele_size * static_cast<size_t>(index);
    const uint8_t* src_ptr = static_cast<const uint8_t*>(value);
    std::memcpy(dest_ptr, src_ptr, ele_size);
    RET_VOID_OK();
}

/// @intrinsic: System.Array::get_Length
static RtResultVoid get_length_invoker_intrinsics_system_array(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtArray* arr = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, length, SystemArray::get_length(arr));
    interp::EvalStackOp::set_return(ret, length);
    RET_VOID_OK();
}

/// @intrinsic: System.Array::get_LongLength
static RtResultVoid get_long_length_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtArray* arr = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, length, SystemArray::get_long_length(arr));
    interp::EvalStackOp::set_return(ret, length);
    RET_VOID_OK();
}

/// @intrinsic: System.Array::GetGenericValueImpl<>
static RtResultVoid get_generic_value_impl_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtArray* arr = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);
    int32_t index = interp::EvalStackOp::get_param<int32_t>(params, 1);
    void* value_ptr = interp::EvalStackOp::get_param<void*>(params, 2);

    RET_ERR_ON_FAIL(SystemArray::get_generic_value_impl(arr, index, value_ptr));
    RET_VOID_OK();
}

/// @intrinsic: System.Array::SetGenericValueImpl<>
static RtResultVoid set_generic_value_impl_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtArray* arr = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);
    int32_t index = interp::EvalStackOp::get_param<int32_t>(params, 1);
    void* value_ptr = interp::EvalStackOp::get_param<void*>(params, 2);

    RET_ERR_ON_FAIL(SystemArray::set_generic_value_impl(arr, index, value_ptr));
    RET_VOID_OK();
}

// Intrinsic registry
static vm::IntrinsicEntry s_intrinsic_entries_system_array[] = {
    {"System.Array::get_Length", (vm::IntrinsicFunction)&SystemArray::get_length, get_length_invoker_intrinsics_system_array},
    {"System.Array::get_LongLength", (vm::IntrinsicFunction)&SystemArray::get_long_length, get_long_length_invoker},
    {"System.Array::GetGenericValueImpl<>", (vm::IntrinsicFunction)&SystemArray::get_generic_value_impl, get_generic_value_impl_invoker},
    {"System.Array::SetGenericValueImpl<>", (vm::IntrinsicFunction)&SystemArray::set_generic_value_impl, set_generic_value_impl_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemArray::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_array, sizeof(s_intrinsic_entries_system_array) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

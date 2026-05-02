#include "system_array.h"
#include "icall_base.h"
#include "vm/rt_array.h"
#include "vm/object.h"
#include "vm/class.h"
#include "vm/array_class.h"
#include <cstring>

namespace leanclr
{
namespace icalls
{

/// @icall: System.Array::GetRank
RtResult<int32_t> SystemArray::get_rank(vm::RtArray* arr) noexcept
{
    if (arr == nullptr)
        RET_ERR(RtErr::NullReference);

    const metadata::RtClass* klass = arr->klass;
    assert(vm::Class::is_array_or_szarray(klass));
    RET_OK(static_cast<int32_t>(vm::Class::get_rank(klass)));
}

static RtResultVoid get_rank_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, rank, SystemArray::get_rank(arr));
    EvalStackOp::set_return(ret, rank);
    RET_VOID_OK();
}

/// @icall: System.Array::GetLength
RtResult<int32_t> SystemArray::get_length(vm::RtArray* arr, int32_t dimension) noexcept
{
    return vm::Array::get_array_length_at_dimension(arr, static_cast<size_t>(dimension));
}

static RtResultVoid get_length_invoker_icalls_system_array(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                           interp::RtStackObject* ret) noexcept
{
    auto arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    int32_t dimension = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, length, SystemArray::get_length(arr, dimension));
    EvalStackOp::set_return(ret, length);
    RET_VOID_OK();
}

/// @icall: System.Array::GetLowerBound
RtResult<int32_t> SystemArray::get_lower_bound(vm::RtArray* arr, int32_t dimension) noexcept
{
    return vm::Array::get_array_lower_bound_at_dimension(arr, static_cast<size_t>(dimension));
}

static RtResultVoid get_lower_bound_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                            interp::RtStackObject* ret) noexcept
{
    auto arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    int32_t dimension = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, lower_bound, SystemArray::get_lower_bound(arr, dimension));
    EvalStackOp::set_return(ret, lower_bound);
    RET_VOID_OK();
}

/// @icall: System.Array::GetValue(System.Int32[])
RtResult<vm::RtObject*> SystemArray::get_value(vm::RtArray* arr, vm::RtArray* indices) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, index, vm::Array::get_global_index_from_indices(arr, indices));
    return get_value_impl(arr, index);
}

static RtResultVoid get_array_value_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                            interp::RtStackObject* ret) noexcept
{
    auto arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    auto indices = EvalStackOp::get_param<vm::RtArray*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, value, SystemArray::get_value(arr, indices));
    EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @icall: System.Array::SetValue(System.Object,System.Int32[])
RtResultVoid SystemArray::set_value(vm::RtArray* arr, vm::RtObject* value, vm::RtArray* indices) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, index, vm::Array::get_global_index_from_indices(arr, indices));
    return set_value_impl(arr, value, index);
}

static RtResultVoid set_value_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject*) noexcept
{
    auto arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    auto value = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    auto indices = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    RET_ERR_ON_FAIL(SystemArray::set_value(arr, value, indices));
    RET_VOID_OK();
}

/// @icall: System.Array::GetValueImpl
RtResult<vm::RtObject*> SystemArray::get_value_impl(vm::RtArray* arr, int32_t global_index) noexcept
{
    if (static_cast<uint32_t>(global_index) >= static_cast<uint32_t>(arr->length))
        RET_ERR(RtErr::IndexOutOfRange);

    const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(arr);

    if (vm::Class::is_value_type(ele_klass))
    {
        // Box value type
        size_t ele_size = vm::Array::get_array_element_size(arr);
        const void* ele_ptr = static_cast<const uint8_t*>(vm::Array::get_array_data_start_as_ptr_void(arr)) + static_cast<size_t>(global_index) * ele_size;
        return vm::Object::box_object(ele_klass, ele_ptr);
    }
    else
    {
        // Return reference type directly
        vm::RtObject* obj = vm::Array::get_array_data_at<vm::RtObject*>(arr, global_index);
        RET_OK(obj);
    }
}

static RtResultVoid get_value_impl_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    auto arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    int32_t global_index = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, value, SystemArray::get_value_impl(arr, global_index));
    EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

/// @icall: System.Array::SetValueImpl
RtResultVoid SystemArray::set_value_impl(vm::RtArray* arr, vm::RtObject* value, int32_t global_index) noexcept
{
    if (static_cast<uint32_t>(global_index) >= static_cast<uint32_t>(arr->length))
        RET_ERR(RtErr::IndexOutOfRange);

    const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(arr);

    if (vm::Class::is_value_type(ele_klass))
    {
        // Unbox value type
        if (value == nullptr)
            RET_ERR(RtErr::NullReference);

        void* ele_ptr = vm::Array::get_array_element_address_as_ptr_void(arr, global_index);
        return vm::Object::unbox_any(value, ele_klass, ele_ptr, false);
    }
    else
    {
        // Set reference type directly
        vm::Array::set_array_data_at<vm::RtObject*>(arr, global_index, value);
        RET_VOID_OK();
    }
}

static RtResultVoid set_value_impl_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject*) noexcept
{
    auto arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    auto value = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    int32_t global_index = EvalStackOp::get_param<int32_t>(params, 2);
    RET_ERR_ON_FAIL(SystemArray::set_value_impl(arr, value, global_index));
    RET_VOID_OK();
}

/// @icall: System.Array::FastCopy
RtResult<bool> SystemArray::fast_copy(vm::RtArray* src, int32_t src_index, vm::RtArray* dst, int32_t dst_index, int32_t length) noexcept
{
    uint32_t src_length = static_cast<uint32_t>(src->length);
    uint32_t dst_length = static_cast<uint32_t>(dst->length);

    // Check bounds
    if (static_cast<uint32_t>(src_index) > src_length || static_cast<uint32_t>(dst_index) > dst_length ||
        static_cast<uint32_t>(length) > src_length - static_cast<uint32_t>(src_index) ||
        static_cast<uint32_t>(length) > dst_length - static_cast<uint32_t>(dst_index))
    {
        RET_ERR(RtErr::IndexOutOfRange);
    }

    const metadata::RtClass* src_klass = src->klass;
    const metadata::RtClass* dst_klass = dst->klass;

    // Fast path: same class
    if (src_klass == dst_klass)
    {
        size_t ele_size = vm::Array::get_array_element_size(src);
        const void* src_data_ptr = vm::Array::get_array_data_start_as_ptr_void(src);
        void* dst_data_ptr = vm::Array::get_array_data_start_as_ptr_void(dst);

        if (src == dst)
        {
            if (src_index != dst_index)
            {
                // Overlapping copy
                std::memmove(static_cast<uint8_t*>(dst_data_ptr) + static_cast<size_t>(dst_index) * ele_size,
                             static_cast<const uint8_t*>(src_data_ptr) + static_cast<size_t>(src_index) * ele_size, static_cast<size_t>(length) * ele_size);
            }
        }
        else
        {
            // Non-overlapping copy
            std::memcpy(static_cast<uint8_t*>(dst_data_ptr) + static_cast<size_t>(dst_index) * ele_size,
                        static_cast<const uint8_t*>(src_data_ptr) + static_cast<size_t>(src_index) * ele_size, static_cast<size_t>(length) * ele_size);
        }
        RET_OK(true);
    }

    // Check element sizes
    size_t src_ele_size = vm::Array::get_array_element_size(src);
    size_t dst_ele_size = vm::Array::get_array_element_size(dst);
    if (src_ele_size != dst_ele_size)
        RET_OK(false);

    const metadata::RtClass* src_ele_klass = vm::Array::get_array_element_class(src);
    const metadata::RtClass* dst_ele_klass = vm::Array::get_array_element_class(dst);

    const void* src_data_ptr = vm::Array::get_array_data_start_as_ptr_void(src);
    void* dst_data_ptr = vm::Array::get_array_data_start_as_ptr_void(dst);

    if (vm::Class::is_value_type(src_ele_klass))
    {
        // Value types must be exact match
        if (src_ele_klass != dst_ele_klass)
            RET_OK(false);

        std::memcpy(static_cast<uint8_t*>(dst_data_ptr) + static_cast<size_t>(dst_index) * src_ele_size,
                    static_cast<const uint8_t*>(src_data_ptr) + static_cast<size_t>(src_index) * src_ele_size, static_cast<size_t>(length) * src_ele_size);
    }
    else
    {
        // Reference types: check assignability
        if (!vm::Class::is_assignable_from(src_ele_klass, dst_ele_klass))
            RET_OK(false);

        std::memcpy(static_cast<vm::RtObject**>(dst_data_ptr) + static_cast<size_t>(dst_index),
                    static_cast<vm::RtObject* const*>(src_data_ptr) + static_cast<size_t>(src_index), static_cast<size_t>(length) * sizeof(vm::RtObject*));
    }

    RET_OK(true);
}

static RtResultVoid fast_copy_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto src = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    int32_t src_index = EvalStackOp::get_param<int32_t>(params, 1);
    auto dst = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    int32_t dst_index = EvalStackOp::get_param<int32_t>(params, 3);
    int32_t length = EvalStackOp::get_param<int32_t>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemArray::fast_copy(src, src_index, dst, dst_index, length));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.Array::CreateInstanceImpl
RtResult<vm::RtArray*> SystemArray::create_instance_impl(vm::RtReflectionType* ele_ref_type, vm::RtArray* lengths, vm::RtArray* lower_bounds) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, ele_klass, vm::Class::get_class_from_typesig(ele_ref_type->type_handle));

    size_t dimension;
    if (lengths == nullptr)
    {
        if (lower_bounds == nullptr)
            RET_ERR(RtErr::Argument);
        dimension = static_cast<size_t>(lower_bounds->length);
    }
    else
    {
        dimension = static_cast<size_t>(lengths->length);
        if (lower_bounds != nullptr && static_cast<size_t>(lower_bounds->length) != dimension)
            RET_ERR(RtErr::Argument);
    }

    if (dimension == 0 || dimension > metadata::RT_MAX_ARRAY_RANK)
        RET_ERR(RtErr::Argument);

    const int32_t* length_indices = (lengths != nullptr) ? vm::Array::get_array_data_start_as<int32_t>(lengths) : nullptr;
    const int32_t* lower_bound_indices = (lower_bounds != nullptr) ? vm::Array::get_array_data_start_as<int32_t>(lower_bounds) : nullptr;

    // Single-dimensional zero-based array
    if (dimension == 1 && (lower_bounds == nullptr || vm::Array::get_array_data_at<int32_t>(lower_bounds, 0) == 0))
    {
        return vm::Array::new_szarray_from_ele_klass(ele_klass, *length_indices);
    }
    else
    {
        // Multi-dimensional or non-zero-based array
        return vm::Array::new_mdarray_from_ele_klass(ele_klass, static_cast<uint8_t>(dimension), length_indices, lower_bound_indices);
    }
}

static RtResultVoid create_instance_impl_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    auto klass = EvalStackOp::get_param<vm::RtReflectionType*>(params, 0);
    auto length_arr = EvalStackOp::get_param<vm::RtArray*>(params, 1);
    auto lower_bounds_arr = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, SystemArray::create_instance_impl(klass, length_arr, lower_bounds_arr));
    EvalStackOp::set_return(ret, arr);
    RET_VOID_OK();
}

/// @icall: System.Array::ClearInternal
RtResultVoid SystemArray::clear_internal(vm::RtArray* arr, int32_t index, int32_t length) noexcept
{
    uint32_t arr_length = static_cast<uint32_t>(arr->length);
    if (static_cast<uint32_t>(index) >= arr_length || static_cast<uint32_t>(length) > arr_length - static_cast<uint32_t>(index))
    {
        RET_ERR(RtErr::IndexOutOfRange);
    }

    const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(arr);
    void* arr_data_ptr = vm::Array::get_array_data_start_as_ptr_void(arr);

    if (vm::Class::is_value_type(ele_klass))
    {
        size_t ele_size = vm::Array::get_array_element_size(arr);
        std::memset(static_cast<uint8_t*>(arr_data_ptr) + static_cast<size_t>(index) * ele_size, 0, static_cast<size_t>(length) * ele_size);
    }
    else
    {
        std::memset(static_cast<vm::RtObject**>(arr_data_ptr) + static_cast<size_t>(index), 0, static_cast<size_t>(length) * sizeof(vm::RtObject*));
    }

    RET_VOID_OK();
}

static RtResultVoid clear_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject*) noexcept
{
    auto arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    int32_t index = EvalStackOp::get_param<int32_t>(params, 1);
    int32_t length = EvalStackOp::get_param<int32_t>(params, 2);
    RET_ERR_ON_FAIL(SystemArray::clear_internal(arr, index, length));
    RET_VOID_OK();
}

// Internal call registry
static vm::InternalCallEntry s_internal_call_entries_system_array[] = {
    {"System.Array::GetRank", (vm::InternalCallFunction)&SystemArray::get_rank, get_rank_invoker},
    {"System.Array::GetLength", (vm::InternalCallFunction)&SystemArray::get_length, get_length_invoker_icalls_system_array},
    {"System.Array::GetLowerBound", (vm::InternalCallFunction)&SystemArray::get_lower_bound, get_lower_bound_invoker},
    {"System.Array::GetValue(System.Int32[])", (vm::InternalCallFunction)&SystemArray::get_value, get_array_value_invoker},
    {"System.Array::SetValue(System.Object,System.Int32[])", (vm::InternalCallFunction)&SystemArray::set_value, set_value_invoker},
    {"System.Array::GetValueImpl", (vm::InternalCallFunction)&SystemArray::get_value_impl, get_value_impl_invoker},
    {"System.Array::SetValueImpl", (vm::InternalCallFunction)&SystemArray::set_value_impl, set_value_impl_invoker},
    {"System.Array::FastCopy", (vm::InternalCallFunction)&SystemArray::fast_copy, fast_copy_invoker},
    {"System.Array::CreateInstanceImpl", (vm::InternalCallFunction)&SystemArray::create_instance_impl, create_instance_impl_invoker},
    {"System.Array::ClearInternal", (vm::InternalCallFunction)&SystemArray::clear_internal, clear_internal_invoker},
};

utils::Span<vm::InternalCallEntry> SystemArray::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_array,
                                              sizeof(s_internal_call_entries_system_array) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

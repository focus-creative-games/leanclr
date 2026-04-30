#include "rt_array.h"
#include "class.h"
#include "object.h"
#include "array_class.h"
#include "gc/garbage_collector.h"
#include "utils/mem_op.h"
#include "rt_managed_types.h"
#include "interp/eval_stack_op.h"

namespace leanclr
{
namespace vm
{

// Constants

// Helper: Calculate total byte size for array including elements
size_t Array::get_array_allocation_size(const metadata::RtClass* klass, int32_t length)
{
    assert(klass && klass->element_class);
    size_t element_size = Class::get_stack_location_size(klass->element_class);
    return sizeof(RtArray) - 8 + static_cast<size_t>(length) * element_size;
}

// Array creation methods

RtResult<RtArray*> Array::new_empty_szarray_by_ele_klass(const metadata::RtClass* ele_class)
{
    return new_szarray_from_ele_klass(ele_class, 0);
}

RtResult<RtArray*> Array::new_szarray_from_array_klass(const metadata::RtClass* klass, int32_t length)
{
    assert(klass);

    RET_ERR_ON_FAIL(Class::initialize_all(const_cast<metadata::RtClass*>(klass)));

    size_t arr_length = get_array_allocation_size(klass, length);
    RtArray* arr_obj = reinterpret_cast<RtArray*>(gc::GarbageCollector::allocate_array(klass, arr_length));

    if (!arr_obj)
    {
        RET_ERR(RtErr::OutOfMemory);
    }

    arr_obj->length = length;
    RET_OK(arr_obj);
}

RtResult<RtArray*> Array::new_szarray_from_ele_klass(const metadata::RtClass* ele_class, int32_t length)
{
    assert(ele_class);
    if (length < 0)
    {
        RET_ERR(RtErr::Overflow);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, ArrayClass::get_szarray_class_from_element_class(ele_class));
    RET_ERR_ON_FAIL(Class::initialize_all(klass));

    size_t arr_length = get_array_allocation_size(klass, length);
    RtArray* arr_obj = reinterpret_cast<RtArray*>(gc::GarbageCollector::allocate_array(klass, arr_length));

    if (!arr_obj)
    {
        RET_ERR(RtErr::OutOfMemory);
    }

    arr_obj->length = length;
    RET_OK(arr_obj);
}

RtResult<RtArray*> Array::new_mdarray_from_array_klass(const metadata::RtClass* arr_klass, const int32_t* lengths, const int32_t* lower_bounds)
{
    assert(arr_klass && lengths);

    // Verify it's an array type
    assert(arr_klass->by_val->ele_type == metadata::RtElementType::Array);

    RET_ERR_ON_FAIL(Class::initialize_all(const_cast<metadata::RtClass*>(arr_klass)));

    const metadata::RtArrayType* arr_type = arr_klass->by_val->data.array_type;
    uint8_t rank = arr_type->rank;

    // Calculate total length by multiplying all dimensions
    int32_t total_length = 1;
    for (uint8_t i = 0; i < rank; ++i)
    {
        int32_t dimension_length = lengths[i];
        // Check for overflow
        if (static_cast<uint32_t>(dimension_length) > static_cast<uint32_t>(RT_MAX_ARRAY_INDEX) / static_cast<uint32_t>(total_length))
        {
            RET_ERR(RtErr::Overflow);
        }
        total_length *= dimension_length;
    }

    // Calculate data size
    const metadata::RtClass* ele_klass = arr_klass->element_class;
    int32_t ele_size = static_cast<int32_t>(Class::get_stack_location_size(ele_klass));

    if (ele_size > RT_MAX_ARRAY_INDEX / total_length)
    {
        RET_ERR(RtErr::Overflow);
    }

    int32_t total_data_bytes = total_length * ele_size;

    // Calculate layout: array header + data + bounds array
    size_t arr_total_bytes_without_bounds = get_array_allocation_size(arr_klass, 0) + static_cast<size_t>(total_data_bytes);
    size_t bounds_start_index = utils::MemOp::align_up(arr_total_bytes_without_bounds, 8);
    size_t total_array_bytes = bounds_start_index + sizeof(ArrayBounds) * rank;

    RtArray* arr_obj = reinterpret_cast<RtArray*>(gc::GarbageCollector::allocate_array(arr_klass, total_array_bytes));

    // Set up bounds
    ArrayBounds* bounds = reinterpret_cast<ArrayBounds*>(reinterpret_cast<uint8_t*>(arr_obj) + bounds_start_index);
    arr_obj->bounds = bounds;

    for (uint8_t i = 0; i < rank; ++i)
    {
        bounds[i].length = lengths[i];
        bounds[i].lower_bound = (lower_bounds != nullptr) ? lower_bounds[i] : 0;
    }

    arr_obj->length = total_length;
    RET_OK(arr_obj);
}

RtResult<RtArray*> Array::new_mdarray_from_ele_klass(const metadata::RtClass* ele_klass, int32_t rank, const int32_t* lengths, const int32_t* lower_bounds)
{
    assert(ele_klass && lengths);
    if (rank < 1 || rank > RT_MAX_ARRAY_RANK)
    {
        RET_ERR(RtErr::Overflow);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, arr_klass, ArrayClass::get_array_class_from_element_klass(ele_klass, (uint8_t)rank));
    return new_mdarray_from_array_klass(arr_klass, lengths, lower_bounds);
}

// Array information methods

size_t Array::get_array_byte_length(const RtArray* array)
{
    assert(array);
    size_t ele_size = get_array_element_size(array);
    int32_t length = get_array_length(array);
    return ele_size * static_cast<size_t>(length);
}

size_t Array::get_array_element_size(const RtArray* array)
{
    assert(array);
    const metadata::RtClass* ele_class = get_array_element_class(array);
    return Class::get_stack_location_size(ele_class);
}

size_t Array::get_array_element_size_by_klass(const metadata::RtClass* array_klass)
{
    assert(array_klass && array_klass->element_class);
    return Class::get_stack_location_size(array_klass->element_class);
}

// Array data access methods

void* Array::get_array_data_start_as_ptr_void(RtArray* array)
{
    assert(array);
    return &array->first_data;
}

void* Array::get_array_element_address_as_ptr_void(RtArray* array, int32_t index)
{
    assert(array);
    assert(index >= 0 && index < get_array_length(array));
    size_t ele_size = get_array_element_size(array);
    return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(&array->first_data) + ele_size * static_cast<size_t>(index));
}

void* Array::get_array_element_address_with_size_as_ptr_void(RtArray* array, int32_t index, size_t ele_size)
{
    assert(array);
    assert(index >= 0 && index < get_array_length(array));
    return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(&array->first_data) + ele_size * static_cast<size_t>(index));
}

void Array::copy_array_data_to_no_eval_stack(const RtArray* arr, int32_t start_index, void* dest)
{
    assert(arr && dest);
    assert(start_index >= 0 && start_index < get_array_length(arr));
    size_t ele_size = get_array_element_size(arr);
    const uint8_t* src_ptr = reinterpret_cast<const uint8_t*>(&arr->first_data) + ele_size * static_cast<size_t>(start_index);
    std::memcpy(dest, src_ptr, ele_size);
}

// Multi-dimensional array methods

RtResult<int32_t> Array::get_array_length_at_dimension(const RtArray* array, size_t dimension)
{
    assert(array);

    const metadata::RtClass* klass = array->klass;
    const metadata::RtTypeSig* type_sig = klass->by_val;

    switch (type_sig->ele_type)
    {
    case metadata::RtElementType::SZArray:
        if (dimension > 0)
        {
            RET_ERR(RtErr::IndexOutOfRange);
        }
        RET_OK(get_array_length(array));

    case metadata::RtElementType::Array:
    {
        const metadata::RtArrayType* arr_type = type_sig->data.array_type;
        uint8_t rank = arr_type->rank;
        if (dimension >= rank)
        {
            RET_ERR(RtErr::IndexOutOfRange);
        }
        RET_OK(array->bounds[dimension].length);
    }

    default:
        RET_ERR(RtErr::InvalidCast);
    }
}

RtResult<int32_t> Array::get_array_lower_bound_at_dimension(const RtArray* array, size_t dimension)
{
    assert(array);

    const metadata::RtClass* klass = array->klass;
    const metadata::RtTypeSig* type_sig = klass->by_val;

    switch (type_sig->ele_type)
    {
    case metadata::RtElementType::SZArray:
        if (dimension > 0)
        {
            RET_ERR(RtErr::IndexOutOfRange);
        }
        RET_OK(0);

    case metadata::RtElementType::Array:
    {
        const metadata::RtArrayType* arr_type = type_sig->data.array_type;
        uint8_t rank = arr_type->rank;
        if (dimension >= rank)
        {
            RET_ERR(RtErr::IndexOutOfRange);
        }
        RET_OK(array->bounds[dimension].lower_bound);
    }

    default:
        RET_ERR(RtErr::InvalidCast);
    }
}

RtResult<int32_t> Array::get_global_index_from_indices(const RtArray* arr, RtArray* indices)
{
    assert(arr && indices);

    const metadata::RtClass* indice_klass = indices->klass;
    assert(indice_klass->by_val->ele_type == metadata::RtElementType::SZArray);

    int32_t indice_length = get_array_length(indices);
    const metadata::RtClass* klass = arr->klass;
    const metadata::RtTypeSig* type_sig = klass->by_val;

    int32_t index = 0;

    switch (type_sig->ele_type)
    {
    case metadata::RtElementType::SZArray:
        if (indice_length != 1)
        {
            RET_ERR(RtErr::ArgumentNull);
        }
        index = get_array_data_at<int32_t>(indices, 0);
        break;

    case metadata::RtElementType::Array:
    {
        const metadata::RtArrayType* arr_type = type_sig->data.array_type;
        if (arr_type->rank != indice_length)
        {
            RET_ERR(RtErr::ArgumentNull);
        }

        int32_t offset = 0;
        const ArrayBounds* bounds = arr->bounds;

        for (int32_t i = 0; i < indice_length; ++i)
        {
            int32_t idx = get_array_data_at<int32_t>(indices, i);
            const ArrayBounds* bound = &bounds[i];
            int32_t index_relate_to_lower = idx - bound->lower_bound;

            if ((uint32_t)index_relate_to_lower >= (uint32_t)bound->length)
            {
                RET_ERR(RtErr::IndexOutOfRange);
            }

            offset = offset * bound->length + index_relate_to_lower;
        }
        index = offset;
        break;
    }

    default:
        RET_ERR(RtErr::InvalidCast);
    }

    RET_OK(index);
}

RtResult<int32_t> Array::get_mdarray_global_index_from_indices2(const RtArray* arr, const interp::RtStackObject* indices)
{
    assert(arr && indices);

    const metadata::RtClass* klass = arr->klass;
    const metadata::RtTypeSig* type_sig = klass->by_val;

    assert(type_sig->ele_type == metadata::RtElementType::Array);

    const metadata::RtArrayType* arr_type = type_sig->data.array_type;
    uint8_t rank = arr_type->rank;

    int32_t offset = 0;
    const ArrayBounds* bounds = arr->bounds;

    for (uint8_t i = 0; i < rank; ++i)
    {
        int32_t idx = indices[i].i32;
        const ArrayBounds* bound = &bounds[i];
        int32_t index_relate_to_lower = idx - bound->lower_bound;

        if ((uint32_t)index_relate_to_lower >= (uint32_t)bound->length)
        {
            RET_ERR(RtErr::IndexOutOfRange);
        }

        offset = offset * bound->length + index_relate_to_lower;
    }

    RET_OK(offset);
}

RtResult<int32_t> Array::get_mdarray_global_index_from_indices3(const RtArray* arr, int32_t* indices)
{
    assert(arr && indices);

    const metadata::RtClass* klass = arr->klass;
    const metadata::RtTypeSig* type_sig = klass->by_val;

    assert(type_sig->ele_type == metadata::RtElementType::Array);

    const metadata::RtArrayType* arr_type = type_sig->data.array_type;
    uint8_t rank = arr_type->rank;

    int32_t offset = 0;
    const ArrayBounds* bounds = arr->bounds;

    for (uint8_t i = 0; i < rank; ++i)
    {
        int32_t idx = indices[i];
        const ArrayBounds* bound = &bounds[i];
        int32_t index_relate_to_lower = idx - bound->lower_bound;

        if ((uint32_t)index_relate_to_lower >= (uint32_t)bound->length)
        {
            RET_ERR(RtErr::IndexOutOfRange);
        }

        offset = offset * bound->length + index_relate_to_lower;
    }

    RET_OK(offset);
}

// Method invoker implementations

RtResultVoid Array::szarray_new_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    assert(method && params && ret);
    assert(method->parameter_count == 1);

    int32_t length = interp::EvalStackOp::get_param<int32_t>(params, 0);
    metadata::RtClass* klass = const_cast<metadata::RtClass*>(method->parent);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, arr, new_szarray_from_array_klass(klass, length));
    interp::EvalStackOp::set_return(ret, arr);
    RET_VOID_OK();
}

RtResultVoid Array::szarray_get_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    assert(method && params && ret);
    assert(method->parameter_count == 1);

    RtArray* arr = interp::EvalStackOp::get_param<RtArray*>(params, 0);
    int32_t index = interp::EvalStackOp::get_param<int32_t>(params, 1);
    if ((uint32_t)index >= (uint32_t)get_array_length(arr))
    {
        RET_ERR(RtErr::IndexOutOfRange);
    }
    const void* data_ptr = get_array_element_address_as_ptr_void(arr, index);
    const metadata::RtClass* ele_class = get_array_element_class(arr);

    Object::extends_to_eval_stack(data_ptr, ret, ele_class);

    RET_VOID_OK();
}

RtResultVoid Array::szarray_set_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    assert(method && params);
    assert(method->parameter_count == 2);

    RtArray* arr = interp::EvalStackOp::get_param<RtArray*>(params, 0);
    int32_t index = interp::EvalStackOp::get_param<int32_t>(params, 1);
    if ((uint32_t)index >= (uint32_t)get_array_length(arr))
    {
        RET_ERR(RtErr::IndexOutOfRange);
    }
    size_t element_size = get_array_element_size(arr);
    const uint8_t* value_ptr = reinterpret_cast<const uint8_t*>(params + 2);

    uint8_t* data_ptr = reinterpret_cast<uint8_t*>(const_cast<uint64_t*>(&arr->first_data)) + element_size * static_cast<size_t>(index);
    std::memcpy(data_ptr, value_ptr, element_size);

    RET_VOID_OK();
}

RtResultVoid Array::szarray_address_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    assert(method && params && ret);
    assert(method->parameter_count == 1);

    RtArray* arr = interp::EvalStackOp::get_param<RtArray*>(params, 0);
    int32_t index = interp::EvalStackOp::get_param<int32_t>(params, 1);
    if ((uint32_t)index >= (uint32_t)get_array_length(arr))
    {
        RET_ERR(RtErr::IndexOutOfRange);
    }
    const void* data_ptr = get_array_element_address_as_ptr_void(arr, index);

    interp::EvalStackOp::set_return(ret, data_ptr);
    RET_VOID_OK();
}

RtResultVoid Array::newmdarray_lengths_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    assert(method && params && ret);

    size_t param_count = method->parameter_count;

    // Allocate temporary buffer for lengths
    int32_t* i32_lengths = static_cast<int32_t*>(alloca(param_count * sizeof(int32_t)));
    for (size_t i = 0; i < param_count; ++i)
    {
        i32_lengths[i] = interp::EvalStackOp::get_param<int32_t>(params, i);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, arr, new_mdarray_from_array_klass(const_cast<metadata::RtClass*>(method->parent), i32_lengths, nullptr));
    interp::EvalStackOp::set_return(ret, arr);
    RET_VOID_OK();
}

RtResultVoid Array::newmdarray_lengths_lower_bounds_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    assert(method && params && ret);

    size_t param_count = method->parameter_count / 2;

    int32_t* i32_lengths = static_cast<int32_t*>(alloca(param_count * sizeof(int32_t)));
    int32_t* i32_lower_bounds = static_cast<int32_t*>(alloca(param_count * sizeof(int32_t)));

    for (size_t i = 0; i < param_count; ++i)
    {
        i32_lengths[i] = interp::EvalStackOp::get_param<int32_t>(params, i);
        i32_lower_bounds[i] = interp::EvalStackOp::get_param<int32_t>(params, i + param_count);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, arr,
                                            new_mdarray_from_array_klass(const_cast<metadata::RtClass*>(method->parent), i32_lengths, i32_lower_bounds));
    interp::EvalStackOp::set_return(ret, arr);

    RET_VOID_OK();
}

RtResultVoid Array::mdarray_get_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    assert(params && ret);

    RtArray* arr = interp::EvalStackOp::get_param<RtArray*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, index, get_mdarray_global_index_from_indices2(arr, params + 1));

    const void* data_ptr = get_array_element_address_as_ptr_void(arr, index);
    const metadata::RtClass* ele_class = get_array_element_class(arr);

    Object::extends_to_eval_stack(data_ptr, ret, ele_class);

    RET_VOID_OK();
}

RtResultVoid Array::mdarray_set_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    assert(method && params);

    RtArray* arr = interp::EvalStackOp::get_param<RtArray*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, index, get_mdarray_global_index_from_indices2(arr, params + 1));

    size_t element_size = get_array_element_size(arr);
    const uint8_t* value_ptr = reinterpret_cast<const uint8_t*>(params + method->parameter_count);

    uint8_t* data_ptr = reinterpret_cast<uint8_t*>(const_cast<uint64_t*>(&arr->first_data)) + element_size * static_cast<size_t>(index);
    std::memcpy(data_ptr, value_ptr, element_size);

    RET_VOID_OK();
}

RtResultVoid Array::mdarray_address_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    assert(params && ret);

    RtArray* arr = interp::EvalStackOp::get_param<RtArray*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, index, get_mdarray_global_index_from_indices2(arr, params + 1));

    const void* data_ptr = get_array_element_address_as_ptr_void(arr, index);
    interp::EvalStackOp::set_return(ret, data_ptr);
    RET_VOID_OK();
}

// Array cloning

RtResult<RtArray*> Array::clone(RtArray* old_arr) noexcept
{
    assert(old_arr);

    size_t total_bytes = get_array_allocation_size(old_arr->klass, old_arr->length);
    RtArray* new_arr = reinterpret_cast<RtArray*>(gc::GarbageCollector::allocate_array(old_arr->klass, total_bytes));

    if (!new_arr)
    {
        RET_ERR(RtErr::OutOfMemory);
    }
    std::memcpy(new_arr, old_arr, total_bytes);

    // new_arr->length = old_arr->length;
    // size_t ele_size = get_array_element_size(old_arr);

    // std::memcpy(const_cast<uint64_t*>(&new_arr->first_data), &old_arr->first_data, ele_size * old_arr->length);

    RET_OK(new_arr);
}

} // namespace vm
} // namespace leanclr

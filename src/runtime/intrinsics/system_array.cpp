#include "system_array.h"

#include <cstring>

#include "interp/eval_stack_op.h"
#include "vm/class.h"
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
    uint8_t* dest_ptr = static_cast<uint8_t*>(vm::Array::get_array_data_start_as_ptr_void(arr)) + ele_size * static_cast<size_t>(index);
    const uint8_t* src_ptr = static_cast<const uint8_t*>(value);
    std::memcpy(dest_ptr, src_ptr, ele_size);
    RET_VOID_OK();
}

static metadata::RtElementType get_normalized_integral_array_element_type(metadata::RtElementType element_type) noexcept
{
    switch (element_type)
    {
    case metadata::RtElementType::U1:
        return metadata::RtElementType::I1;
    case metadata::RtElementType::U2:
        return metadata::RtElementType::I2;
    case metadata::RtElementType::U4:
        return metadata::RtElementType::I4;
    case metadata::RtElementType::U8:
        return metadata::RtElementType::I8;
    case metadata::RtElementType::U:
        return metadata::RtElementType::I;
    default:
        return element_type;
    }
}

static metadata::RtElementType get_array_copy_element_type(const metadata::RtClass* klass) noexcept
{
    if (vm::Class::is_enum_type(klass))
    {
        klass = klass->element_class;
    }

    switch (klass->by_val->ele_type)
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
        return get_normalized_integral_array_element_type(klass->by_val->ele_type);
    default:
        return klass->by_val->ele_type;
    }
}

static bool is_value_array_copy_compatible(const metadata::RtClass* source_element_class, const metadata::RtClass* destination_element_class) noexcept
{
    if (source_element_class == destination_element_class)
    {
        return true;
    }

    if (!vm::Class::is_enum_type(source_element_class) && !vm::Class::is_enum_type(destination_element_class))
    {
        return false;
    }

    return get_array_copy_element_type(source_element_class) == get_array_copy_element_type(destination_element_class);
}

RtResultVoid SystemArray::copy(vm::RtArray* source_array, int32_t source_index, vm::RtArray* destination_array, int32_t destination_index,
                               int32_t length) noexcept
{
    if (source_array == nullptr || destination_array == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    if (length < 0 || source_index < 0 || destination_index < 0)
    {
        RET_ERR(RtErr::ArgumentOutOfRange);
    }

    const uint32_t source_length = static_cast<uint32_t>(vm::Array::get_array_length(source_array));
    const uint32_t destination_length = static_cast<uint32_t>(vm::Array::get_array_length(destination_array));
    const uint32_t source_index_u = static_cast<uint32_t>(source_index);
    const uint32_t destination_index_u = static_cast<uint32_t>(destination_index);
    const uint32_t length_u = static_cast<uint32_t>(length);

    if (source_index_u > source_length || destination_index_u > destination_length || length_u > source_length - source_index_u ||
        length_u > destination_length - destination_index_u)
    {
        RET_ERR(RtErr::Argument);
    }
    if (length == 0)
    {
        RET_VOID_OK();
    }

    const metadata::RtClass* source_class = source_array->klass;
    const metadata::RtClass* destination_class = destination_array->klass;
    const metadata::RtClass* source_element_class = vm::Array::get_array_element_class(source_array);
    const metadata::RtClass* destination_element_class = vm::Array::get_array_element_class(destination_array);

    if (source_class != destination_class)
    {
        bool compatible = false;
        if (vm::Class::is_value_type(source_element_class) || vm::Class::is_value_type(destination_element_class))
        {
            compatible = is_value_array_copy_compatible(source_element_class, destination_element_class);
        }
        else
        {
            compatible = vm::Class::is_assignable_from(source_element_class, destination_element_class);
        }

        if (!compatible)
        {
            RET_ERR(RtErr::ArrayTypeMismatch);
        }
    }

    const size_t source_element_size = vm::Array::get_array_element_size(source_array);
    const size_t destination_element_size = vm::Array::get_array_element_size(destination_array);
    if (source_element_size != destination_element_size)
    {
        RET_ERR(RtErr::ArrayTypeMismatch);
    }

    const uint8_t* source = static_cast<const uint8_t*>(vm::Array::get_array_data_start_as_ptr_void(source_array)) +
                            static_cast<size_t>(source_index) * source_element_size;
    uint8_t* destination = static_cast<uint8_t*>(vm::Array::get_array_data_start_as_ptr_void(destination_array)) +
                           static_cast<size_t>(destination_index) * destination_element_size;
    std::memmove(destination, source, static_cast<size_t>(length) * source_element_size);
    RET_VOID_OK();
}

RtResultVoid SystemArray::copy(vm::RtArray* source_array, vm::RtArray* destination_array, int32_t length) noexcept
{
    return copy(source_array, 0, destination_array, 0, length);
}

RtResultVoid SystemArray::copy_indexed(vm::RtArray* source_array, int32_t source_index, vm::RtArray* destination_array,
                                       int32_t destination_index, int32_t length) noexcept
{
    return copy(source_array, source_index, destination_array, destination_index, length);
}

RtResultVoid SystemArray::copy_simple(vm::RtArray* source_array, vm::RtArray* destination_array, int32_t length) noexcept
{
    return copy(source_array, destination_array, length);
}

RtResultVoid SystemArray::clear(vm::RtArray* array, int32_t index, int32_t length) noexcept
{
    if (array == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    if (index < 0 || length < 0)
    {
        RET_ERR(RtErr::IndexOutOfRange);
    }

    const uint32_t array_length = static_cast<uint32_t>(vm::Array::get_array_length(array));
    const uint32_t index_u = static_cast<uint32_t>(index);
    const uint32_t length_u = static_cast<uint32_t>(length);
    if (index_u > array_length || length_u > array_length - index_u)
    {
        RET_ERR(RtErr::IndexOutOfRange);
    }
    if (length == 0)
    {
        RET_VOID_OK();
    }

    const size_t element_size = vm::Array::get_array_element_size(array);
    uint8_t* destination = static_cast<uint8_t*>(vm::Array::get_array_data_start_as_ptr_void(array)) +
                           static_cast<size_t>(index) * element_size;
    std::memset(destination, 0, static_cast<size_t>(length) * element_size);
    RET_VOID_OK();
}

RtResultVoid SystemArray::clear(vm::RtArray* array) noexcept
{
    if (array == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    return clear(array, 0, vm::Array::get_array_length(array));
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

/// @intrinsic: System.Array::Copy(System.Array,System.Int32,System.Array,System.Int32,System.Int32)
static RtResultVoid copy_indexed_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;

    vm::RtArray* source_array = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);
    int32_t source_index = interp::EvalStackOp::get_param<int32_t>(params, 1);
    vm::RtArray* destination_array = interp::EvalStackOp::get_param<vm::RtArray*>(params, 2);
    int32_t destination_index = interp::EvalStackOp::get_param<int32_t>(params, 3);
    int32_t length = interp::EvalStackOp::get_param<int32_t>(params, 4);

    RET_ERR_ON_FAIL(SystemArray::copy(source_array, source_index, destination_array, destination_index, length));
    RET_VOID_OK();
}

/// @intrinsic: System.Array::Copy(System.Array,System.Array,System.Int32)
static RtResultVoid copy_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;

    vm::RtArray* source_array = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);
    vm::RtArray* destination_array = interp::EvalStackOp::get_param<vm::RtArray*>(params, 1);
    int32_t length = interp::EvalStackOp::get_param<int32_t>(params, 2);

    RET_ERR_ON_FAIL(SystemArray::copy(source_array, destination_array, length));
    RET_VOID_OK();
}

/// @intrinsic: System.Array::Clear(System.Array,System.Int32,System.Int32)
static RtResultVoid clear_indexed_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                          const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;

    vm::RtArray* array = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);
    int32_t index = interp::EvalStackOp::get_param<int32_t>(params, 1);
    int32_t length = interp::EvalStackOp::get_param<int32_t>(params, 2);

    RET_ERR_ON_FAIL(SystemArray::clear(array, index, length));
    RET_VOID_OK();
}

/// @intrinsic: System.Array::Clear(System.Array)
static RtResultVoid clear_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;

    vm::RtArray* array = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);

    RET_ERR_ON_FAIL(SystemArray::clear(array));
    RET_VOID_OK();
}

// Intrinsic registry
static vm::IntrinsicEntry s_intrinsic_entries_system_array[] = {
    {"System.Array::get_Length()", (vm::IntrinsicFunction)&SystemArray::get_length, get_length_invoker_intrinsics_system_array},
    {"System.Array::get_Length", (vm::IntrinsicFunction)&SystemArray::get_length, get_length_invoker_intrinsics_system_array},
    {"System.Array::get_LongLength()", (vm::IntrinsicFunction)&SystemArray::get_long_length, get_long_length_invoker},
    {"System.Array::get_LongLength", (vm::IntrinsicFunction)&SystemArray::get_long_length, get_long_length_invoker},
    {"System.Array::GetGenericValueImpl<>", (vm::IntrinsicFunction)&SystemArray::get_generic_value_impl, get_generic_value_impl_invoker},
    {"System.Array::SetGenericValueImpl<>", (vm::IntrinsicFunction)&SystemArray::set_generic_value_impl, set_generic_value_impl_invoker},
    {"System.Array::Copy(System.Array,System.Int32,System.Array,System.Int32,System.Int32)",
     (vm::IntrinsicFunction)static_cast<RtResultVoid (*)(vm::RtArray*, int32_t, vm::RtArray*, int32_t, int32_t)>(&SystemArray::copy),
     copy_indexed_invoker},
    {"System.Array::Copy(System.Array,System.Array,System.Int32)",
     (vm::IntrinsicFunction)static_cast<RtResultVoid (*)(vm::RtArray*, vm::RtArray*, int32_t)>(&SystemArray::copy), copy_invoker},
    {"System.Array::Clear(System.Array,System.Int32,System.Int32)",
     (vm::IntrinsicFunction)static_cast<RtResultVoid (*)(vm::RtArray*, int32_t, int32_t)>(&SystemArray::clear), clear_indexed_invoker},
    {"System.Array::Clear(System.Array)", (vm::IntrinsicFunction)static_cast<RtResultVoid (*)(vm::RtArray*)>(&SystemArray::clear),
     clear_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemArray::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_array, sizeof(s_intrinsic_entries_system_array) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

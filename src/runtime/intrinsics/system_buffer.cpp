#include "system_buffer.h"

#include <climits>
#include <cstring>

#include "interp/eval_stack_op.h"
#include "metadata/rt_metadata.h"
#include "vm/class.h"
#include "vm/rt_array.h"

namespace leanclr
{
namespace intrinsics
{
namespace
{
static bool is_primitive_buffer_element_type(metadata::RtElementType element_type) noexcept
{
    switch (element_type)
    {
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::Char:
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

static RtResult<size_t> get_primitive_array_byte_length(vm::RtArray* array) noexcept
{
    if (array == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    const metadata::RtClass* element_class = vm::Array::get_array_element_class(array);
    const metadata::RtElementType element_type = vm::Class::get_element_type(element_class);
    if (!is_primitive_buffer_element_type(element_type))
    {
        RET_ERR(RtErr::Argument);
    }

    RET_OK(static_cast<size_t>(vm::Array::get_array_length(array)) * vm::Array::get_array_element_size(array));
}

static RtResultVoid validate_copy_range(size_t byte_length, int32_t offset, int32_t count) noexcept
{
    if (offset < 0 || count < 0)
    {
        RET_ERR(RtErr::ArgumentOutOfRange);
    }

    const size_t offset_size = static_cast<size_t>(offset);
    const size_t count_size = static_cast<size_t>(count);
    if (offset_size > byte_length || count_size > byte_length - offset_size)
    {
        RET_ERR(RtErr::Argument);
    }

    RET_VOID_OK();
}
} // namespace

RtResult<int32_t> SystemBuffer::byte_length(vm::RtArray* array) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, length, get_primitive_array_byte_length(array));
    if (length > static_cast<size_t>(INT32_MAX))
    {
        RET_ERR(RtErr::Overflow);
    }

    RET_OK(static_cast<int32_t>(length));
}

RtResultVoid SystemBuffer::block_copy(vm::RtArray* source, int32_t source_offset, vm::RtArray* destination, int32_t destination_offset,
                                      int32_t count) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, source_length, get_primitive_array_byte_length(source));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, destination_length, get_primitive_array_byte_length(destination));
    RET_ERR_ON_FAIL(validate_copy_range(source_length, source_offset, count));
    RET_ERR_ON_FAIL(validate_copy_range(destination_length, destination_offset, count));

    if (count == 0)
    {
        RET_VOID_OK();
    }

    const uint8_t* source_ptr = static_cast<const uint8_t*>(vm::Array::get_array_data_start_as_ptr_void(source)) + source_offset;
    uint8_t* destination_ptr = static_cast<uint8_t*>(vm::Array::get_array_data_start_as_ptr_void(destination)) + destination_offset;
    std::memmove(destination_ptr, source_ptr, static_cast<size_t>(count));
    RET_VOID_OK();
}

RtResultVoid SystemBuffer::memory_copy(const void* source, void* destination, uint64_t destination_size_in_bytes,
                                       uint64_t source_bytes_to_copy) noexcept
{
    if (source_bytes_to_copy > destination_size_in_bytes)
    {
        RET_ERR(RtErr::ArgumentOutOfRange);
    }
    if (source_bytes_to_copy == 0)
    {
        RET_VOID_OK();
    }
    if (source == nullptr || destination == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    std::memmove(destination, source, static_cast<size_t>(source_bytes_to_copy));
    RET_VOID_OK();
}

static RtResultVoid byte_length_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    vm::RtArray* array = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, length, SystemBuffer::byte_length(array));
    interp::EvalStackOp::set_return(ret, length);
    RET_VOID_OK();
}

static RtResultVoid block_copy_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    (void)ret;

    vm::RtArray* source = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);
    int32_t source_offset = interp::EvalStackOp::get_param<int32_t>(params, 1);
    vm::RtArray* destination = interp::EvalStackOp::get_param<vm::RtArray*>(params, 2);
    int32_t destination_offset = interp::EvalStackOp::get_param<int32_t>(params, 3);
    int32_t count = interp::EvalStackOp::get_param<int32_t>(params, 4);

    RET_ERR_ON_FAIL(SystemBuffer::block_copy(source, source_offset, destination, destination_offset, count));
    RET_VOID_OK();
}

static RtResultVoid memory_copy_int64_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    (void)ret;

    const void* source = interp::EvalStackOp::get_param<const void*>(params, 0);
    void* destination = interp::EvalStackOp::get_param<void*>(params, 1);
    int64_t destination_size_in_bytes = interp::EvalStackOp::get_param<int64_t>(params, 2);
    int64_t source_bytes_to_copy = interp::EvalStackOp::get_param<int64_t>(params, 3);
    if (destination_size_in_bytes < 0 || source_bytes_to_copy < 0)
    {
        RET_ERR(RtErr::ArgumentOutOfRange);
    }

    RET_ERR_ON_FAIL(SystemBuffer::memory_copy(source, destination, static_cast<uint64_t>(destination_size_in_bytes),
                                             static_cast<uint64_t>(source_bytes_to_copy)));
    RET_VOID_OK();
}

static RtResultVoid memory_copy_uint64_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    (void)ret;

    const void* source = interp::EvalStackOp::get_param<const void*>(params, 0);
    void* destination = interp::EvalStackOp::get_param<void*>(params, 1);
    uint64_t destination_size_in_bytes = interp::EvalStackOp::get_param<uint64_t>(params, 2);
    uint64_t source_bytes_to_copy = interp::EvalStackOp::get_param<uint64_t>(params, 3);

    RET_ERR_ON_FAIL(SystemBuffer::memory_copy(source, destination, destination_size_in_bytes, source_bytes_to_copy));
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_buffer[] = {
    {"System.Buffer::ByteLength(System.Array)", (vm::IntrinsicFunction)&SystemBuffer::byte_length, byte_length_invoker},
    {"System.Buffer::BlockCopy(System.Array,System.Int32,System.Array,System.Int32,System.Int32)",
     (vm::IntrinsicFunction)&SystemBuffer::block_copy, block_copy_invoker},
    {"System.Buffer::MemoryCopy(System.Void*,System.Void*,System.Int64,System.Int64)",
     (vm::IntrinsicFunction)&SystemBuffer::memory_copy, memory_copy_int64_invoker},
    {"System.Buffer::MemoryCopy(System.Void*,System.Void*,System.UInt64,System.UInt64)",
     (vm::IntrinsicFunction)&SystemBuffer::memory_copy, memory_copy_uint64_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemBuffer::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_buffer,
                                           sizeof(s_intrinsic_entries_system_buffer) / sizeof(s_intrinsic_entries_system_buffer[0]));
}

} // namespace intrinsics
} // namespace leanclr

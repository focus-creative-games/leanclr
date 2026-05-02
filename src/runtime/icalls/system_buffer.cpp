#include "system_buffer.h"
#include "vm/rt_array.h"
#include "vm/class.h"

using namespace leanclr::core;
using namespace leanclr::vm;
using namespace leanclr::metadata;
using namespace leanclr::interp;

namespace leanclr
{
namespace icalls
{

// Implementation functions

RtResult<int32_t> SystemBuffer::byte_length(RtArray* arr) noexcept
{
    const metadata::RtClass* klass = arr->klass;
    int32_t length = Array::get_array_length(arr);
    const metadata::RtClass* ele_klass = klass->element_class;

    metadata::RtElementType ele_type = Class::get_element_type(ele_klass);
    int32_t byte_length = 0;

    switch (ele_type)
    {
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::I1:
    case metadata::RtElementType::U1:
        byte_length = length;
        break;
    case metadata::RtElementType::I2:
    case metadata::RtElementType::U2:
    case metadata::RtElementType::Char:
        byte_length = length * 2;
        break;
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    case metadata::RtElementType::R4:
        byte_length = length * 4;
        break;
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    case metadata::RtElementType::R8:
        byte_length = length * 8;
        break;
    case metadata::RtElementType::I:
    case metadata::RtElementType::U:
    {
        int32_t ptr_size = static_cast<int32_t>(sizeof(void*));
        byte_length = length * ptr_size;
        break;
    }
    default:
        RET_OK(-1);
    }

    RET_OK(byte_length);
}

RtResultVoid SystemBuffer::internal_memcpy(uint8_t* dst, const uint8_t* src, int32_t count) noexcept
{
    std::memcpy(dst, src, static_cast<size_t>(count));
    RET_VOID_OK();
}

RtResult<bool> SystemBuffer::internal_block_copy(RtArray* src, int32_t src_offset, RtArray* dst, int32_t dst_offset, int32_t count) noexcept
{
    if (src == nullptr || dst == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, src_byte_length, byte_length(src));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, dst_byte_length, byte_length(dst));

    if (static_cast<uint32_t>(src_offset) > static_cast<uint32_t>(src_byte_length - count) ||
        static_cast<uint32_t>(dst_offset) > static_cast<uint32_t>(dst_byte_length - count))
    {
        RET_OK(false);
    }

    uint8_t* src_ptr = Array::get_array_element_address<uint8_t>(src, src_offset);
    uint8_t* dst_ptr = Array::get_array_element_address<uint8_t>(dst, dst_offset);

    if (src != dst)
    {
        std::memcpy(dst_ptr, src_ptr, static_cast<size_t>(count));
    }
    else
    {
        std::memmove(dst_ptr, src_ptr, static_cast<size_t>(count));
    }

    RET_OK(true);
}

// Invoker functions

/// @icall: System.Buffer::_ByteLength(System.Array)
static RtResultVoid byte_length_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    RtArray* arr = EvalStackOp::get_param<RtArray*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemBuffer::byte_length(arr));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Buffer::InternalMemcpy(System.Byte*,System.Byte*,System.Int32)
static RtResultVoid internal_memcpy_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    uint8_t* dst = EvalStackOp::get_param<uint8_t*>(params, 0);
    const uint8_t* src = EvalStackOp::get_param<const uint8_t*>(params, 1);
    int32_t count = EvalStackOp::get_param<int32_t>(params, 2);
    (void)ret; // Suppress unused parameter warning
    return SystemBuffer::internal_memcpy(dst, src, count);
}

/// @icall: System.Buffer::InternalBlockCopy(System.Array,System.Int32,System.Array,System.Int32,System.Int32)
static RtResultVoid internal_block_copy_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    RtArray* src = EvalStackOp::get_param<RtArray*>(params, 0);
    int32_t src_offset = EvalStackOp::get_param<int32_t>(params, 1);
    RtArray* dst = EvalStackOp::get_param<RtArray*>(params, 2);
    int32_t dst_offset = EvalStackOp::get_param<int32_t>(params, 3);
    int32_t count = EvalStackOp::get_param<int32_t>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemBuffer::internal_block_copy(src, src_offset, dst, dst_offset, count));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

// Internal call entries

static InternalCallEntry s_internal_call_entries_system_buffer[] = {
    {"System.Buffer::_ByteLength(System.Array)", (InternalCallFunction)&SystemBuffer::byte_length, byte_length_invoker},
    {"System.Buffer::InternalMemcpy(System.Byte*,System.Byte*,System.Int32)", (InternalCallFunction)&SystemBuffer::internal_memcpy, internal_memcpy_invoker},
    {"System.Buffer::InternalBlockCopy(System.Array,System.Int32,System.Array,System.Int32,System.Int32)",
     (InternalCallFunction)&SystemBuffer::internal_block_copy, internal_block_copy_invoker},
};

utils::Span<InternalCallEntry> SystemBuffer::get_internal_call_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_internal_call_entries_system_buffer) / sizeof(s_internal_call_entries_system_buffer[0]);
    return utils::Span<InternalCallEntry>(s_internal_call_entries_system_buffer, entry_count);
}

} // namespace icalls
} // namespace leanclr

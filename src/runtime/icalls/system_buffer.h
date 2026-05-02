#pragma once

#include "vm/internal_calls.h"

namespace leanclr
{
namespace icalls
{

class SystemBuffer
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get byte length of array
    static RtResult<int32_t> byte_length(vm::RtArray* arr) noexcept;

    // Internal memcpy operation
    static RtResultVoid internal_memcpy(uint8_t* dst, const uint8_t* src, int32_t count) noexcept;

    // Internal block copy between arrays
    static RtResult<bool> internal_block_copy(vm::RtArray* src, int32_t src_offset, vm::RtArray* dst, int32_t dst_offset, int32_t count) noexcept;
};

} // namespace icalls
} // namespace leanclr

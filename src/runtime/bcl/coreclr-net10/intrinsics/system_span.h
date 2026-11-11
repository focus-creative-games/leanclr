#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemSpan
{
  public:
    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
    static utils::Span<vm::NewobjIntrinsicEntry> get_newobj_intrinsic_entries() noexcept;

    // Get item at index from a Span (delegates to ReadOnlySpan)
    static RtResult<const uint8_t*> get_item(const vm::RtReadOnlySpan<uint8_t>& span, int32_t index, size_t ele_size) noexcept;
    static RtResult<vm::RtReadOnlySpan<uint8_t>> newobj_pointer_length(void* pointer, int32_t length) noexcept;
    static RtResult<int32_t> index_of_null_byte(const uint8_t* pointer) noexcept;
    static RtResult<int32_t> sequence_equal(const uint8_t* first, const uint8_t* second, size_t length) noexcept;
};
} // namespace intrinsics
} // namespace leanclr

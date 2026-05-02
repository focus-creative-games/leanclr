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

    // Get item at index from a Span (delegates to ReadOnlySpan)
    static RtResult<const uint8_t*> get_item(const vm::RtReadOnlySpan<uint8_t>& span, int32_t index, size_t ele_size) noexcept;
};
} // namespace intrinsics
} // namespace leanclr

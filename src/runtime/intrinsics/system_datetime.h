#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemDateTime
{
  public:
    static RtResult<uint64_t> utc_now() noexcept;

    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
};
} // namespace intrinsics
} // namespace leanclr

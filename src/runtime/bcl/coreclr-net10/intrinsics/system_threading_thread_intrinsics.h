#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemThreadingThreadIntrinsics
{
  public:
    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;

    static RtResultVoid fast_poll_gc() noexcept;
};
} // namespace intrinsics
} // namespace leanclr

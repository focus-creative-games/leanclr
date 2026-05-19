#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{

class SystemMathF
{
  public:
    static RtResult<float> round(float value) noexcept;

    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
};

} // namespace intrinsics
} // namespace leanclr

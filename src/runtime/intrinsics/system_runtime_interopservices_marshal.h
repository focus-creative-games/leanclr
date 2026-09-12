#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemRuntimeInteropServicesMarshal
{
  public:
    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;

    static RtResult<bool> is_pinnable(vm::RtObject* obj) noexcept;
};
} // namespace intrinsics
} // namespace leanclr

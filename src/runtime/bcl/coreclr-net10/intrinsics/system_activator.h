#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemActivator
{
  public:
    static RtResultVoid create_instance(const metadata::RtMethodInfo* method, interp::RtStackObject* ret) noexcept;
    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
};
} // namespace intrinsics
} // namespace leanclr

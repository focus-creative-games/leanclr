#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemReflectionRtFieldInfo
{
  public:
    static RtResult<vm::RtObject*> get_value(vm::RtReflectionField* field, vm::RtObject* obj) noexcept;
    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
};
} // namespace intrinsics
} // namespace leanclr

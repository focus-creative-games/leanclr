#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemSR
{
  public:
    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;

    static RtResult<vm::RtString*> get_resource_string(vm::RtString* key) noexcept;
    static RtResult<vm::RtString*> format(vm::RtString* resource_format) noexcept;
    static RtResult<bool> using_resource_keys() noexcept;
};
} // namespace intrinsics
} // namespace leanclr

#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemRuntimeCompilerServicesCastHelpers
{
  public:
    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;

    static RtResult<vm::RtObject*> is_instance_of(const void* to_type_handle, vm::RtObject* obj) noexcept;
    static RtResult<vm::RtObject*> chk_cast(const void* to_type_handle, vm::RtObject* obj) noexcept;
};
} // namespace intrinsics
} // namespace leanclr

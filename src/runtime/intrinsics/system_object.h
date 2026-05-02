#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemObject
{
  public:
    static RtResultVoid ctor(vm::RtObject* obj) noexcept;

    static RtResult<vm::RtObject*> newobj_ctor() noexcept;

    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
    static utils::Span<vm::NewobjIntrinsicEntry> get_newobj_intrinsic_entries() noexcept;
};
} // namespace intrinsics
} // namespace leanclr

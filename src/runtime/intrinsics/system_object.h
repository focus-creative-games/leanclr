#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemObject
{
  public:
    static RtResultVoid ctor(vm::RtObject* obj);

    static RtResult<vm::RtObject*> newobj_ctor();

    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries();
    static utils::Span<vm::NewobjIntrinsicEntry> get_newobj_intrinsic_entries();
};
} // namespace intrinsics
} // namespace leanclr

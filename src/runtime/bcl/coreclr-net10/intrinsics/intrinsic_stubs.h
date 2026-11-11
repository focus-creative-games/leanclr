#pragma once

#include "metadata/rt_metadata.h"
#include "utils/rt_vector.h"
#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class IntrinsicStubs
{
  public:
    static void get_intrinsic_entries(utils::Vector<vm::IntrinsicEntry>& entries) noexcept;
    static void get_newobj_intrinsic_entries(utils::Vector<vm::NewobjIntrinsicEntry>& entries) noexcept;
};
} // namespace intrinsics
} // namespace leanclr

#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemValueType
{
  public:
    static RtResult<int32_t> get_hash_code(vm::RtObject* obj) noexcept;

    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
};
} // namespace intrinsics
} // namespace leanclr

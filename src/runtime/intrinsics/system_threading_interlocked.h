#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemThreadingInterlocked
{
  public:
    // Exchange operations
    static RtResult<vm::RtObject*> exchange_object(vm::RtObject** location, vm::RtObject* value) noexcept;
    static RtResult<void*> exchange(void** location, void* value) noexcept;

    // Compare and exchange
    static RtResult<void*> compare_exchange(void** location, void* value, void* comparand) noexcept;

    // Memory barrier
    static RtResultVoid memory_barrier() noexcept;

    // Get intrinsic entries
    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
};
} // namespace intrinsics
} // namespace leanclr

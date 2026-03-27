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
    static RtResult<vm::RtObject*> exchange_object(vm::RtObject** location, vm::RtObject* value);
    static RtResult<void*> exchange(void** location, void* value);

    // Compare and exchange
    static RtResult<void*> compare_exchange(void** location, void* value, void* comparand);

    // Memory barrier
    static RtResultVoid memory_barrier();

    // Get intrinsic entries
    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries();
};
} // namespace intrinsics
} // namespace leanclr

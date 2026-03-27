#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemThreadingTimer
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    static RtResult<int64_t> get_time_monotonic();
};

} // namespace icalls
} // namespace leanclr

#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemDiagnosticsStopwatch
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<int64_t> get_timestamp() noexcept;
};

} // namespace icalls
} // namespace leanclr

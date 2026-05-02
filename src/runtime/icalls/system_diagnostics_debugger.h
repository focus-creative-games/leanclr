#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemDiagnosticsDebugger
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Check if a debugger is attached
    static RtResult<bool> is_attached_internal() noexcept;

    // Check if logging is enabled
    static RtResult<bool> is_logging() noexcept;

    // Log a message to the debugger
    static RtResultVoid log_icall(int32_t level, vm::RtString** category, vm::RtString** message) noexcept;
};

} // namespace icalls
} // namespace leanclr

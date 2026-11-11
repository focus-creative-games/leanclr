#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemConsoleWindowsConsole
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<int32_t> get_console_cp() noexcept;
    static RtResult<int32_t> get_console_output_cp() noexcept;
};

} // namespace icalls
} // namespace leanclr

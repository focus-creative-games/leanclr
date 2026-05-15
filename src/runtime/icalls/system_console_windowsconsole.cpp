#include "system_console_windowsconsole.h"

#include "platform/kernel32.h"

namespace leanclr
{
namespace icalls
{

RtResult<int32_t> SystemConsoleWindowsConsole::get_console_cp() noexcept
{
    RET_OK(platform::Kernel32::get_console_cp());
}

/// @icall: System.Console/WindowsConsole::GetConsoleCP
static RtResultVoid get_console_cp_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                           interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemConsoleWindowsConsole::get_console_cp());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<int32_t> SystemConsoleWindowsConsole::get_console_output_cp() noexcept
{
    RET_OK(platform::Kernel32::get_console_output_cp());
}

/// @icall: System.Console/WindowsConsole::GetConsoleOutputCP
static RtResultVoid get_console_output_cp_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                                  interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemConsoleWindowsConsole::get_console_output_cp());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_console_windowsconsole[] = {
    {"System.Console/WindowsConsole::GetConsoleCP", (vm::InternalCallFunction)&SystemConsoleWindowsConsole::get_console_cp, get_console_cp_invoker},
    {"System.Console/WindowsConsole::GetConsoleOutputCP", (vm::InternalCallFunction)&SystemConsoleWindowsConsole::get_console_output_cp,
     get_console_output_cp_invoker},
};

utils::Span<vm::InternalCallEntry> SystemConsoleWindowsConsole::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_console_windowsconsole,
                                              sizeof(s_internal_call_entries_system_console_windowsconsole) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

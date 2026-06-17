#include "system_windowsconsoledriver.h"

#include "platform/kernel32.h"

namespace leanclr
{
namespace icalls
{

RtResult<intptr_t> SystemWindowsConsoleDriver::get_std_handle(int32_t handle) noexcept
{
    RET_OK(platform::Kernel32::get_std_handle(handle));
}

RtResult<bool> SystemWindowsConsoleDriver::get_console_screen_buffer_info(intptr_t handle, void* info) noexcept
{
    RET_OK(platform::Kernel32::get_console_screen_buffer_info(handle, info));
}

RtResult<bool> SystemWindowsConsoleDriver::peek_console_input(intptr_t handle, void* record, int32_t length, int32_t* events_read) noexcept
{
    RET_OK(platform::Kernel32::peek_console_input(handle, record, length, events_read));
}

RtResult<bool> SystemWindowsConsoleDriver::read_console_input(intptr_t handle, void* record, int32_t length, int32_t* events_read) noexcept
{
    RET_OK(platform::Kernel32::read_console_input(handle, record, length, events_read));
}

RtResult<bool> SystemWindowsConsoleDriver::get_console_mode(intptr_t handle, int32_t* mode) noexcept
{
    RET_OK(platform::Kernel32::get_console_mode(handle, mode));
}

RtResult<bool> SystemWindowsConsoleDriver::set_console_mode(intptr_t handle, int32_t mode) noexcept
{
    RET_OK(platform::Kernel32::set_console_mode(handle, mode));
}

/// @icall: System.WindowsConsoleDriver::GetStdHandle(System.Handles)
static RtResultVoid get_std_handle_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    auto handle = EvalStackOp::get_param<int32_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, result, SystemWindowsConsoleDriver::get_std_handle(handle));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.WindowsConsoleDriver::GetConsoleScreenBufferInfo(System.IntPtr,System.ConsoleScreenBufferInfo&)
static RtResultVoid get_console_screen_buffer_info_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                           interp::RtStackObject* ret) noexcept
{
    auto handle = EvalStackOp::get_param<intptr_t>(params, 0);
    auto info = EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, ok, SystemWindowsConsoleDriver::get_console_screen_buffer_info(handle, info));
    EvalStackOp::set_return(ret, ok);
    RET_VOID_OK();
}

/// @icall: System.WindowsConsoleDriver::PeekConsoleInput(System.IntPtr,System.InputRecord&,System.Int32,System.Int32&)
static RtResultVoid peek_console_input_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    auto handle = EvalStackOp::get_param<intptr_t>(params, 0);
    auto record = EvalStackOp::get_param<void*>(params, 1);
    auto length = EvalStackOp::get_param<int32_t>(params, 2);
    auto events_read = EvalStackOp::get_param<int32_t*>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, ok, SystemWindowsConsoleDriver::peek_console_input(handle, record, length, events_read));
    EvalStackOp::set_return(ret, ok);
    RET_VOID_OK();
}

/// @icall: System.WindowsConsoleDriver::ReadConsoleInput(System.IntPtr,System.InputRecord&,System.Int32,System.Int32&)
static RtResultVoid read_console_input_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    auto handle = EvalStackOp::get_param<intptr_t>(params, 0);
    auto record = EvalStackOp::get_param<void*>(params, 1);
    auto length = EvalStackOp::get_param<int32_t>(params, 2);
    auto events_read = EvalStackOp::get_param<int32_t*>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, ok, SystemWindowsConsoleDriver::read_console_input(handle, record, length, events_read));
    EvalStackOp::set_return(ret, ok);
    RET_VOID_OK();
}

/// @icall: System.WindowsConsoleDriver::GetConsoleMode(System.IntPtr,System.Int32&)
static RtResultVoid get_console_mode_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    auto handle = EvalStackOp::get_param<intptr_t>(params, 0);
    auto mode = EvalStackOp::get_param<int32_t*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, ok, SystemWindowsConsoleDriver::get_console_mode(handle, mode));
    EvalStackOp::set_return(ret, ok);
    RET_VOID_OK();
}

/// @icall: System.WindowsConsoleDriver::SetConsoleMode(System.IntPtr,System.Int32)
static RtResultVoid set_console_mode_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    auto handle = EvalStackOp::get_param<intptr_t>(params, 0);
    auto mode = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, ok, SystemWindowsConsoleDriver::set_console_mode(handle, mode));
    EvalStackOp::set_return(ret, ok);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_windowsconsoleriver[] = {
    {"System.WindowsConsoleDriver::GetStdHandle(System.Handles)", (vm::InternalCallFunction)&SystemWindowsConsoleDriver::get_std_handle,
     get_std_handle_invoker},
    {"System.WindowsConsoleDriver::GetConsoleScreenBufferInfo(System.IntPtr,System.ConsoleScreenBufferInfo&)",
     (vm::InternalCallFunction)&SystemWindowsConsoleDriver::get_console_screen_buffer_info, get_console_screen_buffer_info_invoker},
    {"System.WindowsConsoleDriver::PeekConsoleInput(System.IntPtr,System.InputRecord&,System.Int32,System.Int32&)",
     (vm::InternalCallFunction)&SystemWindowsConsoleDriver::peek_console_input, peek_console_input_invoker},
    {"System.WindowsConsoleDriver::ReadConsoleInput(System.IntPtr,System.InputRecord&,System.Int32,System.Int32&)",
     (vm::InternalCallFunction)&SystemWindowsConsoleDriver::read_console_input, read_console_input_invoker},
    {"System.WindowsConsoleDriver::GetConsoleMode(System.IntPtr,System.Int32&)", (vm::InternalCallFunction)&SystemWindowsConsoleDriver::get_console_mode,
     get_console_mode_invoker},
    {"System.WindowsConsoleDriver::SetConsoleMode(System.IntPtr,System.Int32)", (vm::InternalCallFunction)&SystemWindowsConsoleDriver::set_console_mode,
     set_console_mode_invoker},
};

utils::Span<vm::InternalCallEntry> SystemWindowsConsoleDriver::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_windowsconsoleriver,
                                              sizeof(s_internal_call_entries_system_windowsconsoleriver) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

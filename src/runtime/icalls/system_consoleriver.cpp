#include "system_consoleriver.h"

#include "platform/rt_console.h"
#include "utils/string_builder.h"
#include "vm/class.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace icalls
{

namespace
{

const char* string_to_utf8_cstr(vm::RtString* str, utils::Utf8StringBuilder& buffer) noexcept
{
    if (str == nullptr)
    {
        return nullptr;
    }
    buffer.clear();
    buffer.append_utf16_str(vm::String::get_chars_ptr(str), static_cast<size_t>(vm::String::get_length(str)));
    return buffer.get_const_chars();
}

} // namespace

bool SystemConsoleDriver::isatty(intptr_t handle) noexcept
{
    return platform::RtConsole::isatty(handle);
}

int32_t SystemConsoleDriver::internal_key_available(int32_t timeout_msec) noexcept
{
    return platform::RtConsole::internal_key_available(timeout_msec);
}

bool SystemConsoleDriver::set_echo(bool enable) noexcept
{
    return platform::RtConsole::set_echo(enable);
}

bool SystemConsoleDriver::set_break(bool enable) noexcept
{
    return platform::RtConsole::set_break(enable);
}

RtResult<bool> SystemConsoleDriver::tty_setup(vm::RtString* keypad, vm::RtString* teardown, vm::RtArray** control_chars, int32_t** size) noexcept
{
    utils::Utf8StringBuilder keypad_buf;
    utils::Utf8StringBuilder teardown_buf;
    const char* keypad_cstr = string_to_utf8_cstr(keypad, keypad_buf);
    const char* teardown_cstr = string_to_utf8_cstr(teardown, teardown_buf);

    platform::RtConsole::TtySetupResult setup_result{};
    if (!platform::RtConsole::tty_setup(keypad_cstr, teardown_cstr, &setup_result))
    {
        RET_OK(false);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, control_chars_arr,
                                            LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(vm::Class::get_corlib_types().cls_byte,
                                                                                        platform::RtConsole::kControlCharsCount,
                                                                                        "icalls::SystemConsoleDriver::tty_setup"));
    for (int32_t i = 0; i < platform::RtConsole::kControlCharsCount; ++i)
    {
        vm::Array::set_array_data_at<int8_t>(control_chars_arr, i, static_cast<int8_t>(setup_result.control_chars[i]));
    }

    *control_chars = control_chars_arr;
    *size = platform::RtConsole::get_cols_and_lines_ptr();
    RET_OK(true);
}

// ===== invokers =====

/// @icall: System.ConsoleDriver::Isatty(System.IntPtr)
static RtResultVoid isatty_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                   interp::RtStackObject* ret) noexcept
{
    auto handle = EvalStackOp::get_param<intptr_t>(params, 0);
    EvalStackOp::set_return(ret, SystemConsoleDriver::isatty(handle));
    RET_VOID_OK();
}

/// @icall: System.ConsoleDriver::InternalKeyAvailable(System.Int32)
static RtResultVoid internal_key_available_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* ret) noexcept
{
    auto timeout = EvalStackOp::get_param<int32_t>(params, 0);
    EvalStackOp::set_return(ret, SystemConsoleDriver::internal_key_available(timeout));
    RET_VOID_OK();
}

/// @icall: System.ConsoleDriver::SetEcho(System.Boolean)
static RtResultVoid set_echo_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto enable = EvalStackOp::get_param<bool>(params, 0);
    EvalStackOp::set_return(ret, SystemConsoleDriver::set_echo(enable));
    RET_VOID_OK();
}

/// @icall: System.ConsoleDriver::SetBreak(System.Boolean)
static RtResultVoid set_break_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto enable = EvalStackOp::get_param<bool>(params, 0);
    EvalStackOp::set_return(ret, SystemConsoleDriver::set_break(enable));
    RET_VOID_OK();
}

/// @icall: System.ConsoleDriver::TtySetup(System.String,System.String,System.Byte[]&,System.Int32*&)
static RtResultVoid tty_setup_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto keypad = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto teardown = EvalStackOp::get_param<vm::RtString*>(params, 1);
    vm::RtArray* control_chars = nullptr;
    int32_t* size = nullptr;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, ok, SystemConsoleDriver::tty_setup(keypad, teardown, &control_chars, &size));
    EvalStackOp::set_return(ret, ok);

    vm::RtArray** control_chars_ptr = EvalStackOp::get_param<vm::RtArray**>(params, 2);
    int32_t** size_ptr = EvalStackOp::get_param<int32_t**>(params, 3);
    *control_chars_ptr = control_chars;
    *size_ptr = size;
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_consoleriver[] = {
    {"System.ConsoleDriver::InternalKeyAvailable", (vm::InternalCallFunction)&SystemConsoleDriver::internal_key_available, internal_key_available_invoker},
    {"System.ConsoleDriver::Isatty", (vm::InternalCallFunction)&SystemConsoleDriver::isatty, isatty_invoker},
    {"System.ConsoleDriver::SetBreak", (vm::InternalCallFunction)&SystemConsoleDriver::set_break, set_break_invoker},
    {"System.ConsoleDriver::SetEcho", (vm::InternalCallFunction)&SystemConsoleDriver::set_echo, set_echo_invoker},
    {"System.ConsoleDriver::TtySetup", (vm::InternalCallFunction)&SystemConsoleDriver::tty_setup, tty_setup_invoker},
};

utils::Span<vm::InternalCallEntry> SystemConsoleDriver::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_consoleriver,
                                              sizeof(s_internal_call_entries_system_consoleriver) / sizeof(vm::InternalCallEntry));
}
} // namespace icalls
} // namespace leanclr

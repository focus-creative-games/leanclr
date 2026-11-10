#include "platform/rt_console.h"

#include <cstring>

namespace leanclr
{
namespace platform
{

namespace
{
int32_t g_cols_and_lines = -1;
} // namespace

bool RtConsole::isatty(intptr_t /*handle*/) noexcept
{
    return false;
}

int32_t RtConsole::internal_key_available(int32_t /*timeout_msec*/) noexcept
{
    return 0;
}

bool RtConsole::set_echo(bool /*enable*/) noexcept
{
    return false;
}

bool RtConsole::set_break(bool /*enable*/) noexcept
{
    return false;
}

bool RtConsole::tty_setup(const char* /*keypad_xmit*/, const char* /*teardown*/, TtySetupResult* out_result) noexcept
{
    if (out_result == nullptr)
    {
        return false;
    }

    std::memset(out_result->control_chars, 0, static_cast<size_t>(kControlCharsCount));
    g_cols_and_lines = -1;
    out_result->cols_and_lines = g_cols_and_lines;
    return false;
}

int32_t* RtConsole::get_cols_and_lines_ptr() noexcept
{
    return &g_cols_and_lines;
}

} // namespace platform
} // namespace leanclr

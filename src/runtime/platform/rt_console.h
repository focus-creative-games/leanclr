#pragma once

#include "core/rt_base.h"

namespace leanclr
{
namespace platform
{

class RtConsole
{
  public:
    static constexpr int32_t kControlCharsCount = 17;

    static bool isatty(intptr_t handle) noexcept;
    static int32_t internal_key_available(int32_t timeout_msec) noexcept;
    static bool set_echo(bool enable) noexcept;
    static bool set_break(bool enable) noexcept;

    struct TtySetupResult
    {
        int32_t cols_and_lines;
        uint8_t control_chars[kControlCharsCount];
    };

    static bool tty_setup(const char* keypad_xmit, const char* teardown, TtySetupResult* out_result) noexcept;
    static int32_t* get_cols_and_lines_ptr() noexcept;
};

} // namespace platform
} // namespace leanclr

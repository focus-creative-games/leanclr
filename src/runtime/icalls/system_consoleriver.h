#pragma once

#include <cstdint>
#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemConsoleDriver
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static bool isatty(intptr_t handle) noexcept;
    static int32_t internal_key_available(int32_t timeout_msec) noexcept;
    static bool set_echo(bool enable) noexcept;
    static bool set_break(bool enable) noexcept;
    static RtResult<bool> tty_setup(vm::RtString* keypad, vm::RtString* teardown, vm::RtArray** control_chars, int32_t** size) noexcept;
};
} // namespace icalls
} // namespace leanclr

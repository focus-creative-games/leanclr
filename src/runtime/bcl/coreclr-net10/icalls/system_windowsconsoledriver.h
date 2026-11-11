#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemWindowsConsoleDriver
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<intptr_t> get_std_handle(int32_t handle) noexcept;
    static RtResult<bool> get_console_screen_buffer_info(intptr_t handle, void* info) noexcept;
    static RtResult<bool> peek_console_input(intptr_t handle, void* record, int32_t length, int32_t* events_read) noexcept;
    static RtResult<bool> read_console_input(intptr_t handle, void* record, int32_t length, int32_t* events_read) noexcept;
    static RtResult<bool> get_console_mode(intptr_t handle, int32_t* mode) noexcept;
    static RtResult<bool> set_console_mode(intptr_t handle, int32_t mode) noexcept;
};

} // namespace icalls
} // namespace leanclr

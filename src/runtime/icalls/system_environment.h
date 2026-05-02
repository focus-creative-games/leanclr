#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemEnvironment
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // System.Environment icalls
    static RtResult<int32_t> get_exit_code() noexcept;
    static RtResultVoid set_exit_code(int32_t code) noexcept;
    static RtResult<bool> get_has_shutdown_started() noexcept;
    static RtResult<vm::RtString*> get_machine_name() noexcept;
    static RtResult<vm::RtString*> get_new_line() noexcept;
    static RtResult<int32_t> get_platform() noexcept;
    static RtResult<vm::RtString*> get_os_version_string() noexcept;
    static RtResult<int32_t> get_tick_count() noexcept;
    static RtResult<vm::RtString*> get_user_name() noexcept;
    static RtResultVoid exit(int32_t code) noexcept;
    static RtResult<vm::RtArray*> get_command_line_args() noexcept;

    static RtResult<vm::RtString*> internal_get_environment_variable_native(const char* variable_name) noexcept;
    static RtResultVoid internal_set_environment_variable_native(const Utf16Char* variable_name, int32_t variable_name_length, const Utf16Char* value,
                                                                 int32_t value_length) noexcept;
    static RtResult<vm::RtArray*> get_environment_variable_names() noexcept;

    static RtResult<vm::RtString*> get_windows_folder_path(int32_t folder) noexcept;
    static RtResult<vm::RtArray*> get_logical_drives_internal() noexcept;
    static RtResult<vm::RtString*> get_machine_config_path() noexcept;
    static RtResult<vm::RtString*> internal_get_home() noexcept;
    static RtResult<vm::RtString*> get_bundled_machine_config() noexcept;
    static RtResultVoid fail_fast() noexcept;
    static RtResult<bool> get_is_64bit_operating_system() noexcept;
    static RtResult<int32_t> get_processor_count() noexcept;
    static RtResult<int32_t> get_page_size() noexcept;
};

} // namespace icalls
} // namespace leanclr

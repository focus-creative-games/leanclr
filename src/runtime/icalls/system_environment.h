#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemEnvironment
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // System.Environment icalls
    static RtResult<int32_t> get_exit_code();
    static RtResultVoid set_exit_code(int32_t code);
    static RtResult<bool> get_has_shutdown_started();
    static RtResult<vm::RtString*> get_machine_name();
    static RtResult<vm::RtString*> get_new_line();
    static RtResult<int32_t> get_platform();
    static RtResult<vm::RtString*> get_os_version_string();
    static RtResult<int32_t> get_tick_count();
    static RtResult<vm::RtString*> get_user_name();
    static RtResultVoid exit(int32_t code);
    static RtResult<vm::RtArray*> get_command_line_args();

    static RtResult<vm::RtString*> internal_get_environment_variable_native(const char* variable_name);
    static RtResultVoid internal_set_environment_variable_native(const Utf16Char* variable_name, int32_t variable_name_length, const Utf16Char* value,
                                                                 int32_t value_length);
    static RtResult<vm::RtArray*> get_environment_variable_names();

    static RtResult<vm::RtString*> get_windows_folder_path(int32_t folder);
    static RtResult<vm::RtArray*> get_logical_drives_internal();
    static RtResult<vm::RtString*> get_machine_config_path();
    static RtResult<vm::RtString*> internal_get_home();
    static RtResult<vm::RtString*> get_bundled_machine_config();
    static RtResultVoid fail_fast();
    static RtResult<bool> get_is_64bit_operating_system();
    static RtResult<int32_t> get_processor_count();
    static RtResult<int32_t> get_page_size();
};

} // namespace icalls
} // namespace leanclr

#pragma once

#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{
enum class Platform
{
    Win32S,
    Win32Windows,
    Win32NT,
    WinCE,
    Unix,
    Xbox,
    MacOSX,
};

class Environment
{
  public:
    static void exit(int32_t code);
    static int32_t get_exit_code();
    static void set_exit_code(int32_t code);
    static void shutdown();
    static bool has_shutdown_started();

    static RtResult<RtString*> get_machine_name();
    static RtResult<RtString*> get_new_line();
    static RtResult<vm::RtString*> get_os_version_string();
    static RtResult<vm::RtString*> get_user_name();

    static Platform get_platform();

    static RtArray* get_command_line_args();
    static RtResultVoid init_cmdline_args(const char** argv, int32_t argc);
    static RtResult<RtString*> get_environment_variable(const char* variable_name);
    static RtResultVoid set_environment_variable(const Utf16Char* variable_name, int32_t variable_name_length, const Utf16Char* value, int32_t value_length);
    static RtResult<RtArray*> get_environment_variable_names();

    static RtResult<vm::RtString*> get_windows_folder_path(int32_t folder);
    static RtResult<vm::RtArray*> get_logical_drives();
    static RtResult<vm::RtString*> get_machine_config_path();
    static RtResult<vm::RtString*> get_home();
    static RtResult<vm::RtString*> get_bundled_machine_config();

    static void fail_fast();

    static int32_t get_processor_count();
    static int32_t get_page_size();
};
} // namespace vm
} // namespace leanclr

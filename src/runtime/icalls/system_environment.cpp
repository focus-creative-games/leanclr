#include "system_environment.h"
#include "icall_base.h"

#include "vm/runtime.h"
#include "vm/rt_string.h"
#include "vm/rt_array.h"
#include "vm/class.h"
#include "vm/environment.h"

#include "platform/rt_time.h"

namespace leanclr
{
namespace icalls
{

RtResult<int32_t> SystemEnvironment::get_exit_code() noexcept
{
    RET_OK(vm::Environment::get_exit_code());
}

/// @icall: System.Environment::get_ExitCode
static RtResultVoid get_exit_code_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                          interp::RtStackObject* ret) noexcept
{
    int32_t code = vm::Environment::get_exit_code();
    EvalStackOp::set_return(ret, code);
    RET_VOID_OK();
}

RtResultVoid SystemEnvironment::set_exit_code(int32_t code) noexcept
{
    vm::Environment::set_exit_code(code);
    RET_VOID_OK();
}

/// @icall: System.Environment::set_ExitCode
static RtResultVoid set_exit_code_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                          interp::RtStackObject* /*ret*/) noexcept
{
    int32_t code = EvalStackOp::get_param<int32_t>(params, 0);
    return SystemEnvironment::set_exit_code(code);
}

RtResult<bool> SystemEnvironment::get_has_shutdown_started() noexcept
{
    RET_OK(vm::Environment::has_shutdown_started());
}

/// @icall: System.Environment::get_HasShutdownStarted
static RtResultVoid get_has_shutdown_started_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                                     interp::RtStackObject* ret) noexcept
{
    bool result = vm::Environment::has_shutdown_started();
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemEnvironment::get_machine_name() noexcept
{
    return vm::Environment::get_machine_name();
}

/// @icall: System.Environment::get_MachineName
static RtResultVoid get_machine_name_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                             interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemEnvironment::get_machine_name());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemEnvironment::get_new_line() noexcept
{
    return vm::Environment::get_new_line();
}

/// @icall: System.Environment::get_NewLine
static RtResultVoid get_new_line_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                         interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemEnvironment::get_new_line());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<int32_t> SystemEnvironment::get_platform() noexcept
{
    RET_OK(static_cast<int32_t>(vm::Environment::get_platform()));
}

/// @icall: System.Environment::get_Platform
static RtResultVoid get_platform_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                         interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, platform, SystemEnvironment::get_platform());
    EvalStackOp::set_return(ret, platform);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemEnvironment::get_os_version_string() noexcept
{
    return vm::Environment::get_os_version_string();
}

/// @icall: System.Environment::GetOSVersionString
static RtResultVoid get_os_version_string_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                                  interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, s, SystemEnvironment::get_os_version_string());
    EvalStackOp::set_return(ret, s);
    RET_VOID_OK();
}

RtResult<int32_t> SystemEnvironment::get_tick_count() noexcept
{
    RET_OK(os::Time::get_tick_count());
}

/// @icall: System.Environment::get_TickCount
static RtResultVoid get_tick_count_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                           interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(int32_t, ticks, SystemEnvironment::get_tick_count());
    EvalStackOp::set_return(ret, ticks);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemEnvironment::get_user_name() noexcept
{
    return vm::Environment::get_user_name();
}

/// @icall: System.Environment::get_UserName
static RtResultVoid get_user_name_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                          interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, s, SystemEnvironment::get_user_name());
    EvalStackOp::set_return(ret, s);
    RET_VOID_OK();
}

RtResultVoid SystemEnvironment::exit(int32_t code) noexcept
{
    vm::Environment::exit(code);
    RET_VOID_OK();
}

/// @icall: System.Environment::Exit
static RtResultVoid exit_invoker_system_environment(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                    interp::RtStackObject* /*ret*/) noexcept
{
    int32_t code = EvalStackOp::get_param<int32_t>(params, 0);
    RET_ERR_ON_FAIL(SystemEnvironment::exit(code));
    RET_VOID_OK();
}

RtResult<vm::RtArray*> SystemEnvironment::get_command_line_args() noexcept
{
    RET_OK(vm::Environment::get_command_line_args());
}

/// @icall: System.Environment::GetCommandLineArgs
static RtResultVoid get_command_line_args_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                                  interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, SystemEnvironment::get_command_line_args());
    EvalStackOp::set_return(ret, arr);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemEnvironment::internal_get_environment_variable_native(const char* variable_name) noexcept
{
    if (variable_name == nullptr)
        RET_OK(vm::String::get_empty_string());
    return vm::Environment::get_environment_variable(variable_name);
}

/// @icall: System.Environment::internalGetEnvironmentVariable_native
static RtResultVoid internal_get_environment_variable_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    const char* name = EvalStackOp::get_param<const char*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, s, SystemEnvironment::internal_get_environment_variable_native(name));
    EvalStackOp::set_return(ret, s);
    RET_VOID_OK();
}

RtResultVoid SystemEnvironment::internal_set_environment_variable_native(const Utf16Char* variable_name, int32_t variable_name_length, const Utf16Char* value,
                                                                         int32_t value_length) noexcept
{
    return vm::Environment::set_environment_variable(variable_name, variable_name_length, value, value_length);
}

/// @icall: System.Environment::InternalSetEnvironmentVariable(System.Char*,System.Int32,System.Char*,System.Int32)
static RtResultVoid internal_set_environment_variable_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                     const interp::RtStackObject* params, interp::RtStackObject* /*ret*/) noexcept
{
    auto var_name = EvalStackOp::get_param<const Utf16Char*>(params, 0);
    auto var_name_len = EvalStackOp::get_param<int32_t>(params, 1);
    auto val = EvalStackOp::get_param<const Utf16Char*>(params, 2);
    auto val_len = EvalStackOp::get_param<int32_t>(params, 3);
    RET_ERR_ON_FAIL(SystemEnvironment::internal_set_environment_variable_native(var_name, var_name_len, val, val_len));
    RET_VOID_OK();
}

RtResult<vm::RtArray*> SystemEnvironment::get_environment_variable_names() noexcept
{
    return vm::Environment::get_environment_variable_names();
}

/// @icall: System.Environment::GetEnvironmentVariableNames
static RtResultVoid get_environment_variable_names_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                           const interp::RtStackObject* /*params*/, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, SystemEnvironment::get_environment_variable_names());
    EvalStackOp::set_return(ret, arr);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemEnvironment::get_windows_folder_path(int32_t folder) noexcept
{
    return vm::Environment::get_windows_folder_path(folder);
}

/// @icall: System.Environment::GetWindowsFolderPath
static RtResultVoid get_windows_folder_path_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                    interp::RtStackObject* ret) noexcept
{
    int32_t folder = EvalStackOp::get_param<int32_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, s, SystemEnvironment::get_windows_folder_path(folder));
    EvalStackOp::set_return(ret, s);
    RET_VOID_OK();
}

RtResult<vm::RtArray*> SystemEnvironment::get_logical_drives_internal() noexcept
{
    return vm::Environment::get_logical_drives();
}

/// @icall: System.Environment::GetLogicalDrivesInternal
static RtResultVoid get_logical_drives_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                        const interp::RtStackObject* /*params*/, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, SystemEnvironment::get_logical_drives_internal());
    EvalStackOp::set_return(ret, arr);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemEnvironment::get_machine_config_path() noexcept
{
    return vm::Environment::get_machine_config_path();
}

/// @icall: System.Environment::GetMachineConfigPath
static RtResultVoid get_machine_config_path_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                                    interp::RtStackObject* ret) noexcept
{
    vm::RtString* s = vm::String::get_empty_string();
    EvalStackOp::set_return(ret, s);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemEnvironment::internal_get_home() noexcept
{
    return vm::Environment::get_home();
}

/// @icall: System.Environment::internalGetHome
static RtResultVoid internal_get_home_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                              interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, s, SystemEnvironment::internal_get_home());
    EvalStackOp::set_return(ret, s);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemEnvironment::get_bundled_machine_config() noexcept
{
    return vm::Environment::get_bundled_machine_config();
}

/// @icall: System.Environment::get_bundled_machine_config
static RtResultVoid get_bundled_machine_config_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                                       interp::RtStackObject* ret) noexcept
{
    vm::RtString* s = vm::String::get_empty_string();
    EvalStackOp::set_return(ret, s);
    RET_VOID_OK();
}

RtResultVoid SystemEnvironment::fail_fast() noexcept
{
    vm::Environment::fail_fast();
    RET_VOID_OK();
}

/// @icall: System.Environment::FailFast
static RtResultVoid fail_fast_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                      interp::RtStackObject* /*ret*/) noexcept
{
    RET_ERR_ON_FAIL(SystemEnvironment::fail_fast());
    RET_VOID_OK();
}

RtResult<bool> SystemEnvironment::get_is_64bit_operating_system() noexcept
{
    RET_OK(sizeof(void*) == 8);
}

/// @icall: System.Environment::get_Is64BitOperatingSystem
static RtResultVoid get_is_64bit_operating_system_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                          const interp::RtStackObject* /*params*/, interp::RtStackObject* ret) noexcept
{
    EvalStackOp::set_return(ret, static_cast<int32_t>(sizeof(void*) == 8));
    RET_VOID_OK();
}

RtResult<int32_t> SystemEnvironment::get_processor_count() noexcept
{
    RET_OK(vm::Environment::get_processor_count());
}

/// @icall: System.Environment::get_ProcessorCount
static RtResultVoid get_processor_count_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                                interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(int32_t, count, SystemEnvironment::get_processor_count());
    EvalStackOp::set_return(ret, count);
    RET_VOID_OK();
}

RtResult<int32_t> SystemEnvironment::get_page_size() noexcept
{
    RET_OK(vm::Environment::get_page_size());
}

/// @icall: System.Environment::GetPageSize
static RtResultVoid get_page_size_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                          interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(int32_t, size, SystemEnvironment::get_page_size());
    EvalStackOp::set_return(ret, size);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_environment[] = {
    {"System.Environment::get_ExitCode", (vm::InternalCallFunction)SystemEnvironment::get_exit_code, get_exit_code_invoker},
    {"System.Environment::set_ExitCode", (vm::InternalCallFunction)SystemEnvironment::set_exit_code, set_exit_code_invoker},
    {"System.Environment::get_HasShutdownStarted", (vm::InternalCallFunction)SystemEnvironment::get_has_shutdown_started, get_has_shutdown_started_invoker},
    {"System.Environment::get_MachineName", (vm::InternalCallFunction)SystemEnvironment::get_machine_name, get_machine_name_invoker},
    {"System.Environment::GetNewLine", (vm::InternalCallFunction)SystemEnvironment::get_new_line, get_new_line_invoker},
    {"System.Environment::get_Platform", (vm::InternalCallFunction)SystemEnvironment::get_platform, get_platform_invoker},
    {"System.Environment::GetOSVersionString", (vm::InternalCallFunction)SystemEnvironment::get_os_version_string, get_os_version_string_invoker},
    {"System.Environment::get_TickCount", (vm::InternalCallFunction)SystemEnvironment::get_tick_count, get_tick_count_invoker},
    {"System.Environment::get_UserName", (vm::InternalCallFunction)SystemEnvironment::get_user_name, get_user_name_invoker},
    {"System.Environment::Exit", (vm::InternalCallFunction)SystemEnvironment::exit, exit_invoker_system_environment},
    {"System.Environment::GetCommandLineArgs", (vm::InternalCallFunction)SystemEnvironment::get_command_line_args, get_command_line_args_invoker},
    {"System.Environment::internalGetEnvironmentVariable_native", (vm::InternalCallFunction)SystemEnvironment::internal_get_environment_variable_native,
     internal_get_environment_variable_native_invoker},
    {"System.Environment::InternalSetEnvironmentVariable(System.Char*,System.Int32,System.Char*,System.Int32)",
     (vm::InternalCallFunction)SystemEnvironment::internal_set_environment_variable_native, internal_set_environment_variable_native_invoker},
    {"System.Environment::GetEnvironmentVariableNames", (vm::InternalCallFunction)SystemEnvironment::get_environment_variable_names,
     get_environment_variable_names_invoker},
    {"System.Environment::GetWindowsFolderPath", (vm::InternalCallFunction)SystemEnvironment::get_windows_folder_path, get_windows_folder_path_invoker},
    {"System.Environment::GetLogicalDrivesInternal", (vm::InternalCallFunction)SystemEnvironment::get_logical_drives_internal,
     get_logical_drives_internal_invoker},
    {"System.Environment::GetMachineConfigPath", (vm::InternalCallFunction)SystemEnvironment::get_machine_config_path, get_machine_config_path_invoker},
    {"System.Environment::internalGetHome", (vm::InternalCallFunction)SystemEnvironment::internal_get_home, internal_get_home_invoker},
    {"System.Environment::get_bundled_machine_config", (vm::InternalCallFunction)SystemEnvironment::get_bundled_machine_config,
     get_bundled_machine_config_invoker},
    {"System.Environment::FailFast", (vm::InternalCallFunction)SystemEnvironment::fail_fast, fail_fast_invoker},
    {"System.Environment::get_Is64BitOperatingSystem", (vm::InternalCallFunction)SystemEnvironment::get_is_64bit_operating_system,
     get_is_64bit_operating_system_invoker},
    {"System.Environment::get_ProcessorCount", (vm::InternalCallFunction)SystemEnvironment::get_processor_count, get_processor_count_invoker},
    {"System.Environment::GetPageSize", (vm::InternalCallFunction)SystemEnvironment::get_page_size, get_page_size_invoker},
};

utils::Span<vm::InternalCallEntry> SystemEnvironment::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_environment,
                                              sizeof(s_internal_call_entries_system_environment) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

#include "interop.h"

#include "vm/class.h"
#include "vm/object.h"
#include "vm/assembly.h"
#include "metadata/module_def.h"
#include "platform/bcrypt.h"
#include "platform/kernel32.h"
namespace leanclr
{
namespace icalls
{

RtResult<int32_t> Interop::double_to_string(double value, const char* format, char* buffer, int32_t buffer_size)
{
    return ::snprintf(buffer, static_cast<size_t>(buffer_size), format, value);
}

/// @icall: Interop/Sys::DoubleToString(System.Double,System.Byte*,System.Byte*,System.Int32)
RtResultVoid double_to_string_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    auto format = EvalStackOp::get_param<const char*>(params, 1);
    auto buffer = EvalStackOp::get_param<char*>(params, 2);
    auto buffer_size = EvalStackOp::get_param<int32_t>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::double_to_string(value, format, buffer, buffer_size));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<int32_t> Interop::sys_lchflags_can_set_hidden_flag()
{
    RET_OK(0);
}

/// @icall: Interop/Sys::LChflagsCanSetHiddenFlag
RtResultVoid sys_lchflags_can_set_hidden_flag_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                      interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_lchflags_can_set_hidden_flag());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<uint32_t> Interop::bcrypt_gen_random(intptr_t algo_handle, uint8_t* buffer, int32_t length, int32_t flags)
{
    platform::Bcrypt::gen_random(algo_handle, buffer, length, flags);
    RET_OK(0);
}

/// @icall: Interop/BCrypt::BCryptGenRandom(System.IntPtr,System.Byte*,System.Int32,System.Int32)
RtResultVoid bcrypt_gen_random_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    intptr_t algo_handle = interp::EvalStackOp::get_param<intptr_t>(params, 0);
    uint8_t* buffer = interp::EvalStackOp::get_param<uint8_t*>(params, 1);
    int32_t length = interp::EvalStackOp::get_param<int32_t>(params, 2);
    int32_t flags = interp::EvalStackOp::get_param<int32_t>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, result, Interop::bcrypt_gen_random(algo_handle, buffer, length, flags));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_set_thread_error_mode(uint32_t mode, uint32_t& old_mode)
{
    return platform::Kernel32::set_thread_error_mode(mode, old_mode);
}
/// @icall: Interop/Kernel32::SetThreadErrorMode(System.UInt32,System.UInt32&)
RtResultVoid kernel32_set_thread_error_mode_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                    interp::RtStackObject* ret) noexcept
{
    uint32_t mode = interp::EvalStackOp::get_param<uint32_t>(params, 0);
    uint32_t& old_mode = *interp::EvalStackOp::get_param<uint32_t*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_set_thread_error_mode(mode, old_mode));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_get_file_attributes_ex_private(vm::RtString* name, uint32_t file_info_level, void* file_info)
{
    return platform::Kernel32::get_file_attributes_ex_private(name, file_info_level, file_info);
}

/// @icall: Interop/Kernel32::GetFileAttributesExPrivate(System.String,Interop/Kernel32/GET_FILEEX_INFO_LEVELS, Interop/Kernel32/WIN32_FILE_ATTRIBUTE_DATA&)
RtResultVoid kernel32_get_file_attributes_ex_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                      interp::RtStackObject* ret) noexcept
{
    vm::RtString* name = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    uint32_t file_info_level = interp::EvalStackOp::get_param<uint32_t>(params, 1);
    void* file_info = interp::EvalStackOp::get_param<void*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_get_file_attributes_ex_private(name, file_info_level, file_info));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

struct SafeFindHandle : public vm::RtObject
{
    intptr_t handle;
};

RtResult<vm::RtObject*> Interop::kernel32_find_first_file_ex_private(vm::RtString* lp_file_name, uint32_t f_info_level_id, void* lp_find_file_data,
                                                                 uint32_t f_search_op, intptr_t lp_search_filter, int32_t dw_additional_flags)
{
    static metadata::RtClass* safe_find_handle_class = nullptr;
    if (!safe_find_handle_class)
    {
        metadata::RtModuleDef* mod = vm::Assembly::get_corlib()->mod;
        UNWRAP_OR_RET_ERR_ON_FAIL(safe_find_handle_class, mod->get_class_by_name("Microsoft.Win32.SafeHandles.SafeFindHandle", false, true));
    }
    intptr_t handle = platform::Kernel32::find_first_file_ex_private(lp_file_name, f_info_level_id, lp_find_file_data, f_search_op, lp_search_filter,
                                                          dw_additional_flags);
    const metadata::RtClass* safe_handle_klass = safe_find_handle_class->parent->parent;
    assert(safe_handle_klass && std::strcmp(safe_handle_klass->name, "SafeHandle") == 0);
    assert(std::strcmp(safe_handle_klass->fields[0].name, "handle") == 0 && safe_handle_klass->fields[0].offset == 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, safe_handle, vm::Object::new_object(safe_handle_klass));
    reinterpret_cast<SafeFindHandle*>(safe_handle)->handle = handle;
    RET_OK(safe_handle);
}

/// @icall: Interop/Kernel32::FindFirstFileExPrivate(System.String,Interop/Kernel32/FINDEX_INFO_LEVELS,Interop/Kernel32/WIN32_FIND_DATA&,Interop/Kernel32/FINDEX_SEARCH_OPS,System.IntPtr,System.Int32)
RtResultVoid kernel32_find_first_file_ex_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                         interp::RtStackObject* ret) noexcept
{
    vm::RtString* lp_file_name = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    uint32_t f_info_level_id = interp::EvalStackOp::get_param<uint32_t>(params, 1);
    void* lp_find_file_data = interp::EvalStackOp::get_param<void*>(params, 2);
    uint32_t f_search_op = interp::EvalStackOp::get_param<uint32_t>(params, 3);
    intptr_t lp_search_filter = interp::EvalStackOp::get_param<intptr_t>(params, 4);
    int32_t dw_additional_flags = interp::EvalStackOp::get_param<int32_t>(params, 5);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result,
                                            Interop::kernel32_find_first_file_ex_private(lp_file_name, f_info_level_id, lp_find_file_data, f_search_op,
                                                                                         lp_search_filter, dw_additional_flags));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<int32_t> Interop::windows_console_get_console_cp()
{
    RET_OK(platform::Kernel32::get_console_cp());
}

/// @icall: System.Console/WindowsConsole::GetConsoleCP
RtResultVoid windows_console_get_console_cp_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                                    interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::windows_console_get_console_cp());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<int32_t> Interop::windows_console_get_console_output_cp()
{
    RET_OK(platform::Kernel32::get_console_output_cp());
}

/// @icall: System.Console/WindowsConsole::GetConsoleOutputCP
RtResultVoid windows_console_get_console_output_cp_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                                           interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::windows_console_get_console_output_cp());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_interop_internal_call_entries[] = {
    {"Interop/Sys::DoubleToString(System.Double,System.Byte*,System.Byte*,System.Int32)", (vm::InternalCallFunction)&Interop::double_to_string,
     double_to_string_invoker},
    {"Interop/Sys::LChflagsCanSetHiddenFlag", (vm::InternalCallFunction)&Interop::sys_lchflags_can_set_hidden_flag, sys_lchflags_can_set_hidden_flag_invoker},
    {"Interop/BCrypt::BCryptGenRandom(System.IntPtr,System.Byte*,System.Int32,System.Int32)", (vm::InternalCallFunction)&Interop::bcrypt_gen_random,
     bcrypt_gen_random_invoker},
    {"Interop/Kernel32::SetThreadErrorMode(System.UInt32,System.UInt32&)", (vm::InternalCallFunction)&Interop::kernel32_set_thread_error_mode,
     kernel32_set_thread_error_mode_invoker},
    {"Interop/Kernel32::GetFileAttributesExPrivate(System.String,Interop/Kernel32/GET_FILEEX_INFO_LEVELS,Interop/Kernel32/WIN32_FILE_ATTRIBUTE_DATA&)",
     (vm::InternalCallFunction)&Interop::kernel32_get_file_attributes_ex_private,
     kernel32_get_file_attributes_ex_private_invoker},
    {"Interop/Kernel32::FindFirstFileExPrivate(System.String,Interop/Kernel32/FINDEX_INFO_LEVELS,Interop/Kernel32/WIN32_FIND_DATA&,Interop/Kernel32/"
     "FINDEX_SEARCH_OPS,System.IntPtr,System.Int32)",
     (vm::InternalCallFunction)&Interop::kernel32_find_first_file_ex_private, kernel32_find_first_file_ex_private_invoker},
    {"System.Console/WindowsConsole::GetConsoleCP", (vm::InternalCallFunction)&Interop::windows_console_get_console_cp,
     windows_console_get_console_cp_invoker},
    {"System.Console/WindowsConsole::GetConsoleOutputCP", (vm::InternalCallFunction)&Interop::windows_console_get_console_output_cp,
     windows_console_get_console_output_cp_invoker},
};

utils::Span<vm::InternalCallEntry> Interop::get_internal_call_entries()
{
    return utils::Span<vm::InternalCallEntry>(s_interop_internal_call_entries, sizeof(s_interop_internal_call_entries) / sizeof(vm::InternalCallEntry));
}
} // namespace icalls
} // namespace leanclr

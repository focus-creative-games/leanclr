#include "interop.h"

#include "vm/class.h"
#include "vm/object.h"
#include "vm/assembly.h"
#include "metadata/module_def.h"
#include "platform/bcrypt.h"
#include "platform/kernel32.h"
#include "platform/rt_sys.h"
namespace leanclr
{
namespace icalls
{

#if LEANCLR_PLATFORM_POSIX
RtResult<int32_t> Interop::double_to_string(double value, const char* format, char* buffer, int32_t buffer_size) noexcept
{
    RET_OK(platform::RtSys::double_to_string(value, format, buffer, buffer_size));
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

RtResult<int32_t> Interop::sys_lchflags_can_set_hidden_flag() noexcept
{
    RET_OK(platform::RtSys::lchflags_can_set_hidden_flag());
}

/// @icall: Interop/Sys::LChflagsCanSetHiddenFlag
RtResultVoid sys_lchflags_can_set_hidden_flag_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                      interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_lchflags_can_set_hidden_flag());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<int32_t> Interop::globalization_get_time_zone_display_name(vm::RtString* locale_name, vm::RtString* time_zone_id, int32_t type, vm::RtObject* result,
                                                                    int32_t result_length) noexcept
{
    RET_OK(platform::RtSys::globalization_get_time_zone_display_name(locale_name, time_zone_id, type, result, result_length));
}

RtResult<int32_t> Interop::sys_ch_mod(vm::RtString* path, int32_t mode) noexcept
{
    RET_OK(platform::RtSys::ch_mod(path, mode));
}

RtResult<int32_t> Interop::sys_close_dir(intptr_t dir) noexcept
{
    RET_OK(platform::RtSys::close_dir(dir));
}

RtResult<int32_t> Interop::sys_convert_error_pal_to_platform(int32_t error) noexcept
{
    RET_OK(platform::RtSys::convert_error_pal_to_platform(error));
}

RtResult<int32_t> Interop::sys_convert_error_platform_to_pal(int32_t error) noexcept
{
    RET_OK(platform::RtSys::convert_error_platform_to_pal(error));
}

RtResult<int32_t> Interop::sys_copy_file(vm::RtObject* source, vm::RtObject* destination) noexcept
{
    RET_OK(platform::RtSys::copy_file(source, destination));
}

RtResult<int32_t> Interop::sys_f_stat(vm::RtObject* fd, void* output) noexcept
{
    RET_OK(platform::RtSys::f_stat(fd, output));
}

RtResult<uint32_t> Interop::sys_get_e_gid() noexcept
{
    RET_OK(platform::RtSys::get_e_gid());
}

RtResult<uint32_t> Interop::sys_get_e_uid() noexcept
{
    RET_OK(platform::RtSys::get_e_uid());
}

RtResultVoid Interop::sys_get_non_cryptographically_secure_random_bytes(uint8_t* buffer, int32_t length) noexcept
{
    platform::RtSys::get_non_cryptographically_secure_random_bytes(buffer, length);
    RET_VOID_OK();
}

RtResult<int32_t> Interop::sys_get_read_dir_r_buffer_size() noexcept
{
    RET_OK(platform::RtSys::get_read_dir_r_buffer_size());
}

RtResult<int32_t> Interop::sys_lchflags(vm::RtString* path, uint32_t flags) noexcept
{
    RET_OK(platform::RtSys::lchflags(path, flags));
}

RtResult<int32_t> Interop::sys_link(vm::RtString* source, vm::RtString* target) noexcept
{
    RET_OK(platform::RtSys::link(source, target));
}

RtResult<int32_t> Interop::sys_lstat_byte(uint8_t* path, void* output) noexcept
{
    RET_OK(platform::RtSys::lstat_byte(path, output));
}

RtResult<int32_t> Interop::sys_lstat_string(vm::RtString* path, void* output) noexcept
{
    RET_OK(platform::RtSys::lstat_string(path, output));
}

RtResult<int32_t> Interop::sys_mkdir(vm::RtString* path, int32_t mode) noexcept
{
    RET_OK(platform::RtSys::mk_dir(path, mode));
}

RtResult<intptr_t> Interop::sys_open_dir(vm::RtString* path) noexcept
{
    RET_OK(platform::RtSys::open_dir(path));
}

RtResult<int32_t> Interop::sys_read_dir_r(intptr_t dir, uint8_t* buffer, int32_t buffer_size, void* output_entry) noexcept
{
    RET_OK(platform::RtSys::read_dir_r(dir, buffer, buffer_size, output_entry));
}

RtResult<int32_t> Interop::sys_read_link(vm::RtString* path, vm::RtArray* buffer, int32_t buffer_size) noexcept
{
    RET_OK(platform::RtSys::read_link(path, buffer, buffer_size));
}

RtResult<int32_t> Interop::sys_rename(vm::RtString* old_path, vm::RtString* new_path) noexcept
{
    RET_OK(platform::RtSys::rename(old_path, new_path));
}

RtResult<int32_t> Interop::sys_rmdir(vm::RtString* path) noexcept
{
    RET_OK(platform::RtSys::rm_dir(path));
}

RtResult<int32_t> Interop::sys_stat_byte(uint8_t* path, void* output) noexcept
{
    RET_OK(platform::RtSys::stat_byte(path, output));
}

RtResult<int32_t> Interop::sys_stat_string(vm::RtString* path, void* output) noexcept
{
    RET_OK(platform::RtSys::stat_string(path, output));
}

RtResult<uint8_t*> Interop::sys_str_error_r(int32_t error, uint8_t* buffer, int32_t buffer_size) noexcept
{
    RET_OK(platform::RtSys::str_error_r(error, buffer, buffer_size));
}

RtResult<int32_t> Interop::sys_symlink(vm::RtString* target, vm::RtString* link_path) noexcept
{
    RET_OK(platform::RtSys::symlink(target, link_path));
}

RtResult<int32_t> Interop::sys_unlink(vm::RtString* path) noexcept
{
    RET_OK(platform::RtSys::unlink(path));
}

RtResult<int32_t> Interop::sys_utime(vm::RtString* path, void* time_buffer) noexcept
{
    RET_OK(platform::RtSys::utime(path, time_buffer));
}

RtResult<int32_t> Interop::sys_utimes(vm::RtString* path, void* time_value_pair) noexcept
{
    RET_OK(platform::RtSys::utimes(path, time_value_pair));
}

/// @icall:
/// Interop/Globalization::GetTimeZoneDisplayName(System.String,System.String,Interop/Globalization/TimeZoneDisplayNameType,System.Text.StringBuilder,System.Int32)
RtResultVoid globalization_get_time_zone_display_name_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto locale_name = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto time_zone_id = EvalStackOp::get_param<vm::RtString*>(params, 1);
    auto type = EvalStackOp::get_param<int32_t>(params, 2);
    auto result = EvalStackOp::get_param<vm::RtObject*>(params, 3);
    auto result_length = EvalStackOp::get_param<int32_t>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, rc,
                                            Interop::globalization_get_time_zone_display_name(locale_name, time_zone_id, type, result, result_length));
    EvalStackOp::set_return(ret, rc);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::ChMod(System.String,System.Int32)
RtResultVoid sys_ch_mod_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto mode = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_ch_mod(path, mode));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::CloseDir(System.IntPtr)
RtResultVoid sys_close_dir_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                   interp::RtStackObject* ret) noexcept
{
    auto dir = EvalStackOp::get_param<intptr_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_close_dir(dir));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::ConvertErrorPalToPlatform(Interop/Error)
RtResultVoid sys_convert_error_pal_to_platform_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                       interp::RtStackObject* ret) noexcept
{
    auto error = EvalStackOp::get_param<int32_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_convert_error_pal_to_platform(error));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::ConvertErrorPlatformToPal(System.Int32)
RtResultVoid sys_convert_error_platform_to_pal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                       interp::RtStackObject* ret) noexcept
{
    auto error = EvalStackOp::get_param<int32_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_convert_error_platform_to_pal(error));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::CopyFile(Microsoft.Win32.SafeHandles.SafeFileHandle,Microsoft.Win32.SafeHandles.SafeFileHandle)
RtResultVoid sys_copy_file_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                   interp::RtStackObject* ret) noexcept
{
    auto source = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    auto destination = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_copy_file(source, destination));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::FStat(Microsoft.Win32.SafeHandles.SafeFileHandle,Interop/Sys/FileStatus&)
RtResultVoid sys_f_stat_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
    auto fd = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    auto output = EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_f_stat(fd, output));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::GetEGid()
RtResultVoid sys_get_e_gid_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                   interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, result, Interop::sys_get_e_gid());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::GetEUid()
RtResultVoid sys_get_e_uid_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                   interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, result, Interop::sys_get_e_uid());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::GetNonCryptographicallySecureRandomBytes(System.Byte*,System.Int32)
RtResultVoid sys_get_non_cryptographically_secure_random_bytes_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto buffer = EvalStackOp::get_param<uint8_t*>(params, 0);
    auto length = EvalStackOp::get_param<int32_t>(params, 1);
    (void)ret;
    RET_ERR_ON_FAIL(Interop::sys_get_non_cryptographically_secure_random_bytes(buffer, length));
    RET_VOID_OK();
}

/// @icall: Interop/Sys::GetReadDirRBufferSize()
RtResultVoid sys_get_read_dir_r_buffer_size_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                                    interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_get_read_dir_r_buffer_size());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::LChflags(System.String,System.UInt32)
RtResultVoid sys_lchflags_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto flags = EvalStackOp::get_param<uint32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_lchflags(path, flags));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::Link(System.String,System.String)
RtResultVoid sys_link_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                              interp::RtStackObject* ret) noexcept
{
    auto source = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto target = EvalStackOp::get_param<vm::RtString*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_link(source, target));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::LStat(System.Byte&,Interop/Sys/FileStatus&)
RtResultVoid sys_lstat_byte_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                    interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<uint8_t*>(params, 0);
    auto output = EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_lstat_byte(path, output));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::LStat(System.String,Interop/Sys/FileStatus&)
RtResultVoid sys_lstat_string_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto output = EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_lstat_string(path, output));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::MkDir(System.String,System.Int32)
RtResultVoid sys_mkdir_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                               interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto mode = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_mkdir(path, mode));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::OpenDir(System.String)
RtResultVoid sys_open_dir_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, result, Interop::sys_open_dir(path));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::ReadDirR(System.IntPtr,System.Byte*,System.Int32,Interop/Sys/DirectoryEntry&)
RtResultVoid sys_read_dir_r_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                    interp::RtStackObject* ret) noexcept
{
    auto dir = EvalStackOp::get_param<intptr_t>(params, 0);
    auto buffer = EvalStackOp::get_param<uint8_t*>(params, 1);
    auto buffer_size = EvalStackOp::get_param<int32_t>(params, 2);
    auto output_entry = EvalStackOp::get_param<void*>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_read_dir_r(dir, buffer, buffer_size, output_entry));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::ReadLink(System.String,System.Byte[],System.Int32)
RtResultVoid sys_read_link_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                   interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto buffer = EvalStackOp::get_param<vm::RtArray*>(params, 1);
    auto buffer_size = EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_read_link(path, buffer, buffer_size));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::Rename(System.String,System.String)
RtResultVoid sys_rename_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
    auto old_path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto new_path = EvalStackOp::get_param<vm::RtString*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_rename(old_path, new_path));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::RmDir(System.String)
RtResultVoid sys_rmdir_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                               interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_rmdir(path));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::Stat(System.Byte&,Interop/Sys/FileStatus&)
RtResultVoid sys_stat_byte_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                   interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<uint8_t*>(params, 0);
    auto output = EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_stat_byte(path, output));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::Stat(System.String,Interop/Sys/FileStatus&)
RtResultVoid sys_stat_string_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto output = EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_stat_string(path, output));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::StrErrorR(System.Int32,System.Byte*,System.Int32)
RtResultVoid sys_str_error_r_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto error = EvalStackOp::get_param<int32_t>(params, 0);
    auto buffer = EvalStackOp::get_param<uint8_t*>(params, 1);
    auto buffer_size = EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint8_t*, result, Interop::sys_str_error_r(error, buffer, buffer_size));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::Symlink(System.String,System.String)
RtResultVoid sys_symlink_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    auto target = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto link_path = EvalStackOp::get_param<vm::RtString*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_symlink(target, link_path));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::Unlink(System.String)
RtResultVoid sys_unlink_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_unlink(path));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::UTime(System.String,Interop/Sys/UTimBuf&)
RtResultVoid sys_utime_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                               interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto time_buffer = EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_utime(path, time_buffer));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Interop/Sys::UTimes(System.String,Interop/Sys/TimeValPair&)
RtResultVoid sys_utimes_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto time_value_pair = EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::sys_utimes(path, time_value_pair));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}
#endif

RtResult<uint32_t> Interop::bcrypt_gen_random(intptr_t algo_handle, uint8_t* buffer, int32_t length, int32_t flags) noexcept
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

RtResult<bool> Interop::kernel32_set_thread_error_mode(uint32_t mode, uint32_t& old_mode) noexcept
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

RtResult<bool> Interop::kernel32_get_file_attributes_ex_private(vm::RtString* name, uint32_t file_info_level, void* file_info) noexcept
{
    return platform::Kernel32::get_file_attributes_ex_private(name, file_info_level, file_info);
}

/// @icall: Interop/Kernel32::GetFileAttributesExPrivate(System.String,Interop/Kernel32/GET_FILEEX_INFO_LEVELS, Interop/Kernel32/WIN32_FILE_ATTRIBUTE_DATA&)
RtResultVoid kernel32_get_file_attributes_ex_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
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
                                                                     uint32_t f_search_op, intptr_t lp_search_filter, int32_t dw_additional_flags) noexcept
{
    static metadata::RtClass* safe_find_handle_class = nullptr;
    if (!safe_find_handle_class)
    {
        metadata::RtModuleDef* mod = vm::Assembly::get_corlib()->mod;
        UNWRAP_OR_RET_ERR_ON_FAIL(safe_find_handle_class, mod->get_class_by_name("Microsoft.Win32.SafeHandles.SafeFindHandle", false, true));
    }
    intptr_t handle =
        platform::Kernel32::find_first_file_ex_private(lp_file_name, f_info_level_id, lp_find_file_data, f_search_op, lp_search_filter, dw_additional_flags);
    const metadata::RtClass* safe_handle_klass = safe_find_handle_class->parent->parent;
    assert(safe_handle_klass && std::strcmp(safe_handle_klass->name, "SafeHandle") == 0);
    assert(std::strcmp(safe_handle_klass->fields[0].name, "handle") == 0 && safe_handle_klass->fields[0].offset == 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, safe_handle, vm::Object::new_object(safe_handle_klass));
    reinterpret_cast<SafeFindHandle*>(safe_handle)->handle = handle;
    RET_OK(safe_handle);
}

/// @icall:
/// Interop/Kernel32::FindFirstFileExPrivate(System.String,Interop/Kernel32/FINDEX_INFO_LEVELS,Interop/Kernel32/WIN32_FIND_DATA&,Interop/Kernel32/FINDEX_SEARCH_OPS,System.IntPtr,System.Int32)
RtResultVoid kernel32_find_first_file_ex_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                         interp::RtStackObject* ret) noexcept
{
    vm::RtString* lp_file_name = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    uint32_t f_info_level_id = interp::EvalStackOp::get_param<uint32_t>(params, 1);
    void* lp_find_file_data = interp::EvalStackOp::get_param<void*>(params, 2);
    uint32_t f_search_op = interp::EvalStackOp::get_param<uint32_t>(params, 3);
    intptr_t lp_search_filter = interp::EvalStackOp::get_param<intptr_t>(params, 4);
    int32_t dw_additional_flags = interp::EvalStackOp::get_param<int32_t>(params, 5);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        vm::RtObject*, result,
        Interop::kernel32_find_first_file_ex_private(lp_file_name, f_info_level_id, lp_find_file_data, f_search_op, lp_search_filter, dw_additional_flags));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<int32_t> Interop::windows_console_get_console_cp() noexcept
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

RtResult<int32_t> Interop::windows_console_get_console_output_cp() noexcept
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
#if LEANCLR_PLATFORM_POSIX
    {"Interop/Sys::DoubleToString(System.Double,System.Byte*,System.Byte*,System.Int32)", (vm::InternalCallFunction)&Interop::double_to_string,
     double_to_string_invoker},
    {"Interop/Sys::LChflagsCanSetHiddenFlag", (vm::InternalCallFunction)&Interop::sys_lchflags_can_set_hidden_flag, sys_lchflags_can_set_hidden_flag_invoker},
    {"Interop/Globalization::GetTimeZoneDisplayName(System.String,System.String,Interop/Globalization/"
     "TimeZoneDisplayNameType,System.Text.StringBuilder,System.Int32)",
     (vm::InternalCallFunction)&Interop::globalization_get_time_zone_display_name, globalization_get_time_zone_display_name_invoker},
    {"Interop/Sys::ChMod(System.String,System.Int32)", (vm::InternalCallFunction)&Interop::sys_ch_mod, sys_ch_mod_invoker},
    {"Interop/Sys::CloseDir(System.IntPtr)", (vm::InternalCallFunction)&Interop::sys_close_dir, sys_close_dir_invoker},
    {"Interop/Sys::ConvertErrorPalToPlatform(Interop/Error)", (vm::InternalCallFunction)&Interop::sys_convert_error_pal_to_platform,
     sys_convert_error_pal_to_platform_invoker},
    {"Interop/Sys::ConvertErrorPlatformToPal(System.Int32)", (vm::InternalCallFunction)&Interop::sys_convert_error_platform_to_pal,
     sys_convert_error_platform_to_pal_invoker},
    {"Interop/Sys::CopyFile(Microsoft.Win32.SafeHandles.SafeFileHandle,Microsoft.Win32.SafeHandles.SafeFileHandle)",
     (vm::InternalCallFunction)&Interop::sys_copy_file, sys_copy_file_invoker},
    {"Interop/Sys::FStat(Microsoft.Win32.SafeHandles.SafeFileHandle,Interop/Sys/FileStatus&)", (vm::InternalCallFunction)&Interop::sys_f_stat,
     sys_f_stat_invoker},
    {"Interop/Sys::GetEGid()", (vm::InternalCallFunction)&Interop::sys_get_e_gid, sys_get_e_gid_invoker},
    {"Interop/Sys::GetEUid()", (vm::InternalCallFunction)&Interop::sys_get_e_uid, sys_get_e_uid_invoker},
    {"Interop/Sys::GetNonCryptographicallySecureRandomBytes(System.Byte*,System.Int32)",
     (vm::InternalCallFunction)&Interop::sys_get_non_cryptographically_secure_random_bytes, sys_get_non_cryptographically_secure_random_bytes_invoker},
    {"Interop/Sys::GetReadDirRBufferSize()", (vm::InternalCallFunction)&Interop::sys_get_read_dir_r_buffer_size, sys_get_read_dir_r_buffer_size_invoker},
    {"Interop/Sys::LChflags(System.String,System.UInt32)", (vm::InternalCallFunction)&Interop::sys_lchflags, sys_lchflags_invoker},
    {"Interop/Sys::Link(System.String,System.String)", (vm::InternalCallFunction)&Interop::sys_link, sys_link_invoker},
    {"Interop/Sys::LStat(System.Byte&,Interop/Sys/FileStatus&)", (vm::InternalCallFunction)&Interop::sys_lstat_byte, sys_lstat_byte_invoker},
    {"Interop/Sys::LStat(System.String,Interop/Sys/FileStatus&)", (vm::InternalCallFunction)&Interop::sys_lstat_string, sys_lstat_string_invoker},
    {"Interop/Sys::MkDir(System.String,System.Int32)", (vm::InternalCallFunction)&Interop::sys_mkdir, sys_mkdir_invoker},
    {"Interop/Sys::OpenDir(System.String)", (vm::InternalCallFunction)&Interop::sys_open_dir, sys_open_dir_invoker},
    {"Interop/Sys::ReadDirR(System.IntPtr,System.Byte*,System.Int32,Interop/Sys/DirectoryEntry&)", (vm::InternalCallFunction)&Interop::sys_read_dir_r,
     sys_read_dir_r_invoker},
    {"Interop/Sys::ReadLink(System.String,System.Byte[],System.Int32)", (vm::InternalCallFunction)&Interop::sys_read_link, sys_read_link_invoker},
    {"Interop/Sys::Rename(System.String,System.String)", (vm::InternalCallFunction)&Interop::sys_rename, sys_rename_invoker},
    {"Interop/Sys::RmDir(System.String)", (vm::InternalCallFunction)&Interop::sys_rmdir, sys_rmdir_invoker},
    {"Interop/Sys::Stat(System.Byte&,Interop/Sys/FileStatus&)", (vm::InternalCallFunction)&Interop::sys_stat_byte, sys_stat_byte_invoker},
    {"Interop/Sys::Stat(System.String,Interop/Sys/FileStatus&)", (vm::InternalCallFunction)&Interop::sys_stat_string, sys_stat_string_invoker},
    {"Interop/Sys::StrErrorR(System.Int32,System.Byte*,System.Int32)", (vm::InternalCallFunction)&Interop::sys_str_error_r, sys_str_error_r_invoker},
    {"Interop/Sys::Symlink(System.String,System.String)", (vm::InternalCallFunction)&Interop::sys_symlink, sys_symlink_invoker},
    {"Interop/Sys::Unlink(System.String)", (vm::InternalCallFunction)&Interop::sys_unlink, sys_unlink_invoker},
    {"Interop/Sys::UTime(System.String,Interop/Sys/UTimBuf&)", (vm::InternalCallFunction)&Interop::sys_utime, sys_utime_invoker},
    {"Interop/Sys::UTimes(System.String,Interop/Sys/TimeValPair&)", (vm::InternalCallFunction)&Interop::sys_utimes, sys_utimes_invoker},
#endif
    {"Interop/BCrypt::BCryptGenRandom(System.IntPtr,System.Byte*,System.Int32,System.Int32)", (vm::InternalCallFunction)&Interop::bcrypt_gen_random,
     bcrypt_gen_random_invoker},
    {"Interop/Kernel32::SetThreadErrorMode(System.UInt32,System.UInt32&)", (vm::InternalCallFunction)&Interop::kernel32_set_thread_error_mode,
     kernel32_set_thread_error_mode_invoker},
    {"Interop/Kernel32::GetFileAttributesExPrivate(System.String,Interop/Kernel32/GET_FILEEX_INFO_LEVELS,Interop/Kernel32/WIN32_FILE_ATTRIBUTE_DATA&)",
     (vm::InternalCallFunction)&Interop::kernel32_get_file_attributes_ex_private, kernel32_get_file_attributes_ex_private_invoker},
    {"Interop/Kernel32::FindFirstFileExPrivate(System.String,Interop/Kernel32/FINDEX_INFO_LEVELS,Interop/Kernel32/WIN32_FIND_DATA&,Interop/Kernel32/"
     "FINDEX_SEARCH_OPS,System.IntPtr,System.Int32)",
     (vm::InternalCallFunction)&Interop::kernel32_find_first_file_ex_private, kernel32_find_first_file_ex_private_invoker},
    {"System.Console/WindowsConsole::GetConsoleCP", (vm::InternalCallFunction)&Interop::windows_console_get_console_cp, windows_console_get_console_cp_invoker},
    {"System.Console/WindowsConsole::GetConsoleOutputCP", (vm::InternalCallFunction)&Interop::windows_console_get_console_output_cp,
     windows_console_get_console_output_cp_invoker},
};

utils::Span<vm::InternalCallEntry> Interop::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_interop_internal_call_entries, sizeof(s_interop_internal_call_entries) / sizeof(vm::InternalCallEntry));
}
} // namespace icalls
} // namespace leanclr

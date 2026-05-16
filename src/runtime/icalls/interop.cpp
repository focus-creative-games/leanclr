#include "interop.h"

#include "vm/class.h"
#include "vm/object.h"
#include "vm/assembly.h"
#include "vm/field.h"
#include "vm/rt_array.h"
#include "metadata/module_def.h"
#include "platform/bcrypt.h"
#include "platform/kernel32.h"
#include "platform/rt_sys.h"
#include "vmutils/stringbuilder.h"
#include "vmutils/safehandle.h"

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

#if LEANCLR_PLATFORM_WIN
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
    return vmutils::SafeHandle::new_safe_handle(safe_find_handle_class, handle);
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

RtResult<uint32_t> Interop::kernel32_get_time_zone_information(void* lp_time_zone_information) noexcept
{
    RET_OK(platform::Kernel32::get_time_zone_information(lp_time_zone_information));
}

/// @icall: Interop/Kernel32::GetTimeZoneInformation(Interop/Kernel32/TIME_ZONE_INFORMATION&)
RtResultVoid kernel32_get_time_zone_information_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                        interp::RtStackObject* ret) noexcept
{
    void* lp_time_zone_information = interp::EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, result, Interop::kernel32_get_time_zone_information(lp_time_zone_information));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<uint32_t> Interop::kernel32_get_dynamic_time_zone_information(void* lp_dynamic_tz) noexcept
{
    RET_OK(platform::Kernel32::get_dynamic_time_zone_information(lp_dynamic_tz));
}

/// @icall: Interop/Kernel32::GetDynamicTimeZoneInformation(Interop/Kernel32/TIME_DYNAMIC_ZONE_INFORMATION&)
RtResultVoid kernel32_get_dynamic_time_zone_information_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    void* p = interp::EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, result, Interop::kernel32_get_dynamic_time_zone_information(p));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_delete_volume_mount_point_private(vm::RtString* mount_point) noexcept
{
    RET_OK(platform::Kernel32::delete_volume_mount_point_private(mount_point));
}

/// @icall: Interop/Kernel32::DeleteVolumeMountPointPrivate(System.String)
RtResultVoid kernel32_delete_volume_mount_point_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtString* s = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_delete_volume_mount_point_private(s));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_free_library(intptr_t h_module) noexcept
{
    RET_OK(platform::Kernel32::free_library(h_module));
}

/// @icall: Interop/Kernel32::FreeLibrary(System.IntPtr)
RtResultVoid kernel32_free_library_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    intptr_t h = interp::EvalStackOp::get_param<intptr_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_free_library(h));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtObject*> Interop::kernel32_load_library_ex(vm::RtString* lib_filename, intptr_t reserved, int32_t flags) noexcept
{
    static metadata::RtClass* safe_library_handle_class = nullptr;
    if (!safe_library_handle_class)
    {
        metadata::RtModuleDef* mod = vm::Assembly::get_corlib()->mod;
        UNWRAP_OR_RET_ERR_ON_FAIL(safe_library_handle_class, mod->get_class_by_name("Microsoft.Win32.SafeHandles.SafeLibraryHandle", false, true));
    }
    intptr_t h = platform::Kernel32::load_library_ex(lib_filename, reserved, flags);
    return vmutils::SafeHandle::new_safe_handle(safe_library_handle_class, h);
}

/// @icall: Interop/Kernel32::LoadLibraryEx(System.String,System.IntPtr,System.Int32)
RtResultVoid kernel32_load_library_ex_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    vm::RtString* name = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    intptr_t reserved = interp::EvalStackOp::get_param<intptr_t>(params, 1);
    int32_t flags = interp::EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, Interop::kernel32_load_library_ex(name, reserved, flags));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

// RtResult<bool> Interop::kernel32_get_file_mui_path(uint32_t flags, vm::RtString* file_path, vm::RtObject* language, int32_t* language_length_chars,
//                                                    vm::RtObject* file_mui_path, int32_t* file_mui_path_length_chars, int64_t* enumerator) noexcept
// {
//     RETURN_NOT_IMPLEMENTED_ERROR();
// }

// /// @icall:
// /// Interop/Kernel32::GetFileMUIPath(System.UInt32,System.String,System.Text.StringBuilder,System.Int32&,System.Text.StringBuilder,System.Int32&,System.Int64&)
// RtResultVoid kernel32_get_file_mui_path_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
//                                                 interp::RtStackObject* ret) noexcept
// {
//     uint32_t fl = interp::EvalStackOp::get_param<uint32_t>(params, 0);
//     vm::RtString* path = interp::EvalStackOp::get_param<vm::RtString*>(params, 1);
//     vm::RtObject* lang = interp::EvalStackOp::get_param<vm::RtObject*>(params, 2);
//     int32_t* plc = interp::EvalStackOp::get_param<int32_t*>(params, 3);
//     vm::RtObject* mui = interp::EvalStackOp::get_param<vm::RtObject*>(params, 4);
//     int32_t* pmui_c = interp::EvalStackOp::get_param<int32_t*>(params, 5);
//     int64_t* penum = interp::EvalStackOp::get_param<int64_t*>(params, 6);
//     DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_get_file_mui_path(fl, path, lang, plc, mui, pmui_c, penum));
//     EvalStackOp::set_return(ret, result);
//     RET_VOID_OK();
// }

RtResult<bool> Interop::kernel32_close_handle(intptr_t handle) noexcept
{
    RET_OK(platform::Kernel32::close_handle(handle));
}

/// @icall: Interop/Kernel32::CloseHandle(System.IntPtr)
RtResultVoid kernel32_close_handle_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    intptr_t h = interp::EvalStackOp::get_param<intptr_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_close_handle(h));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<int32_t> Interop::kernel32_copy_file2(vm::RtString* existing, vm::RtString* new_file, void* extended_parameters) noexcept
{
    if (existing == nullptr || new_file == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    assert(extended_parameters != nullptr);
    RET_OK(platform::Kernel32::copy_file2(existing, new_file, extended_parameters));
}

/// @icall: Interop/Kernel32::CopyFile2(System.String,System.String,Interop/Kernel32/COPYFILE2_EXTENDED_PARAMETERS&)
RtResultVoid kernel32_copy_file2_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                         interp::RtStackObject* ret) noexcept
{
    vm::RtString* a = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    vm::RtString* b = interp::EvalStackOp::get_param<vm::RtString*>(params, 1);
    void* p = interp::EvalStackOp::get_param<void*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::kernel32_copy_file2(a, b, p));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_copy_file_ex_private(vm::RtString* src, vm::RtString* dst, intptr_t progress_routine, intptr_t progress_data, int32_t* cancel,
                                                      int32_t flags) noexcept
{
    if (src == nullptr || dst == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    assert(cancel != nullptr);
    RET_OK(platform::Kernel32::copy_file_ex_private(src, dst, progress_routine, progress_data, cancel, flags));
}

/// @icall: Interop/Kernel32::CopyFileExPrivate(System.String,System.String,System.IntPtr,System.IntPtr,System.Int32&,System.Int32)
RtResultVoid kernel32_copy_file_ex_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* ret) noexcept
{
    vm::RtString* s = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    vm::RtString* d = interp::EvalStackOp::get_param<vm::RtString*>(params, 1);
    intptr_t pr = interp::EvalStackOp::get_param<intptr_t>(params, 2);
    intptr_t pd = interp::EvalStackOp::get_param<intptr_t>(params, 3);
    int32_t* cancel = interp::EvalStackOp::get_param<int32_t*>(params, 4);
    int32_t fl = interp::EvalStackOp::get_param<int32_t>(params, 5);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_copy_file_ex_private(s, d, pr, pd, cancel, fl));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_create_directory_private(vm::RtString* path, void* security_attributes) noexcept
{
    if (path == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    assert(security_attributes != nullptr);
    RET_OK(platform::Kernel32::create_directory_private(path, security_attributes));
}

/// @icall: Interop/Kernel32::CreateDirectoryPrivate(System.String,Interop/Kernel32/SECURITY_ATTRIBUTES&)
RtResultVoid kernel32_create_directory_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                       interp::RtStackObject* ret) noexcept
{
    vm::RtString* p = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    void* sa = interp::EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_create_directory_private(p, sa));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<intptr_t> Interop::kernel32_create_file_private(vm::RtString* name, int32_t desired_access, int32_t share_mode, void* security_attributes,
                                                         int32_t creation_disposition, int32_t flags_and_attributes, intptr_t template_file) noexcept
{
    if (name == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    assert(security_attributes != nullptr);
    RET_OK(platform::Kernel32::create_file_private(name, desired_access, share_mode, security_attributes, creation_disposition, flags_and_attributes,
                                                   template_file));
}

/// @icall:
/// Interop/Kernel32::CreateFilePrivate(System.String,System.Int32,System.IO.FileShare,Interop/Kernel32/SECURITY_ATTRIBUTES*,System.IO.FileMode,System.Int32,System.IntPtr)
RtResultVoid kernel32_create_file_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                  interp::RtStackObject* ret) noexcept
{
    vm::RtString* n = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    int32_t da = interp::EvalStackOp::get_param<int32_t>(params, 1);
    int32_t sm = interp::EvalStackOp::get_param<int32_t>(params, 2);
    void* sa = interp::EvalStackOp::get_param<void*>(params, 3);
    int32_t cd = interp::EvalStackOp::get_param<int32_t>(params, 4);
    int32_t fa = interp::EvalStackOp::get_param<int32_t>(params, 5);
    intptr_t tf = interp::EvalStackOp::get_param<intptr_t>(params, 6);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, result, Interop::kernel32_create_file_private(n, da, sm, sa, cd, fa, tf));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_delete_file_private(vm::RtString* path) noexcept
{
    RET_OK(platform::Kernel32::delete_file_private(path));
}

/// @icall: Interop/Kernel32::DeleteFilePrivate(System.String)
RtResultVoid kernel32_delete_file_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                  interp::RtStackObject* ret) noexcept
{
    vm::RtString* p = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_delete_file_private(p));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_find_next_file(vm::RtObject* find_handle, void* find_file_data) noexcept
{
    if (find_handle == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    RET_OK(platform::Kernel32::find_next_file(vmutils::SafeHandle::get_handle(find_handle), find_file_data));
}

/// @icall: Interop/Kernel32::FindNextFile(Microsoft.Win32.SafeHandles.SafeFindHandle,Interop/Kernel32/WIN32_FIND_DATA&)
RtResultVoid kernel32_find_next_file_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    vm::RtObject* h = interp::EvalStackOp::get_param<vm::RtObject*>(params, 0);
    void* d = interp::EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_find_next_file(h, d));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

// RtResult<int32_t> Interop::kernel32_format_message(int32_t flags, intptr_t source, uint32_t message_id, int32_t language_id, Utf16Char* buffer,
//                                                    int32_t buffer_chars, vm::RtArray* arguments) noexcept
// {
//     intptr_t stack_args[64];
//     intptr_t* args_ptr = nullptr;
//     int32_t arg_count = 0;
//     if (arguments != nullptr && vm::Array::get_array_length(arguments) > 0)
//     {
//         arg_count = vm::Array::get_array_length(arguments);
//         if (arg_count > 64)
//             arg_count = 64;
//         for (int32_t i = 0; i < arg_count; ++i)
//             stack_args[i] = vm::Array::get_array_data_at<intptr_t>(arguments, i);
//         args_ptr = stack_args;
//     }
//     RET_OK(platform::Kernel32::format_message(flags, source, message_id, language_id, buffer, buffer_chars, args_ptr, arg_count));
// }

// /// @icall: Interop/Kernel32::FormatMessage(System.Int32,System.IntPtr,System.UInt32,System.Int32,System.Char*,System.Int32,System.IntPtr[])
// RtResultVoid kernel32_format_message_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
//                                              interp::RtStackObject* ret) noexcept
// {
//     int32_t fl = interp::EvalStackOp::get_param<int32_t>(params, 0);
//     intptr_t src = interp::EvalStackOp::get_param<intptr_t>(params, 1);
//     uint32_t mid = interp::EvalStackOp::get_param<uint32_t>(params, 2);
//     int32_t lid = interp::EvalStackOp::get_param<int32_t>(params, 3);
//     Utf16Char* buf = interp::EvalStackOp::get_param<Utf16Char*>(params, 4);
//     int32_t n = interp::EvalStackOp::get_param<int32_t>(params, 5);
//     vm::RtArray* args = interp::EvalStackOp::get_param<vm::RtArray*>(params, 6);
//     DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::kernel32_format_message(fl, src, mid, lid, buf, n, args));
//     EvalStackOp::set_return(ret, result);
//     RET_VOID_OK();
// }

RtResult<bool> Interop::kernel32_get_file_information_by_handle_ex(intptr_t h_file, int32_t file_information_class, void* file_information,
                                                                   uint32_t buffer_size) noexcept
{
    RET_OK(platform::Kernel32::get_file_information_by_handle_ex(h_file, file_information_class, file_information, buffer_size));
}

/// @icall: Interop/Kernel32::GetFileInformationByHandleEx(System.IntPtr,Interop/Kernel32/FILE_INFO_BY_HANDLE_CLASS,System.IntPtr,System.UInt32)
RtResultVoid kernel32_get_file_information_by_handle_ex_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    intptr_t h = interp::EvalStackOp::get_param<intptr_t>(params, 0);
    int32_t cl = interp::EvalStackOp::get_param<int32_t>(params, 1);
    void* info = interp::EvalStackOp::get_param<void*>(params, 2);
    uint32_t sz = interp::EvalStackOp::get_param<uint32_t>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_get_file_information_by_handle_ex(h, cl, info, sz));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<int32_t> Interop::kernel32_get_logical_drives() noexcept
{
    RET_OK(platform::Kernel32::get_logical_drives());
}

/// @icall: Interop/Kernel32::GetLogicalDrives()
RtResultVoid kernel32_get_logical_drives_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                                 interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::kernel32_get_logical_drives());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_move_file_ex_private(vm::RtString* src, vm::RtString* dst, uint32_t flags) noexcept
{
    if (src == nullptr || dst == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    RET_OK(platform::Kernel32::move_file_ex_private(src, dst, flags));
}

RtResultVoid kernel32_move_file_ex_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* ret) noexcept
{
    vm::RtString* s = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    vm::RtString* d = interp::EvalStackOp::get_param<vm::RtString*>(params, 1);
    uint32_t f = interp::EvalStackOp::get_param<uint32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_move_file_ex_private(s, d, f));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_remove_directory_private(vm::RtString* path) noexcept
{
    if (path == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    RET_OK(platform::Kernel32::remove_directory_private(path));
}

RtResultVoid kernel32_remove_directory_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                       interp::RtStackObject* ret) noexcept
{
    vm::RtString* p = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_remove_directory_private(p));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_replace_file_private(vm::RtString* replaced_file_name, vm::RtString* replacement_file_name, vm::RtString* backup_file_name,
                                                      int32_t replace_flags, intptr_t exclude, intptr_t reserved) noexcept
{
    if (replaced_file_name == nullptr || replacement_file_name == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    RET_OK(platform::Kernel32::replace_file_private(replaced_file_name, replacement_file_name, backup_file_name, replace_flags, exclude, reserved));
}

RtResultVoid kernel32_replace_file_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* ret) noexcept
{
    vm::RtString* a = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    vm::RtString* b = interp::EvalStackOp::get_param<vm::RtString*>(params, 1);
    vm::RtString* c = interp::EvalStackOp::get_param<vm::RtString*>(params, 2);
    int32_t rf = interp::EvalStackOp::get_param<int32_t>(params, 3);
    intptr_t ex = interp::EvalStackOp::get_param<intptr_t>(params, 4);
    intptr_t rs = interp::EvalStackOp::get_param<intptr_t>(params, 5);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_replace_file_private(a, b, c, rf, ex, rs));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_set_file_attributes_private(vm::RtString* name, int32_t attributes) noexcept
{
    if (name == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    RET_OK(platform::Kernel32::set_file_attributes_private(name, attributes));
}

RtResultVoid kernel32_set_file_attributes_private_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                          interp::RtStackObject* ret) noexcept
{
    vm::RtString* n = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    int32_t a = interp::EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_set_file_attributes_private(n, a));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> Interop::kernel32_set_file_information_by_handle(vm::RtObject* h_file, int32_t file_information_class, void* file_information,
                                                                uint32_t buffer_size) noexcept
{
    if (h_file == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    assert(file_information != nullptr);
    RET_OK(platform::Kernel32::set_file_information_by_handle(vmutils::SafeHandle::get_handle(h_file), file_information_class, file_information, buffer_size));
}

/// @icall:
/// Interop/Kernel32::SetFileInformationByHandle(Microsoft.Win32.SafeHandles.SafeFileHandle,Interop/Kernel32/FILE_INFO_BY_HANDLE_CLASS,Interop/Kernel32/FILE_BASIC_INFO&,System.UInt32)
RtResultVoid kernel32_set_file_information_by_handle_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtObject* h = interp::EvalStackOp::get_param<vm::RtObject*>(params, 0);
    int32_t cl = interp::EvalStackOp::get_param<int32_t>(params, 1);
    void* info = interp::EvalStackOp::get_param<void*>(params, 2);
    uint32_t sz = interp::EvalStackOp::get_param<uint32_t>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, Interop::kernel32_set_file_information_by_handle(h, cl, info, sz));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}
#endif

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
#if LEANCLR_PLATFORM_WIN
    {"Interop/BCrypt::BCryptGenRandom(System.IntPtr,System.Byte*,System.Int32,System.Int32)", (vm::InternalCallFunction)&Interop::bcrypt_gen_random,
     bcrypt_gen_random_invoker},
    {"Interop/Kernel32::SetThreadErrorMode(System.UInt32,System.UInt32&)", (vm::InternalCallFunction)&Interop::kernel32_set_thread_error_mode,
     kernel32_set_thread_error_mode_invoker},
    {"Interop/Kernel32::GetFileAttributesExPrivate(System.String,Interop/Kernel32/GET_FILEEX_INFO_LEVELS,Interop/Kernel32/WIN32_FILE_ATTRIBUTE_DATA&)",
     (vm::InternalCallFunction)&Interop::kernel32_get_file_attributes_ex_private, kernel32_get_file_attributes_ex_private_invoker},
    {"Interop/Kernel32::FindFirstFileExPrivate(System.String,Interop/Kernel32/FINDEX_INFO_LEVELS,Interop/Kernel32/WIN32_FIND_DATA&,Interop/Kernel32/"
     "FINDEX_SEARCH_OPS,System.IntPtr,System.Int32)",
     (vm::InternalCallFunction)&Interop::kernel32_find_first_file_ex_private, kernel32_find_first_file_ex_private_invoker},
    {"Interop/Kernel32::GetTimeZoneInformation(Interop/Kernel32/TIME_ZONE_INFORMATION&)",
     (vm::InternalCallFunction)&Interop::kernel32_get_time_zone_information, kernel32_get_time_zone_information_invoker},
    {"Interop/Kernel32::GetDynamicTimeZoneInformation(Interop/Kernel32/TIME_DYNAMIC_ZONE_INFORMATION&)",
     (vm::InternalCallFunction)&Interop::kernel32_get_dynamic_time_zone_information, kernel32_get_dynamic_time_zone_information_invoker},
    {"Interop/Kernel32::DeleteVolumeMountPointPrivate(System.String)", (vm::InternalCallFunction)&Interop::kernel32_delete_volume_mount_point_private,
     kernel32_delete_volume_mount_point_private_invoker},
    {"Interop/Kernel32::FreeLibrary(System.IntPtr)", (vm::InternalCallFunction)&Interop::kernel32_free_library, kernel32_free_library_invoker},
    {"Interop/Kernel32::LoadLibraryEx(System.String,System.IntPtr,System.Int32)", (vm::InternalCallFunction)&Interop::kernel32_load_library_ex,
     kernel32_load_library_ex_invoker},
    // {"Interop/Kernel32::GetFileMUIPath(System.UInt32,System.String,System.Text.StringBuilder,System.Int32&,System.Text.StringBuilder,System.Int32&,"
    //  "System.Int64&)",
    //  (vm::InternalCallFunction)&Interop::kernel32_get_file_mui_path, kernel32_get_file_mui_path_invoker},
    {"Interop/Kernel32::CloseHandle(System.IntPtr)", (vm::InternalCallFunction)&Interop::kernel32_close_handle, kernel32_close_handle_invoker},
    {"Interop/Kernel32::CopyFile2(System.String,System.String,Interop/Kernel32/COPYFILE2_EXTENDED_PARAMETERS&)",
     (vm::InternalCallFunction)&Interop::kernel32_copy_file2, kernel32_copy_file2_invoker},
    {"Interop/Kernel32::CopyFileExPrivate(System.String,System.String,System.IntPtr,System.IntPtr,System.Int32&,System.Int32)",
     (vm::InternalCallFunction)&Interop::kernel32_copy_file_ex_private, kernel32_copy_file_ex_private_invoker},
    {"Interop/Kernel32::CreateDirectoryPrivate(System.String,Interop/Kernel32/SECURITY_ATTRIBUTES&)",
     (vm::InternalCallFunction)&Interop::kernel32_create_directory_private, kernel32_create_directory_private_invoker},
    {"Interop/Kernel32::CreateFilePrivate(System.String,System.Int32,System.IO.FileShare,Interop/Kernel32/"
     "SECURITY_ATTRIBUTES*,System.IO.FileMode,System.Int32,System.IntPtr)",
     (vm::InternalCallFunction)&Interop::kernel32_create_file_private, kernel32_create_file_private_invoker},
    {"Interop/Kernel32::DeleteFilePrivate(System.String)", (vm::InternalCallFunction)&Interop::kernel32_delete_file_private,
     kernel32_delete_file_private_invoker},
    {"Interop/Kernel32::FindNextFile(Microsoft.Win32.SafeHandles.SafeFindHandle,Interop/Kernel32/WIN32_FIND_DATA&)",
     (vm::InternalCallFunction)&Interop::kernel32_find_next_file, kernel32_find_next_file_invoker},
    // {"Interop/Kernel32::FormatMessage(System.Int32,System.IntPtr,System.UInt32,System.Int32,System.Char*,System.Int32,System.IntPtr[])",
    //  (vm::InternalCallFunction)&Interop::kernel32_format_message, kernel32_format_message_invoker},
    {"Interop/Kernel32::GetFileInformationByHandleEx(System.IntPtr,Interop/Kernel32/FILE_INFO_BY_HANDLE_CLASS,System.IntPtr,System.UInt32)",
     (vm::InternalCallFunction)&Interop::kernel32_get_file_information_by_handle_ex, kernel32_get_file_information_by_handle_ex_invoker},
    {"Interop/Kernel32::GetLogicalDrives()", (vm::InternalCallFunction)&Interop::kernel32_get_logical_drives, kernel32_get_logical_drives_invoker},
    {"Interop/Kernel32::MoveFileExPrivate(System.String,System.String,System.UInt32)", (vm::InternalCallFunction)&Interop::kernel32_move_file_ex_private,
     kernel32_move_file_ex_private_invoker},
    {"Interop/Kernel32::RemoveDirectoryPrivate(System.String)", (vm::InternalCallFunction)&Interop::kernel32_remove_directory_private,
     kernel32_remove_directory_private_invoker},
    {"Interop/Kernel32::ReplaceFilePrivate(System.String,System.String,System.String,System.Int32,System.IntPtr,System.IntPtr)",
     (vm::InternalCallFunction)&Interop::kernel32_replace_file_private, kernel32_replace_file_private_invoker},
    {"Interop/Kernel32::SetFileAttributesPrivate(System.String,System.Int32)", (vm::InternalCallFunction)&Interop::kernel32_set_file_attributes_private,
     kernel32_set_file_attributes_private_invoker},
    {"Interop/Kernel32::SetFileInformationByHandle(Microsoft.Win32.SafeHandles.SafeFileHandle,Interop/Kernel32/FILE_INFO_BY_HANDLE_CLASS,Interop/"
     "Kernel32/FILE_BASIC_INFO&,System.UInt32)",
     (vm::InternalCallFunction)&Interop::kernel32_set_file_information_by_handle, kernel32_set_file_information_by_handle_invoker},
#endif
};

utils::Span<vm::InternalCallEntry> Interop::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_interop_internal_call_entries, sizeof(s_interop_internal_call_entries) / sizeof(vm::InternalCallEntry));
}
} // namespace icalls
} // namespace leanclr

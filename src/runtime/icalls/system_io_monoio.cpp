#include "system_io_monoio.h"

#include "platform/rt_file.h"
#include "platform/rt_path.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace icalls
{

namespace
{

constexpr int32_t kErrorSuccess = 0;
constexpr int32_t kErrorInvalidParameter = 87;

inline void set_error(int32_t* error, int32_t value) noexcept
{
    if (error)
        *error = value;
}

} // namespace

/// @icall: System.IO.MonoIO::get_AltDirectorySeparatorChar
RtResult<Utf16Char> SystemIOMonoIO::get_alt_directory_separator_char() noexcept
{
    RET_OK(os::Path::get_alt_directory_separator_char());
}

static RtResultVoid get_alt_directory_separator_char_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                             const interp::RtStackObject* /*params*/, interp::RtStackObject* ret) noexcept
{
    EvalStackOp::set_return(ret, static_cast<int32_t>(os::Path::get_alt_directory_separator_char()));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::get_DirectorySeparatorChar
RtResult<Utf16Char> SystemIOMonoIO::get_directory_separator_char() noexcept
{
    RET_OK(os::Path::get_directory_separator_char());
}

static RtResultVoid get_directory_separator_char_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                         const interp::RtStackObject* /*params*/, interp::RtStackObject* ret) noexcept
{
    EvalStackOp::set_return(ret, static_cast<int32_t>(os::Path::get_directory_separator_char()));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::get_PathSeparator
RtResult<Utf16Char> SystemIOMonoIO::get_path_separator() noexcept
{
    RET_OK(os::Path::get_path_separator());
}

static RtResultVoid get_path_separator_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                               interp::RtStackObject* ret) noexcept
{
    EvalStackOp::set_return(ret, static_cast<int32_t>(os::Path::get_path_separator()));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::get_VolumeSeparatorChar
RtResult<Utf16Char> SystemIOMonoIO::get_volume_separator_char() noexcept
{
    RET_OK(os::Path::get_volume_separator_char());
}

static RtResultVoid get_volume_separator_char_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                                      interp::RtStackObject* ret) noexcept
{
    EvalStackOp::set_return(ret, static_cast<int32_t>(os::Path::get_volume_separator_char()));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::get_ConsoleError
RtResult<intptr_t> SystemIOMonoIO::get_console_error() noexcept
{
    RET_OK(os::File::get_stderr());
}

static RtResultVoid get_console_error_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                              interp::RtStackObject* ret) noexcept
{
    EvalStackOp::set_return(ret, os::File::get_stderr());
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::get_ConsoleInput
RtResult<intptr_t> SystemIOMonoIO::get_console_input() noexcept
{
    RET_OK(os::File::get_stdin());
}

static RtResultVoid get_console_input_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                              interp::RtStackObject* ret) noexcept
{
    EvalStackOp::set_return(ret, os::File::get_stdin());
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::get_ConsoleOutput
RtResult<intptr_t> SystemIOMonoIO::get_console_output() noexcept
{
    RET_OK(os::File::get_stdout());
}

static RtResultVoid get_console_output_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                               interp::RtStackObject* ret) noexcept
{
    EvalStackOp::set_return(ret, os::File::get_stdout());
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::Close
RtResult<bool> SystemIOMonoIO::close(intptr_t handle, int32_t* error) noexcept
{
    RET_OK(os::File::close(handle, error));
}

static RtResultVoid close_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 1);
    EvalStackOp::set_return(ret, static_cast<int32_t>(os::File::close(handle, error)));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::DumpHandles
RtResultVoid SystemIOMonoIO::dump_handles() noexcept
{
    // No-op: handle tracking is not implemented.
    RET_VOID_OK();
}

static RtResultVoid dump_handles_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                         interp::RtStackObject* /*ret*/) noexcept
{
    RET_ERR_ON_FAIL(SystemIOMonoIO::dump_handles());
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::FindCloseFile
RtResult<bool> SystemIOMonoIO::find_close_file(intptr_t /*handle*/) noexcept
{
    // FindFirstFile / FindNextFile aren't implemented; return success to satisfy callers.
    RET_OK(true);
}

static RtResultVoid find_close_file_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                            interp::RtStackObject* ret) noexcept
{
    EvalStackOp::set_return(ret, static_cast<int32_t>(true));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::GetCurrentDirectory
RtResult<vm::RtString*> SystemIOMonoIO::get_current_directory(int32_t* error) noexcept
{
    RET_OK(os::Path::get_current_directory(error));
}

static RtResultVoid get_current_directory_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                  interp::RtStackObject* ret) noexcept
{
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 0);
    EvalStackOp::set_return(ret, os::Path::get_current_directory(error));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::GetFileType
RtResult<int32_t> SystemIOMonoIO::get_file_type(intptr_t handle, int32_t* error) noexcept
{
    RET_OK(os::File::get_file_type(handle, error));
}

static RtResultVoid get_file_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 1);
    EvalStackOp::set_return(ret, os::File::get_file_type(handle, error));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::GetLength
RtResult<int64_t> SystemIOMonoIO::get_length(intptr_t handle, int32_t* error) noexcept
{
    RET_OK(os::File::get_length(handle, error));
}

static RtResultVoid get_length_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 1);
    EvalStackOp::set_return(ret, os::File::get_length(handle, error));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::Open
RtResult<intptr_t> SystemIOMonoIO::open(const Utf16Char* filename, int32_t mode, int32_t access, int32_t share, int32_t options, int32_t* error) noexcept
{
    RET_OK(os::File::open(filename, mode, access, share, options, error));
}

static RtResultVoid open_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    const Utf16Char* filename = EvalStackOp::get_param<const Utf16Char*>(params, 0);
    int32_t mode = EvalStackOp::get_param<int32_t>(params, 1);
    int32_t access = EvalStackOp::get_param<int32_t>(params, 2);
    int32_t share = EvalStackOp::get_param<int32_t>(params, 3);
    int32_t options = EvalStackOp::get_param<int32_t>(params, 4);
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 5);
    EvalStackOp::set_return(ret, os::File::open(filename, mode, access, share, options, error));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::Read
RtResult<int32_t> SystemIOMonoIO::read(intptr_t handle, vm::RtArray* dest, int32_t dest_offset, int32_t count, int32_t* error) noexcept
{
    set_error(error, kErrorSuccess);

    if (dest == nullptr)
        RET_ERR(RtErr::ArgumentNull);
    if (dest_offset < 0 || count < 0)
    {
        set_error(error, kErrorInvalidParameter);
        RET_OK(static_cast<int32_t>(0));
    }

    uint32_t dest_len = static_cast<uint32_t>(vm::Array::get_array_length(dest));
    if (static_cast<uint64_t>(dest_offset) + static_cast<uint64_t>(count) > dest_len)
    {
        set_error(error, kErrorInvalidParameter);
        RET_OK(static_cast<int32_t>(0));
    }

    uint8_t* buffer = vm::Array::get_array_element_address<uint8_t>(dest, dest_offset);
    RET_OK(os::File::read(handle, buffer, count, error));
}

static RtResultVoid monoio_read_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    auto dest = EvalStackOp::get_param<vm::RtArray*>(params, 1);
    int32_t dest_offset = EvalStackOp::get_param<int32_t>(params, 2);
    int32_t count = EvalStackOp::get_param<int32_t>(params, 3);
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, n, SystemIOMonoIO::read(handle, dest, dest_offset, count, error));
    EvalStackOp::set_return(ret, n);
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::RemapPath
RtResult<bool> SystemIOMonoIO::remap_path(vm::RtString* /*path*/, vm::RtString** new_path) noexcept
{
    if (new_path)
        *new_path = nullptr;
    RET_OK(false);
}

static RtResultVoid remap_path_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto new_path = EvalStackOp::get_param<vm::RtString**>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemIOMonoIO::remap_path(path, new_path));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::Seek
RtResult<int64_t> SystemIOMonoIO::seek(intptr_t handle, int64_t offset, int32_t origin, int32_t* error) noexcept
{
    RET_OK(os::File::seek(handle, offset, origin, error));
}

static RtResultVoid seek_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    int64_t offset = EvalStackOp::get_param<int64_t>(params, 1);
    int32_t origin = EvalStackOp::get_param<int32_t>(params, 2);
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 3);
    EvalStackOp::set_return(ret, os::File::seek(handle, offset, origin, error));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::Write
RtResult<int32_t> SystemIOMonoIO::write(intptr_t handle, vm::RtArray* src, int32_t src_offset, int32_t count, int32_t* error) noexcept
{
    set_error(error, kErrorSuccess);

    if (src == nullptr)
        RET_ERR(RtErr::ArgumentNull);
    if (src_offset < 0 || count < 0)
    {
        set_error(error, kErrorInvalidParameter);
        RET_OK(static_cast<int32_t>(0));
    }

    uint32_t src_len = static_cast<uint32_t>(vm::Array::get_array_length(src));
    if (static_cast<uint64_t>(src_offset) + static_cast<uint64_t>(count) > src_len)
    {
        set_error(error, kErrorInvalidParameter);
        RET_OK(static_cast<int32_t>(0));
    }

    const uint8_t* buffer = vm::Array::get_array_element_address<uint8_t>(src, src_offset);
    RET_OK(os::File::write(handle, buffer, count, error));
}

static RtResultVoid write_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    auto src = EvalStackOp::get_param<vm::RtArray*>(params, 1);
    int32_t src_offset = EvalStackOp::get_param<int32_t>(params, 2);
    int32_t count = EvalStackOp::get_param<int32_t>(params, 3);
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, n, SystemIOMonoIO::write(handle, src, src_offset, count, error));
    EvalStackOp::set_return(ret, n);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_io_monoio[] = {
    {"System.IO.MonoIO::get_AltDirectorySeparatorChar", (vm::InternalCallFunction)&SystemIOMonoIO::get_alt_directory_separator_char,
     get_alt_directory_separator_char_invoker},
    {"System.IO.MonoIO::get_DirectorySeparatorChar", (vm::InternalCallFunction)&SystemIOMonoIO::get_directory_separator_char,
     get_directory_separator_char_invoker},
    {"System.IO.MonoIO::get_PathSeparator", (vm::InternalCallFunction)&SystemIOMonoIO::get_path_separator, get_path_separator_invoker},
    {"System.IO.MonoIO::get_VolumeSeparatorChar", (vm::InternalCallFunction)&SystemIOMonoIO::get_volume_separator_char, get_volume_separator_char_invoker},
    {"System.IO.MonoIO::get_ConsoleError", (vm::InternalCallFunction)&SystemIOMonoIO::get_console_error, get_console_error_invoker},
    {"System.IO.MonoIO::get_ConsoleInput", (vm::InternalCallFunction)&SystemIOMonoIO::get_console_input, get_console_input_invoker},
    {"System.IO.MonoIO::get_ConsoleOutput", (vm::InternalCallFunction)&SystemIOMonoIO::get_console_output, get_console_output_invoker},
    {"System.IO.MonoIO::Close(System.IntPtr,System.IO.MonoIOError&)", (vm::InternalCallFunction)&SystemIOMonoIO::close, close_invoker},
    {"System.IO.MonoIO::DumpHandles", (vm::InternalCallFunction)&SystemIOMonoIO::dump_handles, dump_handles_invoker},
    {"System.IO.MonoIO::FindCloseFile(System.IntPtr)", (vm::InternalCallFunction)&SystemIOMonoIO::find_close_file, find_close_file_invoker},
    {"System.IO.MonoIO::GetCurrentDirectory(System.IO.MonoIOError&)", (vm::InternalCallFunction)&SystemIOMonoIO::get_current_directory,
     get_current_directory_invoker},
    {"System.IO.MonoIO::GetFileType(System.IntPtr,System.IO.MonoIOError&)", (vm::InternalCallFunction)&SystemIOMonoIO::get_file_type, get_file_type_invoker},
    {"System.IO.MonoIO::GetLength(System.IntPtr,System.IO.MonoIOError&)", (vm::InternalCallFunction)&SystemIOMonoIO::get_length, get_length_invoker},
    {"System.IO.MonoIO::Open(System.Char*,System.IO.FileMode,System.IO.FileAccess,System.IO.FileShare,System.IO.FileOptions,System.IO.MonoIOError&)",
     (vm::InternalCallFunction)&SystemIOMonoIO::open, open_invoker},
    {"System.IO.MonoIO::Read(System.IntPtr,System.Byte[],System.Int32,System.Int32,System.IO.MonoIOError&)", (vm::InternalCallFunction)&SystemIOMonoIO::read,
     monoio_read_invoker},
    {"System.IO.MonoIO::RemapPath(System.String,System.String&)", (vm::InternalCallFunction)&SystemIOMonoIO::remap_path, remap_path_invoker},
    {"System.IO.MonoIO::Seek(System.IntPtr,System.Int64,System.IO.SeekOrigin,System.IO.MonoIOError&)", (vm::InternalCallFunction)&SystemIOMonoIO::seek,
     seek_invoker},
    {"System.IO.MonoIO::Write(System.IntPtr,System.Byte[],System.Int32,System.Int32,System.IO.MonoIOError&)", (vm::InternalCallFunction)&SystemIOMonoIO::write,
     write_invoker},
};

utils::Span<vm::InternalCallEntry> SystemIOMonoIO::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_io_monoio,
                                              sizeof(s_internal_call_entries_system_io_monoio) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

#include "system_io_monoio.h"

#include "vm/rt_string.h"
#include "vm/rt_array.h"
#include "utils/string_builder.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef LEANCLR_PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace leanclr
{
namespace icalls
{

namespace
{

// MonoIOError values (mirror System.IO.MonoIOError)
constexpr int32_t kErrorSuccess = 0;
constexpr int32_t kErrorFileNotFound = 2;
constexpr int32_t kErrorPathNotFound = 3;
constexpr int32_t kErrorAccessDenied = 5;
constexpr int32_t kErrorInvalidHandle = 6;
constexpr int32_t kErrorHandleDiskFull = 39;
constexpr int32_t kErrorFileExists = 80;
constexpr int32_t kErrorInvalidParameter = 87;
constexpr int32_t kErrorDirectory = 267;
constexpr int32_t kErrorGenFailure = 31;

// MonoFileType values (mirror System.IO.MonoFileType, which matches Win32 FILE_TYPE_*)
constexpr int32_t kMonoFileTypeUnknown = 0;
constexpr int32_t kMonoFileTypeDisk = 1;
constexpr int32_t kMonoFileTypeChar = 2;
constexpr int32_t kMonoFileTypePipe = 3;

// System.IO.FileMode values
constexpr int32_t kFileModeCreateNew = 1;
constexpr int32_t kFileModeCreate = 2;
constexpr int32_t kFileModeOpen = 3;
constexpr int32_t kFileModeOpenOrCreate = 4;
constexpr int32_t kFileModeTruncate = 5;
constexpr int32_t kFileModeAppend = 6;

// System.IO.FileAccess values
constexpr int32_t kFileAccessRead = 1;
constexpr int32_t kFileAccessWrite = 2;
constexpr int32_t kFileAccessReadWrite = 3;

constexpr intptr_t kInvalidHandle = static_cast<intptr_t>(-1);

#ifdef LEANCLR_PLATFORM_WIN

static int32_t win32_error_to_monoio(DWORD code)
{
    // The System.IO.MonoIOError enum values map directly to Win32 error codes.
    return static_cast<int32_t>(code);
}

#else

static int32_t errno_to_monoio(int err)
{
    switch (err)
    {
    case 0:
        return kErrorSuccess;
    case ENOENT:
        return kErrorFileNotFound;
    case ENOTDIR:
        return kErrorPathNotFound;
    case EACCES:
    case EPERM:
        return kErrorAccessDenied;
    case EBADF:
        return kErrorInvalidHandle;
    case EEXIST:
        return kErrorFileExists;
    case EINVAL:
        return kErrorInvalidParameter;
    case ENOSPC:
        return kErrorHandleDiskFull;
    case EISDIR:
        return kErrorDirectory;
    default:
        return kErrorGenFailure;
    }
}

#endif

static void set_error(int32_t* error, int32_t value)
{
    if (error)
        *error = value;
}

} // namespace

/// @icall: System.IO.MonoIO::get_AltDirectorySeparatorChar
RtResult<Utf16Char> SystemIOMonoIO::get_alt_directory_separator_char()
{
    RET_OK(static_cast<Utf16Char>('/'));
}

static RtResultVoid get_alt_directory_separator_char_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                             const interp::RtStackObject* /*params*/, interp::RtStackObject* ret) noexcept
{
    EvalStackOp::set_return(ret, static_cast<int32_t>('/'));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::get_DirectorySeparatorChar
RtResult<Utf16Char> SystemIOMonoIO::get_directory_separator_char()
{
#ifdef LEANCLR_PLATFORM_WIN
    RET_OK(static_cast<Utf16Char>('\\'));
#else
    RET_OK(static_cast<Utf16Char>('/'));
#endif
}

static RtResultVoid get_directory_separator_char_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                                         interp::RtStackObject* ret) noexcept
{
#ifdef LEANCLR_PLATFORM_WIN
    EvalStackOp::set_return(ret, static_cast<int32_t>('\\'));
#else
    EvalStackOp::set_return(ret, static_cast<int32_t>('/'));
#endif
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::get_PathSeparator
RtResult<Utf16Char> SystemIOMonoIO::get_path_separator()
{
#ifdef LEANCLR_PLATFORM_WIN
    RET_OK(static_cast<Utf16Char>(';'));
#else
    RET_OK(static_cast<Utf16Char>(':'));
#endif
}

static RtResultVoid get_path_separator_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                               interp::RtStackObject* ret) noexcept
{
#ifdef LEANCLR_PLATFORM_WIN
    EvalStackOp::set_return(ret, static_cast<int32_t>(';'));
#else
    EvalStackOp::set_return(ret, static_cast<int32_t>(':'));
#endif
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::get_VolumeSeparatorChar
RtResult<Utf16Char> SystemIOMonoIO::get_volume_separator_char()
{
#ifdef LEANCLR_PLATFORM_WIN
    RET_OK(static_cast<Utf16Char>(':'));
#else
    RET_OK(static_cast<Utf16Char>('/'));
#endif
}

static RtResultVoid get_volume_separator_char_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                                      interp::RtStackObject* ret) noexcept
{
#ifdef LEANCLR_PLATFORM_WIN
    EvalStackOp::set_return(ret, static_cast<int32_t>(':'));
#else
    EvalStackOp::set_return(ret, static_cast<int32_t>('/'));
#endif
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::get_ConsoleError
RtResult<intptr_t> SystemIOMonoIO::get_console_error()
{
#ifdef LEANCLR_PLATFORM_WIN
    RET_OK(reinterpret_cast<intptr_t>(::GetStdHandle(STD_ERROR_HANDLE)));
#else
    RET_OK(static_cast<intptr_t>(STDERR_FILENO));
#endif
}

static RtResultVoid get_console_error_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                              interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, handle, SystemIOMonoIO::get_console_error());
    EvalStackOp::set_return(ret, handle);
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::get_ConsoleInput
RtResult<intptr_t> SystemIOMonoIO::get_console_input()
{
#ifdef LEANCLR_PLATFORM_WIN
    RET_OK(reinterpret_cast<intptr_t>(::GetStdHandle(STD_INPUT_HANDLE)));
#else
    RET_OK(static_cast<intptr_t>(STDIN_FILENO));
#endif
}

static RtResultVoid get_console_input_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                              interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, handle, SystemIOMonoIO::get_console_input());
    EvalStackOp::set_return(ret, handle);
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::get_ConsoleOutput
RtResult<intptr_t> SystemIOMonoIO::get_console_output()
{
#ifdef LEANCLR_PLATFORM_WIN
    RET_OK(reinterpret_cast<intptr_t>(::GetStdHandle(STD_OUTPUT_HANDLE)));
#else
    RET_OK(static_cast<intptr_t>(STDOUT_FILENO));
#endif
}

static RtResultVoid get_console_output_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                               interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, handle, SystemIOMonoIO::get_console_output());
    EvalStackOp::set_return(ret, handle);
    RET_VOID_OK();
}

static bool is_standard_handle(intptr_t handle)
{
#ifdef LEANCLR_PLATFORM_WIN
    HANDLE h = reinterpret_cast<HANDLE>(handle);
    return h == ::GetStdHandle(STD_INPUT_HANDLE) || h == ::GetStdHandle(STD_OUTPUT_HANDLE) || h == ::GetStdHandle(STD_ERROR_HANDLE);
#else
    return handle == STDIN_FILENO || handle == STDOUT_FILENO || handle == STDERR_FILENO;
#endif
}

/// @icall: System.IO.MonoIO::Close
RtResult<bool> SystemIOMonoIO::close(intptr_t handle, int32_t* error)
{
    set_error(error, kErrorSuccess);

    if (handle == 0 || handle == kInvalidHandle)
    {
        set_error(error, kErrorInvalidHandle);
        RET_OK(false);
    }

    if (is_standard_handle(handle))
        RET_OK(true);

#ifdef LEANCLR_PLATFORM_WIN
    if (!::CloseHandle(reinterpret_cast<HANDLE>(handle)))
    {
        set_error(error, win32_error_to_monoio(::GetLastError()));
        RET_OK(false);
    }
    RET_OK(true);
#else
    if (::close(static_cast<int>(handle)) != 0)
    {
        set_error(error, errno_to_monoio(errno));
        RET_OK(false);
    }
    RET_OK(true);
#endif
}

static RtResultVoid close_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemIOMonoIO::close(handle, error));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::DumpHandles
RtResultVoid SystemIOMonoIO::dump_handles()
{
    // No-op: handle tracking is not implemented.
    RET_VOID_OK();
}

static RtResultVoid dump_handles_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                         interp::RtStackObject* /*ret*/)
{
    RET_ERR_ON_FAIL(SystemIOMonoIO::dump_handles());
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::FindCloseFile
RtResult<bool> SystemIOMonoIO::find_close_file(intptr_t /*handle*/)
{
    // FindFirstFile / FindNextFile aren't implemented; return success to satisfy callers.
    RET_OK(true);
}

static RtResultVoid find_close_file_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                            interp::RtStackObject* ret) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemIOMonoIO::find_close_file(handle));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::GetCurrentDirectory
RtResult<vm::RtString*> SystemIOMonoIO::get_current_directory(int32_t* error)
{
    set_error(error, kErrorSuccess);

#ifdef LEANCLR_PLATFORM_WIN
    DWORD needed = ::GetCurrentDirectoryW(0, nullptr);
    if (needed == 0)
    {
        set_error(error, win32_error_to_monoio(::GetLastError()));
        RET_OK(vm::String::get_empty_string());
    }
    wchar_t stack_buf[MAX_PATH + 1];
    wchar_t* buffer = stack_buf;
    wchar_t* heap_buf = nullptr;
    if (needed > sizeof(stack_buf) / sizeof(wchar_t))
    {
        heap_buf = static_cast<wchar_t*>(std::malloc(needed * sizeof(wchar_t)));
        if (!heap_buf)
        {
            set_error(error, kErrorGenFailure);
            RET_OK(vm::String::get_empty_string());
        }
        buffer = heap_buf;
    }
    DWORD written = ::GetCurrentDirectoryW(needed, buffer);
    vm::RtString* result;
    if (written == 0)
    {
        set_error(error, win32_error_to_monoio(::GetLastError()));
        result = vm::String::get_empty_string();
    }
    else
    {
        static_assert(sizeof(wchar_t) == sizeof(Utf16Char), "wchar_t must be 16-bit on Windows");
        result = vm::String::create_string_from_utf16chars(reinterpret_cast<const uint16_t*>(buffer), static_cast<int32_t>(written));
    }
    if (heap_buf)
        std::free(heap_buf);
    RET_OK(result);
#else
    char stack_buf[1024];
    if (::getcwd(stack_buf, sizeof(stack_buf)) != nullptr)
    {
        RET_OK(vm::String::create_string_from_utf8cstr(stack_buf));
    }
    set_error(error, errno_to_monoio(errno));
    RET_OK(vm::String::get_empty_string());
#endif
}

static RtResultVoid get_current_directory_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                  interp::RtStackObject* ret) noexcept
{
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, s, SystemIOMonoIO::get_current_directory(error));
    EvalStackOp::set_return(ret, s);
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::GetFileType
RtResult<int32_t> SystemIOMonoIO::get_file_type(intptr_t handle, int32_t* error)
{
    set_error(error, kErrorSuccess);

    if (handle == 0 || handle == kInvalidHandle)
    {
        set_error(error, kErrorInvalidHandle);
        RET_OK(kMonoFileTypeUnknown);
    }

#ifdef LEANCLR_PLATFORM_WIN
    DWORD file_type = ::GetFileType(reinterpret_cast<HANDLE>(handle));
    if (file_type == FILE_TYPE_UNKNOWN)
    {
        DWORD err = ::GetLastError();
        if (err != NO_ERROR)
            set_error(error, win32_error_to_monoio(err));
    }
    // Win32 FILE_TYPE_* values match MonoFileType enum values.
    RET_OK(static_cast<int32_t>(file_type));
#else
    struct stat st;
    if (::fstat(static_cast<int>(handle), &st) != 0)
    {
        set_error(error, errno_to_monoio(errno));
        RET_OK(kMonoFileTypeUnknown);
    }
    if (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))
        RET_OK(kMonoFileTypeDisk);
    if (S_ISCHR(st.st_mode))
        RET_OK(kMonoFileTypeChar);
    if (S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode))
        RET_OK(kMonoFileTypePipe);
    RET_OK(kMonoFileTypeUnknown);
#endif
}

static RtResultVoid get_file_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, t, SystemIOMonoIO::get_file_type(handle, error));
    EvalStackOp::set_return(ret, t);
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::GetLength
RtResult<int64_t> SystemIOMonoIO::get_length(intptr_t handle, int32_t* error)
{
    set_error(error, kErrorSuccess);

    if (handle == 0 || handle == kInvalidHandle)
    {
        set_error(error, kErrorInvalidHandle);
        RET_OK(static_cast<int64_t>(0));
    }

#ifdef LEANCLR_PLATFORM_WIN
    LARGE_INTEGER size;
    if (!::GetFileSizeEx(reinterpret_cast<HANDLE>(handle), &size))
    {
        set_error(error, win32_error_to_monoio(::GetLastError()));
        RET_OK(static_cast<int64_t>(0));
    }
    RET_OK(static_cast<int64_t>(size.QuadPart));
#else
    struct stat st;
    if (::fstat(static_cast<int>(handle), &st) != 0)
    {
        set_error(error, errno_to_monoio(errno));
        RET_OK(static_cast<int64_t>(0));
    }
    RET_OK(static_cast<int64_t>(st.st_size));
#endif
}

static RtResultVoid get_length_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, len, SystemIOMonoIO::get_length(handle, error));
    EvalStackOp::set_return(ret, len);
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::Open
RtResult<intptr_t> SystemIOMonoIO::open(const Utf16Char* filename, int32_t mode, int32_t access, int32_t share, int32_t options, int32_t* error)
{
    set_error(error, kErrorSuccess);

    if (filename == nullptr)
    {
        set_error(error, kErrorInvalidParameter);
        RET_OK(kInvalidHandle);
    }

#ifdef LEANCLR_PLATFORM_WIN
    DWORD desired_access = 0;
    switch (access)
    {
    case kFileAccessRead:
        desired_access = GENERIC_READ;
        break;
    case kFileAccessWrite:
        desired_access = GENERIC_WRITE;
        break;
    case kFileAccessReadWrite:
        desired_access = GENERIC_READ | GENERIC_WRITE;
        break;
    default:
        set_error(error, kErrorInvalidParameter);
        RET_OK(kInvalidHandle);
    }

    DWORD share_mode = 0;
    if (share & 0x1)
        share_mode |= FILE_SHARE_READ;
    if (share & 0x2)
        share_mode |= FILE_SHARE_WRITE;
    if (share & 0x4)
        share_mode |= FILE_SHARE_DELETE;

    DWORD creation_disposition = 0;
    switch (mode)
    {
    case kFileModeCreateNew:
        creation_disposition = CREATE_NEW;
        break;
    case kFileModeCreate:
        creation_disposition = CREATE_ALWAYS;
        break;
    case kFileModeOpen:
        creation_disposition = OPEN_EXISTING;
        break;
    case kFileModeOpenOrCreate:
        creation_disposition = OPEN_ALWAYS;
        break;
    case kFileModeTruncate:
        creation_disposition = TRUNCATE_EXISTING;
        break;
    case kFileModeAppend:
        creation_disposition = OPEN_ALWAYS;
        break;
    default:
        set_error(error, kErrorInvalidParameter);
        RET_OK(kInvalidHandle);
    }

    DWORD flags_and_attributes = FILE_ATTRIBUTE_NORMAL;
    // Map common FileOptions flags to Win32 flags
    if (static_cast<uint32_t>(options) & 0x80000000u)
        flags_and_attributes |= FILE_FLAG_WRITE_THROUGH;
    if (static_cast<uint32_t>(options) & 0x40000000u)
        flags_and_attributes |= FILE_FLAG_OVERLAPPED;
    if (static_cast<uint32_t>(options) & 0x20000000u)
        flags_and_attributes |= FILE_FLAG_NO_BUFFERING;
    if (static_cast<uint32_t>(options) & 0x10000000u)
        flags_and_attributes |= FILE_FLAG_RANDOM_ACCESS;
    if (static_cast<uint32_t>(options) & 0x08000000u)
        flags_and_attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
    if (static_cast<uint32_t>(options) & 0x04000000u)
        flags_and_attributes |= FILE_FLAG_DELETE_ON_CLOSE;

    static_assert(sizeof(wchar_t) == sizeof(Utf16Char), "wchar_t must be 16-bit on Windows");
    HANDLE h = ::CreateFileW(reinterpret_cast<LPCWSTR>(filename), desired_access, share_mode, nullptr, creation_disposition, flags_and_attributes, nullptr);
    if (h == INVALID_HANDLE_VALUE)
    {
        set_error(error, win32_error_to_monoio(::GetLastError()));
        RET_OK(kInvalidHandle);
    }

    if (mode == kFileModeAppend)
    {
        LARGE_INTEGER zero{};
        ::SetFilePointerEx(h, zero, nullptr, FILE_END);
    }

    RET_OK(reinterpret_cast<intptr_t>(h));
#else
    // Convert UTF-16 filename to UTF-8 for POSIX open
    int32_t u16_len = 0;
    while (filename[u16_len] != 0)
        ++u16_len;
    utils::StringBuilder sb;
    utils::StringUtil::utf16_to_utf8(filename, static_cast<size_t>(u16_len), sb);
    sb.sure_null_terminator_but_not_append();

    int flags = 0;
    switch (access)
    {
    case kFileAccessRead:
        flags = O_RDONLY;
        break;
    case kFileAccessWrite:
        flags = O_WRONLY;
        break;
    case kFileAccessReadWrite:
        flags = O_RDWR;
        break;
    default:
        set_error(error, kErrorInvalidParameter);
        RET_OK(kInvalidHandle);
    }

    switch (mode)
    {
    case kFileModeCreateNew:
        flags |= O_CREAT | O_EXCL;
        break;
    case kFileModeCreate:
        flags |= O_CREAT | O_TRUNC;
        break;
    case kFileModeOpen:
        // no extra flags
        break;
    case kFileModeOpenOrCreate:
        flags |= O_CREAT;
        break;
    case kFileModeTruncate:
        flags |= O_TRUNC;
        break;
    case kFileModeAppend:
        flags |= O_CREAT | O_APPEND;
        break;
    default:
        set_error(error, kErrorInvalidParameter);
        RET_OK(kInvalidHandle);
    }

    (void)share;
    (void)options;

    int fd = ::open(sb.as_cstr(), flags, 0644);
    if (fd < 0)
    {
        set_error(error, errno_to_monoio(errno));
        RET_OK(kInvalidHandle);
    }
    RET_OK(static_cast<intptr_t>(fd));
#endif
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
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, handle, SystemIOMonoIO::open(filename, mode, access, share, options, error));
    EvalStackOp::set_return(ret, handle);
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::Read
RtResult<int32_t> SystemIOMonoIO::read(intptr_t handle, vm::RtArray* dest, int32_t dest_offset, int32_t count, int32_t* error)
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

    if (handle == 0 || handle == kInvalidHandle)
    {
        set_error(error, kErrorInvalidHandle);
        RET_OK(static_cast<int32_t>(-1));
    }

    uint8_t* buffer = vm::Array::get_array_element_address<uint8_t>(dest, dest_offset);

#ifdef LEANCLR_PLATFORM_WIN
    DWORD bytes_read = 0;
    BOOL ok = ::ReadFile(reinterpret_cast<HANDLE>(handle), buffer, static_cast<DWORD>(count), &bytes_read, nullptr);
    if (!ok)
    {
        DWORD err = ::GetLastError();
        if (err == ERROR_HANDLE_EOF || err == ERROR_BROKEN_PIPE)
        {
            RET_OK(static_cast<int32_t>(0));
        }
        set_error(error, win32_error_to_monoio(err));
        RET_OK(static_cast<int32_t>(-1));
    }
    RET_OK(static_cast<int32_t>(bytes_read));
#else
    ssize_t n;
    do
    {
        n = ::read(static_cast<int>(handle), buffer, static_cast<size_t>(count));
    } while (n < 0 && errno == EINTR);
    if (n < 0)
    {
        set_error(error, errno_to_monoio(errno));
        RET_OK(static_cast<int32_t>(-1));
    }
    RET_OK(static_cast<int32_t>(n));
#endif
}

static RtResultVoid read_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
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
RtResult<bool> SystemIOMonoIO::remap_path(vm::RtString* /*path*/, vm::RtString** new_path)
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
RtResult<int64_t> SystemIOMonoIO::seek(intptr_t handle, int64_t offset, int32_t origin, int32_t* error)
{
    set_error(error, kErrorSuccess);

    if (handle == 0 || handle == kInvalidHandle)
    {
        set_error(error, kErrorInvalidHandle);
        RET_OK(static_cast<int64_t>(-1));
    }

#ifdef LEANCLR_PLATFORM_WIN
    DWORD move_method;
    switch (origin)
    {
    case 0:
        move_method = FILE_BEGIN;
        break;
    case 1:
        move_method = FILE_CURRENT;
        break;
    case 2:
        move_method = FILE_END;
        break;
    default:
        set_error(error, kErrorInvalidParameter);
        RET_OK(static_cast<int64_t>(-1));
    }
    LARGE_INTEGER dist;
    dist.QuadPart = offset;
    LARGE_INTEGER result;
    if (!::SetFilePointerEx(reinterpret_cast<HANDLE>(handle), dist, &result, move_method))
    {
        set_error(error, win32_error_to_monoio(::GetLastError()));
        RET_OK(static_cast<int64_t>(-1));
    }
    RET_OK(static_cast<int64_t>(result.QuadPart));
#else
    int whence;
    switch (origin)
    {
    case 0:
        whence = SEEK_SET;
        break;
    case 1:
        whence = SEEK_CUR;
        break;
    case 2:
        whence = SEEK_END;
        break;
    default:
        set_error(error, kErrorInvalidParameter);
        RET_OK(static_cast<int64_t>(-1));
    }
    off_t pos = ::lseek(static_cast<int>(handle), static_cast<off_t>(offset), whence);
    if (pos == static_cast<off_t>(-1))
    {
        set_error(error, errno_to_monoio(errno));
        RET_OK(static_cast<int64_t>(-1));
    }
    RET_OK(static_cast<int64_t>(pos));
#endif
}

static RtResultVoid seek_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    int64_t offset = EvalStackOp::get_param<int64_t>(params, 1);
    int32_t origin = EvalStackOp::get_param<int32_t>(params, 2);
    int32_t* error = EvalStackOp::get_param<int32_t*>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, pos, SystemIOMonoIO::seek(handle, offset, origin, error));
    EvalStackOp::set_return(ret, pos);
    RET_VOID_OK();
}

/// @icall: System.IO.MonoIO::Write
RtResult<int32_t> SystemIOMonoIO::write(intptr_t handle, vm::RtArray* src, int32_t src_offset, int32_t count, int32_t* error)
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

    if (handle == 0 || handle == kInvalidHandle)
    {
        set_error(error, kErrorInvalidHandle);
        RET_OK(static_cast<int32_t>(-1));
    }

    const uint8_t* buffer = vm::Array::get_array_element_address<uint8_t>(src, src_offset);

#ifdef LEANCLR_PLATFORM_WIN
    DWORD bytes_written = 0;
    BOOL ok = ::WriteFile(reinterpret_cast<HANDLE>(handle), buffer, static_cast<DWORD>(count), &bytes_written, nullptr);
    if (!ok)
    {
        set_error(error, win32_error_to_monoio(::GetLastError()));
        RET_OK(static_cast<int32_t>(-1));
    }
    RET_OK(static_cast<int32_t>(bytes_written));
#else
    ssize_t total = 0;
    while (total < count)
    {
        ssize_t n = ::write(static_cast<int>(handle), buffer + total, static_cast<size_t>(count - total));
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            set_error(error, errno_to_monoio(errno));
            RET_OK(total > 0 ? static_cast<int32_t>(total) : static_cast<int32_t>(-1));
        }
        if (n == 0)
            break;
        total += n;
    }
    RET_OK(static_cast<int32_t>(total));
#endif
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
     read_invoker},
    {"System.IO.MonoIO::RemapPath(System.String,System.String&)", (vm::InternalCallFunction)&SystemIOMonoIO::remap_path, remap_path_invoker},
    {"System.IO.MonoIO::Seek(System.IntPtr,System.Int64,System.IO.SeekOrigin,System.IO.MonoIOError&)", (vm::InternalCallFunction)&SystemIOMonoIO::seek,
     seek_invoker},
    {"System.IO.MonoIO::Write(System.IntPtr,System.Byte[],System.Int32,System.Int32,System.IO.MonoIOError&)", (vm::InternalCallFunction)&SystemIOMonoIO::write,
     write_invoker},
};

utils::Span<vm::InternalCallEntry> SystemIOMonoIO::get_internal_call_entries()
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_io_monoio,
                                              sizeof(s_internal_call_entries_system_io_monoio) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

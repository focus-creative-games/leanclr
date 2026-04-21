#include "rt_file.h"
#include "rt_io_error_internal.h"

#include <cstdint>

#ifdef LEANCLR_PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils/string_builder.h"
#include "utils/string_util.h"
#endif

namespace leanclr
{
namespace os
{

namespace
{

// Pull shared error constants and helpers into the unnamed namespace so
// existing call sites (set_error, kErrorXxx, ...) don't need any rewrites.
using io_error_internal::kErrorAccessDenied;
using io_error_internal::kErrorDirectory;
using io_error_internal::kErrorFileExists;
using io_error_internal::kErrorFileNotFound;
using io_error_internal::kErrorGenFailure;
using io_error_internal::kErrorHandleDiskFull;
using io_error_internal::kErrorInvalidHandle;
using io_error_internal::kErrorInvalidParameter;
using io_error_internal::kErrorPathNotFound;
using io_error_internal::kErrorSuccess;
using io_error_internal::set_error;
#ifdef LEANCLR_PLATFORM_WIN
using io_error_internal::win32_error_to_monoio;
#else
using io_error_internal::errno_to_monoio;
#endif

// System.IO.FileMode (file-local; does not conflict with other platform files).
constexpr int32_t kFileModeCreateNew = 1;
constexpr int32_t kFileModeCreate = 2;
constexpr int32_t kFileModeOpen = 3;
constexpr int32_t kFileModeOpenOrCreate = 4;
constexpr int32_t kFileModeTruncate = 5;
constexpr int32_t kFileModeAppend = 6;

// System.IO.FileAccess
constexpr int32_t kFileAccessRead = 1;
constexpr int32_t kFileAccessWrite = 2;
constexpr int32_t kFileAccessReadWrite = 3;

} // namespace

intptr_t File::get_stdin()
{
#ifdef LEANCLR_PLATFORM_WIN
    return reinterpret_cast<intptr_t>(::GetStdHandle(STD_INPUT_HANDLE));
#else
    return static_cast<intptr_t>(STDIN_FILENO);
#endif
}

intptr_t File::get_stdout()
{
#ifdef LEANCLR_PLATFORM_WIN
    return reinterpret_cast<intptr_t>(::GetStdHandle(STD_OUTPUT_HANDLE));
#else
    return static_cast<intptr_t>(STDOUT_FILENO);
#endif
}

intptr_t File::get_stderr()
{
#ifdef LEANCLR_PLATFORM_WIN
    return reinterpret_cast<intptr_t>(::GetStdHandle(STD_ERROR_HANDLE));
#else
    return static_cast<intptr_t>(STDERR_FILENO);
#endif
}

bool File::is_standard_handle(intptr_t handle)
{
#ifdef LEANCLR_PLATFORM_WIN
    HANDLE h = reinterpret_cast<HANDLE>(handle);
    return h == ::GetStdHandle(STD_INPUT_HANDLE) || h == ::GetStdHandle(STD_OUTPUT_HANDLE) || h == ::GetStdHandle(STD_ERROR_HANDLE);
#else
    return handle == STDIN_FILENO || handle == STDOUT_FILENO || handle == STDERR_FILENO;
#endif
}

intptr_t File::open(const Utf16Char* filename, int32_t mode, int32_t access, int32_t share, int32_t options, int32_t* error)
{
    set_error(error, kErrorSuccess);

    if (filename == nullptr)
    {
        set_error(error, kErrorInvalidParameter);
        return kInvalidHandle;
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
        return kInvalidHandle;
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
        return kInvalidHandle;
    }

    DWORD flags_and_attributes = FILE_ATTRIBUTE_NORMAL;
    const uint32_t opts = static_cast<uint32_t>(options);
    if (opts & 0x80000000u)
        flags_and_attributes |= FILE_FLAG_WRITE_THROUGH;
    if (opts & 0x40000000u)
        flags_and_attributes |= FILE_FLAG_OVERLAPPED;
    if (opts & 0x20000000u)
        flags_and_attributes |= FILE_FLAG_NO_BUFFERING;
    if (opts & 0x10000000u)
        flags_and_attributes |= FILE_FLAG_RANDOM_ACCESS;
    if (opts & 0x08000000u)
        flags_and_attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
    if (opts & 0x04000000u)
        flags_and_attributes |= FILE_FLAG_DELETE_ON_CLOSE;

    static_assert(sizeof(wchar_t) == sizeof(Utf16Char), "wchar_t must be 16-bit on Windows");
    HANDLE h = ::CreateFileW(reinterpret_cast<LPCWSTR>(filename), desired_access, share_mode, nullptr, creation_disposition, flags_and_attributes, nullptr);
    if (h == INVALID_HANDLE_VALUE)
    {
        set_error(error, win32_error_to_monoio(::GetLastError()));
        return kInvalidHandle;
    }

    if (mode == kFileModeAppend)
    {
        LARGE_INTEGER zero{};
        ::SetFilePointerEx(h, zero, nullptr, FILE_END);
    }
    return reinterpret_cast<intptr_t>(h);
#else
    // Convert UTF-16 filename to UTF-8 for the POSIX open() syscall.
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
        return kInvalidHandle;
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
        return kInvalidHandle;
    }

    (void)share;
    (void)options;

    int fd = ::open(sb.as_cstr(), flags, 0644);
    if (fd < 0)
    {
        set_error(error, errno_to_monoio(errno));
        return kInvalidHandle;
    }
    return static_cast<intptr_t>(fd);
#endif
}

bool File::close(intptr_t handle, int32_t* error)
{
    set_error(error, kErrorSuccess);

    // if (handle == 0 || handle == kInvalidHandle)
    // {
    //     set_error(error, kErrorInvalidHandle);
    //     return false;
    // }

    // Do not close standard streams; treat the operation as a no-op success.
    if (is_standard_handle(handle))
        return true;

#ifdef LEANCLR_PLATFORM_WIN
    if (!::CloseHandle(reinterpret_cast<HANDLE>(handle)))
    {
        set_error(error, win32_error_to_monoio(::GetLastError()));
        return false;
    }
    return true;
#else
    if (::close(static_cast<int>(handle)) != 0)
    {
        set_error(error, errno_to_monoio(errno));
        return false;
    }
    return true;
#endif
}

int32_t File::read(intptr_t handle, uint8_t* buffer, int32_t count, int32_t* error)
{
    set_error(error, kErrorSuccess);

    // if (handle == 0 || handle == kInvalidHandle)
    // {
    //     set_error(error, kErrorInvalidHandle);
    //     return -1;
    // }
    if (count < 0 || buffer == nullptr)
    {
        set_error(error, kErrorInvalidParameter);
        return -1;
    }

#ifdef LEANCLR_PLATFORM_WIN
    DWORD bytes_read = 0;
    BOOL ok = ::ReadFile(reinterpret_cast<HANDLE>(handle), buffer, static_cast<DWORD>(count), &bytes_read, nullptr);
    if (!ok)
    {
        DWORD err = ::GetLastError();
        // EOF on pipes / broken connection should surface as 0 bytes, not an error.
        if (err == ERROR_HANDLE_EOF || err == ERROR_BROKEN_PIPE)
            return 0;
        set_error(error, win32_error_to_monoio(err));
        return -1;
    }
    return static_cast<int32_t>(bytes_read);
#else
    ssize_t n;
    do
    {
        n = ::read(static_cast<int>(handle), buffer, static_cast<size_t>(count));
    } while (n < 0 && errno == EINTR);
    if (n < 0)
    {
        set_error(error, errno_to_monoio(errno));
        return -1;
    }
    return static_cast<int32_t>(n);
#endif
}

int32_t File::write(intptr_t handle, const uint8_t* buffer, int32_t count, int32_t* error)
{
    set_error(error, kErrorSuccess);

    // if (handle == 0 || handle == kInvalidHandle)
    // {
    //     set_error(error, kErrorInvalidHandle);
    //     return -1;
    // }
    if (count < 0 || buffer == nullptr)
    {
        set_error(error, kErrorInvalidParameter);
        return -1;
    }

#ifdef LEANCLR_PLATFORM_WIN
    DWORD bytes_written = 0;
    BOOL ok = ::WriteFile(reinterpret_cast<HANDLE>(handle), buffer, static_cast<DWORD>(count), &bytes_written, nullptr);
    if (!ok)
    {
        set_error(error, win32_error_to_monoio(::GetLastError()));
        return -1;
    }
    return static_cast<int32_t>(bytes_written);
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
            return total > 0 ? static_cast<int32_t>(total) : -1;
        }
        if (n == 0)
            break;
        total += n;
    }
    return static_cast<int32_t>(total);
#endif
}

int64_t File::seek(intptr_t handle, int64_t offset, int32_t origin, int32_t* error)
{
    set_error(error, kErrorSuccess);

    // if (handle == 0 || handle == kInvalidHandle)
    // {
    //     set_error(error, kErrorInvalidHandle);
    //     return -1;
    // }

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
        return -1;
    }
    LARGE_INTEGER dist;
    dist.QuadPart = offset;
    LARGE_INTEGER result;
    if (!::SetFilePointerEx(reinterpret_cast<HANDLE>(handle), dist, &result, move_method))
    {
        set_error(error, win32_error_to_monoio(::GetLastError()));
        return -1;
    }
    return static_cast<int64_t>(result.QuadPart);
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
        return -1;
    }
    off_t pos = ::lseek(static_cast<int>(handle), static_cast<off_t>(offset), whence);
    if (pos == static_cast<off_t>(-1))
    {
        set_error(error, errno_to_monoio(errno));
        return -1;
    }
    return static_cast<int64_t>(pos);
#endif
}

int64_t File::get_length(intptr_t handle, int32_t* error)
{
    set_error(error, kErrorSuccess);

    // if (handle == 0 || handle == kInvalidHandle)
    // {
    //     set_error(error, kErrorInvalidHandle);
    //     return 0;
    // }

#ifdef LEANCLR_PLATFORM_WIN
    LARGE_INTEGER size;
    if (!::GetFileSizeEx(reinterpret_cast<HANDLE>(handle), &size))
    {
        set_error(error, win32_error_to_monoio(::GetLastError()));
        return 0;
    }
    return static_cast<int64_t>(size.QuadPart);
#else
    struct stat st;
    if (::fstat(static_cast<int>(handle), &st) != 0)
    {
        set_error(error, errno_to_monoio(errno));
        return 0;
    }
    return static_cast<int64_t>(st.st_size);
#endif
}

int32_t File::get_file_type(intptr_t handle, int32_t* error)
{
    set_error(error, kErrorSuccess);

    // if (handle == 0 || handle == kInvalidHandle)
    // {
    //     set_error(error, kErrorInvalidHandle);
    //     return FileTypeUnknown;
    // }

#ifdef LEANCLR_PLATFORM_WIN
    DWORD file_type = ::GetFileType(reinterpret_cast<HANDLE>(handle));
    if (file_type == FILE_TYPE_UNKNOWN)
    {
        // DWORD err = ::GetLastError();
        // if (err != NO_ERROR)
        //     set_error(error, win32_error_to_monoio(err));
    }
    // Win32 FILE_TYPE_* values match MonoFileType enum values.
    return static_cast<int32_t>(file_type);
#else
    struct stat st;
    if (::fstat(static_cast<int>(handle), &st) != 0)
    {
        set_error(error, errno_to_monoio(errno));
        return FileTypeUnknown;
    }
    if (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))
        return FileTypeDisk;
    if (S_ISCHR(st.st_mode))
        return FileTypeChar;
    if (S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode))
        return FileTypePipe;
    return FileTypeUnknown;
#endif
}

} // namespace os
} // namespace leanclr

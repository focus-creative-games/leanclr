#include "kernel32.h"

#ifdef LEANCLR_PLATFORM_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "build_config.h"
#include "platform/rt_event.h"
#include "platform/rt_file.h"
#include "platform/rt_io_error_internal.h"
#include "platform/rt_path.h"
#include "platform/rt_sys.h"
#include "platform/rt_time.h"
#include "utils/rt_vector.h"
#include "utils/string_builder.h"
#include "utils/string_util.h"
#include "vm/rt_string.h"

#include <cstring>

#ifndef LEANCLR_PLATFORM_WIN
#include <errno.h>
#include <fcntl.h>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#endif

namespace leanclr
{
namespace platform
{

#ifndef LEANCLR_PLATFORM_WIN
namespace
{

constexpr intptr_t kInvalidHandle = static_cast<intptr_t>(-1);
constexpr int32_t kErrorInvalidHandle = 6;
constexpr int32_t kErrorInvalidParameter = 87;
constexpr int32_t kErrorCallNotImplemented = 120;
constexpr uint32_t kFileAttributeReadOnly = 0x00000001u;
constexpr uint32_t kFileAttributeHidden = 0x00000002u;
constexpr uint32_t kFileAttributeDirectory = 0x00000010u;
constexpr uint32_t kFileAttributeNormal = 0x00000080u;
constexpr uint32_t kGenericRead = 0x80000000u;
constexpr uint32_t kGenericWrite = 0x40000000u;
constexpr uint32_t kFileReadData = 0x00000001u;
constexpr uint32_t kFileWriteData = 0x00000002u;
constexpr uint32_t kFileAppendData = 0x00000004u;
constexpr uint32_t kCopyFileFailIfExists = 0x00000001u;
constexpr uint32_t kMoveFileReplaceExisting = 0x00000001u;
constexpr uint32_t kMoveFileCopyAllowed = 0x00000002u;
constexpr uint32_t kMoveFileDelayUntilReboot = 0x00000004u;
constexpr int32_t kFileBasicInfoClass = 0;
constexpr int32_t kFileStandardInfoClass = 1;
constexpr int32_t kFileDispositionInfoClass = 4;
constexpr int32_t kFileEndOfFileInfoClass = 6;
constexpr int32_t kFileAttributeTagInfoClass = 9;

struct Win32FileTime
{
    uint32_t LowDateTime;
    uint32_t HighDateTime;
};

struct Win32FileAttributeData
{
    uint32_t FileAttributes;
    Win32FileTime CreationTime;
    Win32FileTime LastAccessTime;
    Win32FileTime LastWriteTime;
    uint32_t FileSizeHigh;
    uint32_t FileSizeLow;
};

struct ByHandleFileInformation
{
    uint32_t FileAttributes;
    Win32FileTime CreationTime;
    Win32FileTime LastAccessTime;
    Win32FileTime LastWriteTime;
    uint32_t VolumeSerialNumber;
    uint32_t FileSizeHigh;
    uint32_t FileSizeLow;
    uint32_t NumberOfLinks;
    uint32_t FileIndexHigh;
    uint32_t FileIndexLow;
};

struct FileBasicInfo
{
    int64_t CreationTime;
    int64_t LastAccessTime;
    int64_t LastWriteTime;
    int64_t ChangeTime;
    uint32_t FileAttributes;
    uint32_t Reserved;
};

struct FileStandardInfo
{
    int64_t AllocationSize;
    int64_t EndOfFile;
    uint32_t NumberOfLinks;
    uint8_t DeletePending;
    uint8_t Directory;
    uint16_t Reserved;
};

struct FileAttributeTagInfo
{
    uint32_t FileAttributes;
    uint32_t ReparseTag;
};

struct FileDispositionInfo
{
    uint8_t DeleteFile;
};

struct FileEndOfFileInfo
{
    int64_t EndOfFile;
};

utils::Vector<intptr_t>& kernel32_event_handles()
{
    static utils::Vector<intptr_t> handles;
    return handles;
}

void register_kernel32_event_handle(intptr_t handle)
{
    if (handle != 0)
    {
        kernel32_event_handles().push_back(handle);
    }
}

bool close_registered_kernel32_event_handle(intptr_t handle)
{
    auto& handles = kernel32_event_handles();
    for (auto it = handles.begin(); it != handles.end(); ++it)
    {
        if (*it == handle)
        {
            handles.erase(it);
            delete reinterpret_cast<EventHandle*>(handle);
            return true;
        }
    }
    return false;
}

void set_last_error_from_errno(int err)
{
    RtSys::set_last_win32_error(os::io_error_internal::errno_to_monoio(err));
}

bool utf16_path_to_utf8(const Utf16Char* path, utils::Utf8StringBuilder& utf8)
{
    if (path == nullptr)
    {
        RtSys::set_last_win32_error(kErrorInvalidParameter);
        return false;
    }
    utf8.append_utf16_str(path, static_cast<size_t>(utils::StringUtil::get_utf16chars_length(path)));
    utf8.sure_null_terminator_but_not_append();
    return true;
}

bool rt_string_path_to_utf8(vm::RtString* path, utils::Utf8StringBuilder& utf8)
{
    return utf16_path_to_utf8(path != nullptr ? vm::String::get_chars_ptr(path) : nullptr, utf8);
}

uint32_t copy_utf16_to_kernel32_buffer(const Utf16Char* chars, uint32_t length, uint32_t buffer_length, Utf16Char* buffer)
{
    if (buffer == nullptr || buffer_length <= length)
    {
        return length + 1;
    }
    if (length > 0)
    {
        std::memcpy(buffer, chars, static_cast<size_t>(length) * sizeof(Utf16Char));
    }
    buffer[length] = 0;
    return length;
}

int64_t unix_time_to_file_time(int64_t seconds, int64_t nanoseconds)
{
    constexpr int64_t unix_epoch_as_file_time = 116444736000000000LL;
    constexpr int64_t ticks_per_second = 10000000LL;
    return unix_epoch_as_file_time + seconds * ticks_per_second + nanoseconds / 100;
}

int64_t stat_atime_nsec(const struct stat& st)
{
#if defined(__APPLE__)
    return static_cast<int64_t>(st.st_atimespec.tv_nsec);
#elif defined(__linux__) || defined(__ANDROID__)
    return static_cast<int64_t>(st.st_atim.tv_nsec);
#else
    (void)st;
    return 0;
#endif
}

int64_t stat_mtime_nsec(const struct stat& st)
{
#if defined(__APPLE__)
    return static_cast<int64_t>(st.st_mtimespec.tv_nsec);
#elif defined(__linux__) || defined(__ANDROID__)
    return static_cast<int64_t>(st.st_mtim.tv_nsec);
#else
    (void)st;
    return 0;
#endif
}

int64_t stat_ctime_nsec(const struct stat& st)
{
#if defined(__APPLE__)
    return static_cast<int64_t>(st.st_ctimespec.tv_nsec);
#elif defined(__linux__) || defined(__ANDROID__)
    return static_cast<int64_t>(st.st_ctim.tv_nsec);
#else
    (void)st;
    return 0;
#endif
}

int64_t stat_birth_time(const struct stat& st)
{
#if defined(__APPLE__)
    return unix_time_to_file_time(static_cast<int64_t>(st.st_birthtimespec.tv_sec), static_cast<int64_t>(st.st_birthtimespec.tv_nsec));
#else
    return unix_time_to_file_time(static_cast<int64_t>(st.st_ctime), stat_ctime_nsec(st));
#endif
}

int64_t stat_access_time(const struct stat& st)
{
    return unix_time_to_file_time(static_cast<int64_t>(st.st_atime), stat_atime_nsec(st));
}

int64_t stat_write_time(const struct stat& st)
{
    return unix_time_to_file_time(static_cast<int64_t>(st.st_mtime), stat_mtime_nsec(st));
}

int64_t stat_change_time(const struct stat& st)
{
    return unix_time_to_file_time(static_cast<int64_t>(st.st_ctime), stat_ctime_nsec(st));
}

timespec file_time_to_timespec(int64_t file_time)
{
    constexpr int64_t unix_epoch_as_file_time = 116444736000000000LL;
    constexpr int64_t ticks_per_second = 10000000LL;
    int64_t unix_ticks = file_time - unix_epoch_as_file_time;
    timespec ts{};
    ts.tv_sec = static_cast<time_t>(unix_ticks / ticks_per_second);
    ts.tv_nsec = static_cast<long>((unix_ticks % ticks_per_second) * 100);
    if (ts.tv_nsec < 0)
    {
        ts.tv_nsec += 1000000000L;
        --ts.tv_sec;
    }
    return ts;
}

timeval file_time_to_timeval(int64_t file_time)
{
    timespec ts = file_time_to_timespec(file_time);
    timeval tv{};
    tv.tv_sec = ts.tv_sec;
    tv.tv_usec = static_cast<suseconds_t>(ts.tv_nsec / 1000);
    return tv;
}

bool get_current_directory_utf8(utils::Utf8StringBuilder& cwd)
{
    size_t capacity = 256;
    for (;;)
    {
        char* buffer = static_cast<char*>(std::malloc(capacity));
        if (buffer == nullptr)
        {
            RtSys::set_last_win32_error(kErrorCallNotImplemented);
            return false;
        }
        if (::getcwd(buffer, capacity) != nullptr)
        {
            cwd.append_cstr(buffer);
            std::free(buffer);
            return true;
        }
        int err = errno;
        std::free(buffer);
        if (err != ERANGE)
        {
            set_last_error_from_errno(err);
            return false;
        }
        capacity *= 2;
    }
}

void write_file_time(Win32FileTime& dest, int64_t value)
{
    uint64_t ticks = static_cast<uint64_t>(value);
    dest.LowDateTime = static_cast<uint32_t>(ticks & 0xFFFFFFFFu);
    dest.HighDateTime = static_cast<uint32_t>(ticks >> 32);
}

uint32_t high_u32(uint64_t value)
{
    return static_cast<uint32_t>(value >> 32);
}

uint32_t low_u32(uint64_t value)
{
    return static_cast<uint32_t>(value & 0xFFFFFFFFu);
}

uint32_t attributes_from_stat(const struct stat& st, const char* path)
{
    uint32_t attributes = 0;
    if (S_ISDIR(st.st_mode))
    {
        attributes |= kFileAttributeDirectory;
    }
    if ((st.st_mode & S_IWUSR) == 0)
    {
        attributes |= kFileAttributeReadOnly;
    }
    if (path != nullptr)
    {
        const char* name = std::strrchr(path, '/');
        name = name != nullptr ? name + 1 : path;
        if (name[0] == '.' && name[1] != '\0')
        {
            attributes |= kFileAttributeHidden;
        }
    }
    return attributes != 0 ? attributes : kFileAttributeNormal;
}

void fill_attribute_data(const struct stat& st, const char* path, Win32FileAttributeData& data)
{
    std::memset(&data, 0, sizeof(data));
    data.FileAttributes = attributes_from_stat(st, path);
    write_file_time(data.CreationTime, stat_birth_time(st));
    write_file_time(data.LastAccessTime, stat_access_time(st));
    write_file_time(data.LastWriteTime, stat_write_time(st));
    uint64_t size = static_cast<uint64_t>(st.st_size);
    data.FileSizeHigh = high_u32(size);
    data.FileSizeLow = low_u32(size);
}

void fill_handle_information(const struct stat& st, ByHandleFileInformation& data)
{
    std::memset(&data, 0, sizeof(data));
    data.FileAttributes = attributes_from_stat(st, nullptr);
    write_file_time(data.CreationTime, stat_birth_time(st));
    write_file_time(data.LastAccessTime, stat_access_time(st));
    write_file_time(data.LastWriteTime, stat_write_time(st));
    uint64_t size = static_cast<uint64_t>(st.st_size);
    data.FileSizeHigh = high_u32(size);
    data.FileSizeLow = low_u32(size);
    data.VolumeSerialNumber = low_u32(static_cast<uint64_t>(st.st_dev));
    data.NumberOfLinks = static_cast<uint32_t>(st.st_nlink);
    uint64_t inode = static_cast<uint64_t>(st.st_ino);
    data.FileIndexHigh = high_u32(inode);
    data.FileIndexLow = low_u32(inode);
}

int32_t file_access_from_desired_access(int32_t desired_access)
{
    uint32_t access = static_cast<uint32_t>(desired_access);
    bool read = (access & (kGenericRead | kFileReadData)) != 0;
    bool write = (access & (kGenericWrite | kFileWriteData | kFileAppendData)) != 0;
    if (read && write)
    {
        return 3; // System.IO.FileAccess.ReadWrite
    }
    return write ? 2 : 1; // System.IO.FileAccess.Write / Read
}

template <typename T>
bool write_sized_file_info(void* buffer, uint32_t buffer_size, const T& value)
{
    if (buffer == nullptr || buffer_size < sizeof(T))
    {
        RtSys::set_last_win32_error(kErrorInvalidParameter);
        return false;
    }
    std::memcpy(buffer, &value, sizeof(T));
    RtSys::set_last_win32_error(0);
    return true;
}

bool copy_file_contents(const char* src, const char* dst, uint32_t flags)
{
    int input = -1;
    int output = -1;
    bool ok = false;

    input = ::open(src, O_RDONLY);
    if (input < 0)
    {
        set_last_error_from_errno(errno);
        goto done;
    }

    int out_flags = O_WRONLY | O_CREAT | O_TRUNC;
    if ((flags & kCopyFileFailIfExists) != 0)
    {
        out_flags |= O_EXCL;
    }
    output = ::open(dst, out_flags, 0666);
    if (output < 0)
    {
        set_last_error_from_errno(errno);
        goto done;
    }

    for (;;)
    {
        uint8_t buffer[32768];
        ssize_t read_count;
        do
        {
            read_count = ::read(input, buffer, sizeof(buffer));
        } while (read_count < 0 && errno == EINTR);
        if (read_count < 0)
        {
            set_last_error_from_errno(errno);
            goto done;
        }
        if (read_count == 0)
        {
            break;
        }

        ssize_t written = 0;
        while (written < read_count)
        {
            ssize_t write_count = ::write(output, buffer + written, static_cast<size_t>(read_count - written));
            if (write_count < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                set_last_error_from_errno(errno);
                goto done;
            }
            written += write_count;
        }
    }

    ok = true;
    RtSys::set_last_win32_error(0);

done:
    if (output >= 0)
    {
        int close_result;
        do
        {
            close_result = ::close(output);
        } while (close_result != 0 && errno == EINTR);
        if (close_result != 0 && ok)
        {
            set_last_error_from_errno(errno);
            ok = false;
        }
    }
    if (input >= 0)
    {
        int close_result;
        do
        {
            close_result = ::close(input);
        } while (close_result != 0 && errno == EINTR);
        if (close_result != 0 && ok)
        {
            set_last_error_from_errno(errno);
            ok = false;
        }
    }
    if (!ok)
    {
        (void)::unlink(dst);
    }
    return ok;
}

bool set_file_times_by_handle(int fd, const FileBasicInfo& info)
{
    struct stat st;
    if (::fstat(fd, &st) != 0)
    {
        set_last_error_from_errno(errno);
        return false;
    }

    timeval times[2];
    times[0] = info.LastAccessTime > 0 ? file_time_to_timeval(info.LastAccessTime) : file_time_to_timeval(stat_access_time(st));
    times[1] = info.LastWriteTime > 0 ? file_time_to_timeval(info.LastWriteTime) : file_time_to_timeval(stat_write_time(st));
    if (::futimes(fd, times) != 0)
    {
        set_last_error_from_errno(errno);
        return false;
    }

    mode_t mode = st.st_mode;
    if ((info.FileAttributes & kFileAttributeReadOnly) != 0)
    {
        mode &= static_cast<mode_t>(~(S_IWUSR | S_IWGRP | S_IWOTH));
    }
    else if (info.FileAttributes != 0)
    {
        mode |= S_IWUSR;
    }
    if (mode != st.st_mode && ::fchmod(fd, mode) != 0)
    {
        set_last_error_from_errno(errno);
        return false;
    }
    RtSys::set_last_win32_error(0);
    return true;
}

} // namespace
#endif

int32_t Kernel32::get_console_cp()
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<int32_t>(::GetConsoleCP());
#else
    return 0;
#endif
}

int32_t Kernel32::get_console_output_cp()
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<int32_t>(::GetConsoleOutputCP());
#else
    return 0;
#endif
}

uint64_t Kernel32::get_tick_count64()
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<uint64_t>(::GetTickCount64());
#else
    int64_t milliseconds = os::Time::get_current_time_nanos() / 1000000;
    return milliseconds > 0 ? static_cast<uint64_t>(milliseconds) : 0;
#endif
}

bool Kernel32::get_system_times(int64_t* idle_time, int64_t* kernel_time, int64_t* user_time)
{
#ifdef LEANCLR_PLATFORM_WIN
    FILETIME idle{};
    FILETIME kernel{};
    FILETIME user{};
    FILETIME* idle_ptr = idle_time != nullptr ? &idle : nullptr;
    FILETIME* kernel_ptr = kernel_time != nullptr ? &kernel : nullptr;
    FILETIME* user_ptr = user_time != nullptr ? &user : nullptr;
    if (::GetSystemTimes(idle_ptr, kernel_ptr, user_ptr) == 0)
    {
        return false;
    }
    auto to_int64 = [](const FILETIME& time) -> int64_t {
        ULARGE_INTEGER value{};
        value.LowPart = time.dwLowDateTime;
        value.HighPart = time.dwHighDateTime;
        return static_cast<int64_t>(value.QuadPart);
    };
    if (idle_time != nullptr)
    {
        *idle_time = to_int64(idle);
    }
    if (kernel_time != nullptr)
    {
        *kernel_time = to_int64(kernel);
    }
    if (user_time != nullptr)
    {
        *user_time = to_int64(user);
    }
    return true;
#else
    int64_t now = os::Time::get_system_time_as_file_time();
    if (idle_time != nullptr)
    {
        *idle_time = 0;
    }
    if (kernel_time != nullptr)
    {
        *kernel_time = now;
    }
    if (user_time != nullptr)
    {
        *user_time = now;
    }
    return true;
#endif
}

intptr_t Kernel32::get_current_thread()
{
#ifdef LEANCLR_PLATFORM_WIN
    return reinterpret_cast<intptr_t>(::GetCurrentThread());
#else
    return static_cast<intptr_t>(-2);
#endif
}

int32_t Kernel32::get_current_thread_id()
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<int32_t>(::GetCurrentThreadId());
#else
    return 1;
#endif
}

bool Kernel32::get_thread_io_pending_flag(intptr_t thread_handle, int32_t* is_io_pending)
{
    if (is_io_pending == nullptr)
    {
        return false;
    }
#ifdef LEANCLR_PLATFORM_WIN
    BOOL pending = FALSE;
    if (::GetThreadIOPendingFlag(reinterpret_cast<HANDLE>(thread_handle), &pending) == 0)
    {
        *is_io_pending = 0;
        return false;
    }
    *is_io_pending = pending != FALSE ? 1 : 0;
    return true;
#else
    (void)thread_handle;
    *is_io_pending = 0;
    return true;
#endif
}

intptr_t Kernel32::get_std_handle(int32_t std_handle)
{
#ifdef LEANCLR_PLATFORM_WIN
    return reinterpret_cast<intptr_t>(::GetStdHandle(static_cast<DWORD>(std_handle)));
#else
    (void)std_handle;
    return static_cast<intptr_t>(-1);
#endif
}

bool Kernel32::get_console_screen_buffer_info(intptr_t handle, void* info)
{
#ifdef LEANCLR_PLATFORM_WIN
    if (info == nullptr)
    {
        return false;
    }
    CONSOLE_SCREEN_BUFFER_INFO native_info{};
    if (::GetConsoleScreenBufferInfo(reinterpret_cast<HANDLE>(handle), &native_info) == 0)
    {
        return false;
    }
    // Managed System.ConsoleScreenBufferInfo matches WIN32 CONSOLE_SCREEN_BUFFER_INFO layout.
    std::memcpy(info, &native_info, sizeof(CONSOLE_SCREEN_BUFFER_INFO));
    return true;
#else
    (void)handle;
    (void)info;
    return false;
#endif
}

#ifdef LEANCLR_PLATFORM_WIN
namespace
{

// Matches System.WindowsConsoleDriver.InputRecord in mscorlib (flattened KEY_EVENT fields).
struct ManagedInputRecord
{
    int16_t event_type;
    bool key_down;
    int16_t repeat_count;
    int16_t virtual_key_code;
    int16_t virtual_scan_code;
    char16_t character;
    int32_t control_key_state;
    int32_t pad1;
    bool pad2;
};

void write_managed_input_record(void* dest, const INPUT_RECORD& native) noexcept
{
    ManagedInputRecord managed{};
    managed.event_type = static_cast<int16_t>(native.EventType);
    if (native.EventType == KEY_EVENT)
    {
        const KEY_EVENT_RECORD& key = native.Event.KeyEvent;
        managed.key_down = key.bKeyDown != FALSE;
        managed.repeat_count = static_cast<int16_t>(key.wRepeatCount);
        managed.virtual_key_code = static_cast<int16_t>(key.wVirtualKeyCode);
        managed.virtual_scan_code = static_cast<int16_t>(key.wVirtualScanCode);
        managed.character = key.uChar.UnicodeChar;
        managed.control_key_state = static_cast<int32_t>(key.dwControlKeyState);
    }
    std::memcpy(dest, &managed, sizeof(ManagedInputRecord));
}

} // namespace
#endif

bool Kernel32::peek_console_input(intptr_t handle, void* record, int32_t length, int32_t* events_read)
{
#ifdef LEANCLR_PLATFORM_WIN
    if (record == nullptr || events_read == nullptr || length <= 0)
    {
        return false;
    }

    INPUT_RECORD native_record{};
    DWORD native_events_read = 0;
    if (::PeekConsoleInput(reinterpret_cast<HANDLE>(handle), &native_record, static_cast<DWORD>(length), &native_events_read) == 0)
    {
        return false;
    }

    write_managed_input_record(record, native_record);
    *events_read = static_cast<int32_t>(native_events_read);
    return true;
#else
    (void)handle;
    (void)record;
    (void)length;
    (void)events_read;
    return false;
#endif
}

bool Kernel32::read_console_input(intptr_t handle, void* record, int32_t length, int32_t* events_read)
{
#ifdef LEANCLR_PLATFORM_WIN
    if (record == nullptr || events_read == nullptr || length <= 0)
    {
        return false;
    }

    INPUT_RECORD native_record{};
    DWORD native_events_read = 0;
    if (::ReadConsoleInput(reinterpret_cast<HANDLE>(handle), &native_record, static_cast<DWORD>(length), &native_events_read) == 0)
    {
        return false;
    }

    write_managed_input_record(record, native_record);
    *events_read = static_cast<int32_t>(native_events_read);
    return true;
#else
    (void)handle;
    (void)record;
    (void)length;
    (void)events_read;
    return false;
#endif
}

bool Kernel32::get_console_mode(intptr_t handle, int32_t* mode)
{
#ifdef LEANCLR_PLATFORM_WIN
    if (mode == nullptr)
    {
        return false;
    }
    DWORD native_mode = 0;
    if (::GetConsoleMode(reinterpret_cast<HANDLE>(handle), &native_mode) == 0)
    {
        return false;
    }
    *mode = static_cast<int32_t>(native_mode);
    return true;
#else
    (void)handle;
    (void)mode;
    return false;
#endif
}

bool Kernel32::set_console_mode(intptr_t handle, int32_t mode)
{
#ifdef LEANCLR_PLATFORM_WIN
    return ::SetConsoleMode(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(mode)) != 0;
#else
    (void)handle;
    (void)mode;
    return false;
#endif
}

int32_t Kernel32::format_message(int32_t flags, intptr_t source, uint32_t message_id, int32_t language_id, void* buffer, int32_t buffer_chars,
                                 intptr_t arguments)
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<int32_t>(::FormatMessageW(static_cast<DWORD>(flags), reinterpret_cast<LPCVOID>(source), static_cast<DWORD>(message_id),
                                                 static_cast<DWORD>(language_id), reinterpret_cast<LPWSTR>(buffer), static_cast<DWORD>(buffer_chars),
                                                 reinterpret_cast<va_list*>(arguments)));
#else
    (void)flags;
    (void)source;
    (void)message_id;
    (void)language_id;
    (void)buffer;
    (void)buffer_chars;
    (void)arguments;
    return 0;
#endif
}

uint32_t Kernel32::get_full_path_name(const Utf16Char* path, uint32_t buffer_length, Utf16Char* buffer, intptr_t file_part)
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<uint32_t>(::GetFullPathNameW(
        reinterpret_cast<LPCWSTR>(path),
        static_cast<DWORD>(buffer_length),
        reinterpret_cast<LPWSTR>(buffer),
        reinterpret_cast<LPWSTR*>(file_part)));
#else
    utils::Utf8StringBuilder path_utf8;
    if (!utf16_path_to_utf8(path, path_utf8))
    {
        return 0;
    }

    utils::Utf8StringBuilder full_path_utf8;
    const char* raw_path = path_utf8.get_const_chars();
    if (raw_path[0] == '/')
    {
        full_path_utf8.append_cstr(raw_path);
    }
    else
    {
        utils::Utf8StringBuilder cwd_utf8;
        if (!get_current_directory_utf8(cwd_utf8))
        {
            return 0;
        }
        full_path_utf8.append_cstr(cwd_utf8.get_const_chars(), cwd_utf8.length());
        if (cwd_utf8.length() == 0 || cwd_utf8.get_const_chars()[cwd_utf8.length() - 1] != '/')
        {
            full_path_utf8.append_char('/');
        }
        full_path_utf8.append_cstr(raw_path);
    }

    utils::Utf16StringBuilder full_path_utf16;
    full_path_utf16.append_utf8_str(full_path_utf8.get_const_chars(), full_path_utf8.length());
    uint32_t length = static_cast<uint32_t>(full_path_utf16.length());
    uint32_t result = copy_utf16_to_kernel32_buffer(full_path_utf16.get_const_chars(), length, buffer_length, buffer);
    if (file_part != 0)
    {
        Utf16Char** file_part_ptr = reinterpret_cast<Utf16Char**>(file_part);
        *file_part_ptr = nullptr;
        if (buffer != nullptr && result == length)
        {
            uint32_t index = length;
            while (index > 0)
            {
                if (buffer[index - 1] == static_cast<Utf16Char>('/'))
                {
                    *file_part_ptr = buffer + index;
                    break;
                }
                --index;
            }
        }
    }
    RtSys::set_last_win32_error(0);
    return result;
#endif
}

intptr_t Kernel32::create_event_ex(intptr_t security_attributes, const Utf16Char* name, uint32_t flags, uint32_t desired_access)
{
#ifdef LEANCLR_PLATFORM_WIN
    HANDLE handle = ::CreateEventExW(reinterpret_cast<LPSECURITY_ATTRIBUTES>(security_attributes), reinterpret_cast<LPCWSTR>(name),
                                     static_cast<DWORD>(flags), static_cast<DWORD>(desired_access));
    return reinterpret_cast<intptr_t>(handle);
#else
    (void)security_attributes;
    (void)desired_access;
    if (name != nullptr)
    {
        return 0;
    }
    constexpr uint32_t create_event_manual_reset = 0x1;
    constexpr uint32_t create_event_initial_set = 0x2;
    auto* ev = new Event((flags & create_event_manual_reset) != 0, (flags & create_event_initial_set) != 0);
    auto* handle = new EventHandle(ev);
    intptr_t result = reinterpret_cast<intptr_t>(handle);
    register_kernel32_event_handle(result);
    return result;
#endif
}

intptr_t Kernel32::open_event(uint32_t desired_access, int32_t inherit_handle, const Utf16Char* name)
{
#ifdef LEANCLR_PLATFORM_WIN
    HANDLE handle = ::OpenEventW(static_cast<DWORD>(desired_access), inherit_handle != 0 ? TRUE : FALSE, reinterpret_cast<LPCWSTR>(name));
    return reinterpret_cast<intptr_t>(handle);
#else
    (void)desired_access;
    (void)inherit_handle;
    (void)name;
    return 0;
#endif
}

bool Kernel32::set_event(intptr_t handle)
{
#ifdef LEANCLR_PLATFORM_WIN
    return ::SetEvent(reinterpret_cast<HANDLE>(handle)) != 0;
#else
    if (handle == 0)
    {
        return false;
    }
    return reinterpret_cast<EventHandle*>(handle)->get().set();
#endif
}

bool Kernel32::reset_event(intptr_t handle)
{
#ifdef LEANCLR_PLATFORM_WIN
    return ::ResetEvent(reinterpret_cast<HANDLE>(handle)) != 0;
#else
    if (handle == 0)
    {
        return false;
    }
    return reinterpret_cast<EventHandle*>(handle)->get().reset();
#endif
}

int32_t Kernel32::wait_for_single_object_ex(intptr_t handle, int32_t milliseconds, int32_t alertable)
{
#ifdef LEANCLR_PLATFORM_WIN
    DWORD timeout = milliseconds < 0 ? INFINITE : static_cast<DWORD>(milliseconds);
    DWORD result = ::WaitForSingleObjectEx(reinterpret_cast<HANDLE>(handle), timeout, alertable != 0 ? TRUE : FALSE);
    return static_cast<int32_t>(result);
#else
    (void)handle;
    (void)milliseconds;
    (void)alertable;
    return 0;
#endif
}

int32_t Kernel32::wait_for_multiple_objects_ex(intptr_t* handles, int32_t count, int32_t wait_all, int32_t milliseconds)
{
    if (handles == nullptr || count <= 0)
    {
        return static_cast<int32_t>(0xFFFFFFFF);
    }
#ifdef LEANCLR_PLATFORM_WIN
    DWORD timeout = milliseconds < 0 ? INFINITE : static_cast<DWORD>(milliseconds);
    DWORD handle_count = static_cast<DWORD>(count);
    HANDLE native_handles[MAXIMUM_WAIT_OBJECTS]{};
    if (handle_count > MAXIMUM_WAIT_OBJECTS)
    {
        return static_cast<int32_t>(0xFFFFFFFF);
    }
    for (DWORD i = 0; i < handle_count; ++i)
    {
        native_handles[i] = reinterpret_cast<HANDLE>(handles[i]);
    }
    DWORD result = ::WaitForMultipleObjectsEx(handle_count, native_handles, wait_all != 0 ? TRUE : FALSE, timeout, FALSE);
    return static_cast<int32_t>(result);
#else
    (void)wait_all;
    (void)milliseconds;
    return 0;
#endif
}

#ifdef LEANCLR_PLATFORM_WIN
bool Kernel32::set_thread_error_mode(uint32_t mode, uint32_t& old_mode)
{
    // SetThreadErrorMode (Vista+, kernel32): per-thread error mode; does not
    // change the process-wide default the way SetErrorMode does.
    // https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setthreaderrormode
    //
    // Use DWORD at the API boundary: Win32 typedefs DWORD as unsigned long;
    // uint32_t is often unsigned int - passing &uint32_t where LPDWORD is
    // expected can trigger MSVC C4312-style strictness issues.
    DWORD old = 0;
    const DWORD new_mode = static_cast<DWORD>(mode);
    if (::SetThreadErrorMode(new_mode, &old) == 0)
    {
        old_mode = 0;
        return false;
    }
    old_mode = static_cast<uint32_t>(old);
    return true;
}

bool Kernel32::get_file_attributes_ex_private(vm::RtString* name, uint32_t file_info_level, void* file_info)
{
    return ::GetFileAttributesExW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(name)), static_cast<GET_FILEEX_INFO_LEVELS>(file_info_level),
                                  file_info) != 0;
}

intptr_t Kernel32::find_first_file_ex_private(vm::RtString* lp_file_name, uint32_t f_info_level_id, void* lp_find_file_data, uint32_t f_search_op,
                                              intptr_t lp_search_filter, int32_t dw_additional_flags)
{
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfileexw
    if (lp_file_name == nullptr || lp_find_file_data == nullptr)
        return reinterpret_cast<intptr_t>(INVALID_HANDLE_VALUE);

    LPVOID search_filter = (lp_search_filter != 0) ? reinterpret_cast<LPVOID>(lp_search_filter) : nullptr;
    HANDLE h = ::FindFirstFileExW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(lp_file_name)), static_cast<FINDEX_INFO_LEVELS>(f_info_level_id),
                                  lp_find_file_data, static_cast<FINDEX_SEARCH_OPS>(f_search_op), search_filter, static_cast<DWORD>(dw_additional_flags));
    return reinterpret_cast<intptr_t>(h);
}

uint32_t Kernel32::get_time_zone_information(void* lp_time_zone_information)
{
    if (lp_time_zone_information == nullptr)
        return 0;
    return static_cast<uint32_t>(::GetTimeZoneInformation(static_cast<TIME_ZONE_INFORMATION*>(lp_time_zone_information)));
}

uint32_t Kernel32::get_dynamic_time_zone_information(void* lp_dynamic_tz)
{
    if (lp_dynamic_tz == nullptr)
        return 0;
    return static_cast<uint32_t>(::GetDynamicTimeZoneInformation(static_cast<DYNAMIC_TIME_ZONE_INFORMATION*>(lp_dynamic_tz)));
}

bool Kernel32::delete_volume_mount_point_private(vm::RtString* mount_point)
{
    if (mount_point == nullptr)
        return false;
    return ::DeleteVolumeMountPointW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(mount_point))) != 0;
}

bool Kernel32::free_library(intptr_t h_module)
{
    if (h_module == 0)
        return false;
    return ::FreeLibrary(reinterpret_cast<HMODULE>(h_module)) != 0;
}

intptr_t Kernel32::load_library_ex(vm::RtString* lib_filename, intptr_t reserved, int32_t flags)
{
    if (lib_filename == nullptr)
        return 0;
    return load_library_ex(vm::String::get_chars_ptr(lib_filename), reserved, flags);
}

intptr_t Kernel32::load_library_ex(const Utf16Char* lib_filename, intptr_t reserved, int32_t flags)
{
    (void)reserved;
    if (lib_filename == nullptr)
        return 0;
    HMODULE m = ::LoadLibraryExW(reinterpret_cast<LPCWSTR>(lib_filename), nullptr, static_cast<DWORD>(flags));
    return reinterpret_cast<intptr_t>(m);
}

intptr_t Kernel32::get_proc_address(intptr_t h_module, const char* proc_name)
{
    if (h_module == 0 || proc_name == nullptr)
        return 0;
    FARPROC proc = ::GetProcAddress(reinterpret_cast<HMODULE>(h_module), proc_name);
    return reinterpret_cast<intptr_t>(proc);
}

uint32_t Kernel32::get_temp_path(uint32_t buffer_length, Utf16Char* buffer)
{
    return static_cast<uint32_t>(::GetTempPathW(static_cast<DWORD>(buffer_length), reinterpret_cast<LPWSTR>(buffer)));
}

bool Kernel32::close_handle(intptr_t handle)
{
    if (handle == 0 || handle == reinterpret_cast<intptr_t>(INVALID_HANDLE_VALUE))
        return false;
    return ::CloseHandle(reinterpret_cast<HANDLE>(handle)) != 0;
}

bool Kernel32::query_performance_frequency(int64_t* frequency)
{
    if (frequency == nullptr)
        return false;
    LARGE_INTEGER value{};
    if (::QueryPerformanceFrequency(&value) == 0)
        return false;
    *frequency = static_cast<int64_t>(value.QuadPart);
    return true;
}

bool Kernel32::query_performance_counter(int64_t* counter)
{
    if (counter == nullptr)
        return false;
    LARGE_INTEGER value{};
    if (::QueryPerformanceCounter(&value) == 0)
        return false;
    *counter = static_cast<int64_t>(value.QuadPart);
    return true;
}

void Kernel32::initialize_critical_section(void* critical_section)
{
    if (critical_section == nullptr)
        return;
    ::InitializeCriticalSection(static_cast<CRITICAL_SECTION*>(critical_section));
}

void Kernel32::delete_critical_section(void* critical_section)
{
    if (critical_section == nullptr)
        return;
    ::DeleteCriticalSection(static_cast<CRITICAL_SECTION*>(critical_section));
}

void Kernel32::enter_critical_section(void* critical_section)
{
    if (critical_section == nullptr)
        return;
    ::EnterCriticalSection(static_cast<CRITICAL_SECTION*>(critical_section));
}

void Kernel32::leave_critical_section(void* critical_section)
{
    if (critical_section == nullptr)
        return;
    ::LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(critical_section));
}

void Kernel32::initialize_condition_variable(void* condition_variable)
{
    if (condition_variable == nullptr)
        return;
    ::InitializeConditionVariable(static_cast<CONDITION_VARIABLE*>(condition_variable));
}

bool Kernel32::sleep_condition_variable_cs(void* condition_variable, void* critical_section, int32_t milliseconds)
{
    if (condition_variable == nullptr || critical_section == nullptr)
        return false;
    DWORD timeout = milliseconds < 0 ? INFINITE : static_cast<DWORD>(milliseconds);
    return ::SleepConditionVariableCS(static_cast<CONDITION_VARIABLE*>(condition_variable), static_cast<CRITICAL_SECTION*>(critical_section),
                                      timeout) != 0;
}

void Kernel32::wake_condition_variable(void* condition_variable)
{
    if (condition_variable == nullptr)
        return;
    ::WakeConditionVariable(static_cast<CONDITION_VARIABLE*>(condition_variable));
}

intptr_t Kernel32::create_io_completion_port(intptr_t file_handle, intptr_t existing_completion_port, uintptr_t completion_key,
                                             int32_t number_of_concurrent_threads)
{
    HANDLE h = ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(file_handle), reinterpret_cast<HANDLE>(existing_completion_port),
                                        static_cast<ULONG_PTR>(completion_key), static_cast<DWORD>(number_of_concurrent_threads));
    return reinterpret_cast<intptr_t>(h);
}

bool Kernel32::post_queued_completion_status(intptr_t completion_port, uint32_t number_of_bytes_transferred, uintptr_t completion_key,
                                             intptr_t overlapped)
{
    return ::PostQueuedCompletionStatus(reinterpret_cast<HANDLE>(completion_port), static_cast<DWORD>(number_of_bytes_transferred),
                                        static_cast<ULONG_PTR>(completion_key), reinterpret_cast<LPOVERLAPPED>(overlapped)) != 0;
}

bool Kernel32::get_queued_completion_status(intptr_t completion_port, uint32_t* number_of_bytes_transferred, uintptr_t* completion_key,
                                            intptr_t* overlapped, int32_t milliseconds)
{
    DWORD native_bytes = 0;
    ULONG_PTR native_completion_key = 0;
    LPOVERLAPPED native_overlapped = nullptr;
    DWORD timeout = milliseconds < 0 ? INFINITE : static_cast<DWORD>(milliseconds);
    BOOL ok = ::GetQueuedCompletionStatus(reinterpret_cast<HANDLE>(completion_port), &native_bytes, &native_completion_key, &native_overlapped, timeout);
    if (number_of_bytes_transferred != nullptr)
        *number_of_bytes_transferred = static_cast<uint32_t>(native_bytes);
    if (completion_key != nullptr)
        *completion_key = static_cast<uintptr_t>(native_completion_key);
    if (overlapped != nullptr)
        *overlapped = reinterpret_cast<intptr_t>(native_overlapped);
    return ok != 0;
}

bool Kernel32::get_queued_completion_status_ex(intptr_t completion_port, void* completion_port_entries, int32_t count,
                                               int32_t* number_of_entries_removed, int32_t milliseconds, int32_t alertable)
{
    if (completion_port_entries == nullptr || count <= 0)
    {
        if (number_of_entries_removed != nullptr)
            *number_of_entries_removed = 0;
        return false;
    }

    ULONG native_entries_removed = 0;
    DWORD timeout = milliseconds < 0 ? INFINITE : static_cast<DWORD>(milliseconds);
    BOOL ok = ::GetQueuedCompletionStatusEx(reinterpret_cast<HANDLE>(completion_port), static_cast<LPOVERLAPPED_ENTRY>(completion_port_entries),
                                            static_cast<ULONG>(count), &native_entries_removed, timeout, alertable != 0 ? TRUE : FALSE);
    if (number_of_entries_removed != nullptr)
        *number_of_entries_removed = static_cast<int32_t>(native_entries_removed);
    return ok != 0;
}

int32_t Kernel32::copy_file2(vm::RtString* existing, vm::RtString* new_file, void* extended_parameters)
{
    const HRESULT hr = ::CopyFile2(reinterpret_cast<PCWSTR>(vm::String::get_chars_ptr(existing)), reinterpret_cast<PCWSTR>(vm::String::get_chars_ptr(new_file)),
                                   static_cast<COPYFILE2_EXTENDED_PARAMETERS*>(extended_parameters));
    return static_cast<int32_t>(hr);
}

bool Kernel32::copy_file_ex_private(vm::RtString* src, vm::RtString* dst, intptr_t progress_routine, intptr_t progress_data, int32_t* cancel, int32_t flags)
{
    BOOL cancel_flag = *cancel ? TRUE : FALSE;
    const BOOL ok =
        ::CopyFileExW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(src)), reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(dst)),
                      reinterpret_cast<LPPROGRESS_ROUTINE>(progress_routine), reinterpret_cast<LPVOID>(progress_data), &cancel_flag, static_cast<DWORD>(flags));
    *cancel = cancel_flag ? 1 : 0;
    return ok != 0;
}

bool Kernel32::create_directory_private(vm::RtString* path, void* security_attributes)
{
    return ::CreateDirectoryW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(path)), static_cast<LPSECURITY_ATTRIBUTES>(security_attributes)) != 0;
}

intptr_t Kernel32::create_file_private(vm::RtString* name, int32_t desired_access, int32_t share_mode, void* security_attributes, int32_t creation_disposition,
                                       int32_t flags_and_attributes, intptr_t template_file)
{
    return create_file_private(vm::String::get_chars_ptr(name), desired_access, share_mode, security_attributes, creation_disposition,
                               flags_and_attributes, template_file);
}

intptr_t Kernel32::create_file_private(const Utf16Char* name, int32_t desired_access, int32_t share_mode, void* security_attributes,
                                       int32_t creation_disposition, int32_t flags_and_attributes, intptr_t template_file)
{
    if (name == nullptr)
        return reinterpret_cast<intptr_t>(INVALID_HANDLE_VALUE);
    HANDLE h = ::CreateFileW(reinterpret_cast<LPCWSTR>(name), static_cast<DWORD>(desired_access), static_cast<DWORD>(share_mode),
                             static_cast<LPSECURITY_ATTRIBUTES>(security_attributes), static_cast<DWORD>(creation_disposition),
                             static_cast<DWORD>(flags_and_attributes), reinterpret_cast<HANDLE>(template_file));
    return reinterpret_cast<intptr_t>(h);
}

bool Kernel32::delete_file_private(vm::RtString* path)
{
    return delete_file_private(path != nullptr ? vm::String::get_chars_ptr(path) : nullptr);
}

bool Kernel32::delete_file_private(const Utf16Char* path)
{
    if (path == nullptr)
        return false;
    return ::DeleteFileW(reinterpret_cast<LPCWSTR>(path)) != 0;
}

bool Kernel32::find_next_file(intptr_t find_handle, void* find_file_data)
{
    if (find_handle == 0 || find_file_data == nullptr)
        return false;
    return ::FindNextFileW(reinterpret_cast<HANDLE>(find_handle), static_cast<LPWIN32_FIND_DATAW>(find_file_data)) != 0;
}

bool Kernel32::get_file_attributes_ex_private(const Utf16Char* name, uint32_t file_info_level, void* file_info)
{
    if (name == nullptr || file_info == nullptr)
        return false;
    return ::GetFileAttributesExW(reinterpret_cast<LPCWSTR>(name), static_cast<GET_FILEEX_INFO_LEVELS>(file_info_level), file_info) != 0;
}

bool Kernel32::get_file_information_by_handle(intptr_t h_file, void* file_information)
{
    if (file_information == nullptr)
        return false;
    return ::GetFileInformationByHandle(reinterpret_cast<HANDLE>(h_file), static_cast<LPBY_HANDLE_FILE_INFORMATION>(file_information)) != 0;
}

bool Kernel32::get_file_information_by_handle_ex(intptr_t h_file, int32_t file_information_class, void* file_information, uint32_t buffer_size)
{
    return ::GetFileInformationByHandleEx(reinterpret_cast<HANDLE>(h_file), static_cast<FILE_INFO_BY_HANDLE_CLASS>(file_information_class), file_information,
                                          static_cast<DWORD>(buffer_size)) != 0;
}

int32_t Kernel32::get_logical_drives()
{
    return static_cast<int32_t>(::GetLogicalDrives());
}

bool Kernel32::move_file_ex_private(vm::RtString* src, vm::RtString* dst, uint32_t flags)
{
    return ::MoveFileExW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(src)), reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(dst)),
                         static_cast<DWORD>(flags)) != 0;
}

bool Kernel32::remove_directory_private(vm::RtString* path)
{
    return ::RemoveDirectoryW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(path))) != 0;
}

bool Kernel32::replace_file_private(vm::RtString* replaced_file_name, vm::RtString* replacement_file_name, vm::RtString* backup_file_name,
                                    int32_t replace_flags, intptr_t exclude, intptr_t reserved)
{
    LPCWSTR backup = backup_file_name ? reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(backup_file_name)) : nullptr;
    return ::ReplaceFileW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(replaced_file_name)),
                          reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(replacement_file_name)), backup, static_cast<DWORD>(replace_flags),
                          reinterpret_cast<LPVOID>(exclude), reinterpret_cast<LPVOID>(reserved)) != 0;
}

bool Kernel32::set_file_attributes_private(vm::RtString* name, int32_t attributes)
{
    return ::SetFileAttributesW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(name)), static_cast<DWORD>(attributes)) != 0;
}

bool Kernel32::set_file_information_by_handle(intptr_t h_file, int32_t file_information_class, void* file_information, uint32_t buffer_size)
{
    return ::SetFileInformationByHandle(reinterpret_cast<HANDLE>(h_file), static_cast<FILE_INFO_BY_HANDLE_CLASS>(file_information_class), file_information,
                                        static_cast<DWORD>(buffer_size)) != 0;
}
#endif

#ifndef LEANCLR_PLATFORM_WIN
uint32_t Kernel32::get_temp_path(uint32_t buffer_length, Utf16Char* buffer)
{
    vm::RtString* temp_path = os::Path::get_temp_path();
    if (temp_path == nullptr)
    {
        RtSys::set_last_win32_error(kErrorCallNotImplemented);
        return 0;
    }

    const Utf16Char* chars = vm::String::get_chars_ptr(temp_path);
    uint32_t length = static_cast<uint32_t>(vm::String::get_length(temp_path));
    bool needs_separator = length == 0 ||
        (chars[length - 1] != static_cast<Utf16Char>('/') && chars[length - 1] != static_cast<Utf16Char>('\\'));
    uint32_t result_length = length + (needs_separator ? 1u : 0u);
    if (buffer == nullptr || buffer_length <= result_length)
    {
        return result_length + 1;
    }
    if (length > 0)
    {
        std::memcpy(buffer, chars, static_cast<size_t>(length) * sizeof(Utf16Char));
    }
    if (needs_separator)
    {
        buffer[length] = static_cast<Utf16Char>('/');
    }
    buffer[result_length] = 0;
    RtSys::set_last_win32_error(0);
    return result_length;
}

int32_t Kernel32::copy_file2(vm::RtString* existing, vm::RtString* new_file, void* extended_parameters)
{
    (void)extended_parameters;
    utils::Utf8StringBuilder src;
    utils::Utf8StringBuilder dst;
    if (!rt_string_path_to_utf8(existing, src) || !rt_string_path_to_utf8(new_file, dst))
    {
        return static_cast<int32_t>(0x80070000u | static_cast<uint32_t>(kErrorInvalidParameter));
    }
    if (!copy_file_contents(src.get_const_chars(), dst.get_const_chars(), 0))
    {
        int32_t error = RtSys::get_last_win32_error();
        return static_cast<int32_t>(0x80070000u | static_cast<uint32_t>(error & 0xFFFF));
    }
    return 0;
}

bool Kernel32::copy_file_ex_private(vm::RtString* src, vm::RtString* dst, intptr_t progress_routine, intptr_t progress_data, int32_t* cancel, int32_t flags)
{
    (void)progress_routine;
    (void)progress_data;
    if (cancel != nullptr && *cancel != 0)
    {
        RtSys::set_last_win32_error(kErrorCallNotImplemented);
        return false;
    }
    utils::Utf8StringBuilder src_path;
    utils::Utf8StringBuilder dst_path;
    if (!rt_string_path_to_utf8(src, src_path) || !rt_string_path_to_utf8(dst, dst_path))
    {
        return false;
    }
    return copy_file_contents(src_path.get_const_chars(), dst_path.get_const_chars(), static_cast<uint32_t>(flags));
}

bool Kernel32::create_directory_private(vm::RtString* path, void* security_attributes)
{
    (void)security_attributes;
    utils::Utf8StringBuilder utf8_path;
    if (!rt_string_path_to_utf8(path, utf8_path))
    {
        return false;
    }
    if (::mkdir(utf8_path.get_const_chars(), 0777) != 0)
    {
        set_last_error_from_errno(errno);
        return false;
    }
    RtSys::set_last_win32_error(0);
    return true;
}

int32_t Kernel32::get_logical_drives()
{
    RtSys::set_last_win32_error(0);
    return 0;
}

bool Kernel32::move_file_ex_private(vm::RtString* src, vm::RtString* dst, uint32_t flags)
{
    if ((flags & kMoveFileDelayUntilReboot) != 0)
    {
        RtSys::set_last_win32_error(kErrorCallNotImplemented);
        return false;
    }

    utils::Utf8StringBuilder src_path;
    utils::Utf8StringBuilder dst_path;
    if (!rt_string_path_to_utf8(src, src_path) || !rt_string_path_to_utf8(dst, dst_path))
    {
        return false;
    }

    if ((flags & kMoveFileReplaceExisting) == 0 && ::access(dst_path.get_const_chars(), F_OK) == 0)
    {
        RtSys::set_last_win32_error(os::io_error_internal::kErrorFileExists);
        return false;
    }

    if (::rename(src_path.get_const_chars(), dst_path.get_const_chars()) == 0)
    {
        RtSys::set_last_win32_error(0);
        return true;
    }

    int err = errno;
    if (err == EXDEV && (flags & kMoveFileCopyAllowed) != 0)
    {
        uint32_t copy_flags = (flags & kMoveFileReplaceExisting) != 0 ? 0 : kCopyFileFailIfExists;
        if (!copy_file_contents(src_path.get_const_chars(), dst_path.get_const_chars(), copy_flags))
        {
            return false;
        }
        if (::unlink(src_path.get_const_chars()) != 0)
        {
            set_last_error_from_errno(errno);
            return false;
        }
        RtSys::set_last_win32_error(0);
        return true;
    }

    set_last_error_from_errno(err);
    return false;
}

bool Kernel32::remove_directory_private(vm::RtString* path)
{
    utils::Utf8StringBuilder utf8_path;
    if (!rt_string_path_to_utf8(path, utf8_path))
    {
        return false;
    }
    if (::rmdir(utf8_path.get_const_chars()) != 0)
    {
        set_last_error_from_errno(errno);
        return false;
    }
    RtSys::set_last_win32_error(0);
    return true;
}

bool Kernel32::replace_file_private(vm::RtString* replaced_file_name, vm::RtString* replacement_file_name, vm::RtString* backup_file_name,
                                    int32_t replace_flags, intptr_t exclude, intptr_t reserved)
{
    (void)replace_flags;
    (void)exclude;
    (void)reserved;
    utils::Utf8StringBuilder replaced_path;
    utils::Utf8StringBuilder replacement_path;
    if (!rt_string_path_to_utf8(replaced_file_name, replaced_path) || !rt_string_path_to_utf8(replacement_file_name, replacement_path))
    {
        return false;
    }

    if (backup_file_name != nullptr)
    {
        utils::Utf8StringBuilder backup_path;
        if (!rt_string_path_to_utf8(backup_file_name, backup_path))
        {
            return false;
        }
        (void)::unlink(backup_path.get_const_chars());
        if (::rename(replaced_path.get_const_chars(), backup_path.get_const_chars()) != 0)
        {
            set_last_error_from_errno(errno);
            return false;
        }
        if (::rename(replacement_path.get_const_chars(), replaced_path.get_const_chars()) != 0)
        {
            int err = errno;
            (void)::rename(backup_path.get_const_chars(), replaced_path.get_const_chars());
            set_last_error_from_errno(err);
            return false;
        }
    }
    else if (::rename(replacement_path.get_const_chars(), replaced_path.get_const_chars()) != 0)
    {
        set_last_error_from_errno(errno);
        return false;
    }

    RtSys::set_last_win32_error(0);
    return true;
}

bool Kernel32::set_file_attributes_private(vm::RtString* name, int32_t attributes)
{
    utils::Utf8StringBuilder utf8_path;
    if (!rt_string_path_to_utf8(name, utf8_path))
    {
        return false;
    }
    struct stat st;
    if (::stat(utf8_path.get_const_chars(), &st) != 0)
    {
        set_last_error_from_errno(errno);
        return false;
    }

    mode_t mode = st.st_mode;
    if ((static_cast<uint32_t>(attributes) & kFileAttributeReadOnly) != 0)
    {
        mode &= static_cast<mode_t>(~(S_IWUSR | S_IWGRP | S_IWOTH));
    }
    else
    {
        mode |= S_IWUSR;
    }
    if (::chmod(utf8_path.get_const_chars(), mode) != 0)
    {
        set_last_error_from_errno(errno);
        return false;
    }
    RtSys::set_last_win32_error(0);
    return true;
}

bool Kernel32::get_file_attributes_ex_private(vm::RtString* name, uint32_t file_info_level, void* file_info)
{
    return get_file_attributes_ex_private(name != nullptr ? vm::String::get_chars_ptr(name) : nullptr, file_info_level, file_info);
}

bool Kernel32::close_handle(intptr_t handle)
{
    if (handle == 0 || handle == kInvalidHandle)
    {
        RtSys::set_last_win32_error(kErrorInvalidHandle);
        return false;
    }
    if (close_registered_kernel32_event_handle(handle))
    {
        RtSys::set_last_win32_error(0);
        return true;
    }
    int32_t error = 0;
    bool ok = os::File::close(handle, &error);
    if (!ok)
    {
        RtSys::set_last_win32_error(error);
    }
    else
    {
        RtSys::set_last_win32_error(0);
    }
    return ok;
}

intptr_t Kernel32::create_file_private(vm::RtString* name, int32_t desired_access, int32_t share_mode, void* security_attributes,
                                       int32_t creation_disposition, int32_t flags_and_attributes, intptr_t template_file)
{
    return create_file_private(name != nullptr ? vm::String::get_chars_ptr(name) : nullptr, desired_access, share_mode, security_attributes,
                               creation_disposition, flags_and_attributes, template_file);
}

intptr_t Kernel32::create_file_private(const Utf16Char* name, int32_t desired_access, int32_t share_mode, void* security_attributes,
                                       int32_t creation_disposition, int32_t flags_and_attributes, intptr_t template_file)
{
    (void)security_attributes;
    (void)template_file;
    int32_t error = 0;
    intptr_t handle = os::File::open(name, creation_disposition, file_access_from_desired_access(desired_access), share_mode, flags_and_attributes, &error);
    if (handle == os::File::kInvalidHandle)
    {
        RtSys::set_last_win32_error(error);
    }
    else
    {
        RtSys::set_last_win32_error(0);
    }
    return handle;
}

bool Kernel32::delete_file_private(vm::RtString* path)
{
    return delete_file_private(path != nullptr ? vm::String::get_chars_ptr(path) : nullptr);
}

bool Kernel32::delete_file_private(const Utf16Char* path)
{
    utils::Utf8StringBuilder utf8_path;
    if (!utf16_path_to_utf8(path, utf8_path))
    {
        return false;
    }
    int result;
    do
    {
        result = ::unlink(utf8_path.get_const_chars());
    } while (result != 0 && errno == EINTR);
    if (result != 0)
    {
        set_last_error_from_errno(errno);
        return false;
    }
    RtSys::set_last_win32_error(0);
    return true;
}

bool Kernel32::get_file_attributes_ex_private(const Utf16Char* name, uint32_t file_info_level, void* file_info)
{
    if (file_info_level != 0 || file_info == nullptr)
    {
        RtSys::set_last_win32_error(kErrorInvalidParameter);
        return false;
    }
    utils::Utf8StringBuilder utf8_path;
    if (!utf16_path_to_utf8(name, utf8_path))
    {
        return false;
    }
    struct stat st;
    if (::stat(utf8_path.get_const_chars(), &st) != 0)
    {
        set_last_error_from_errno(errno);
        return false;
    }
    Win32FileAttributeData data;
    fill_attribute_data(st, utf8_path.get_const_chars(), data);
    std::memcpy(file_info, &data, sizeof(data));
    RtSys::set_last_win32_error(0);
    return true;
}

bool Kernel32::get_file_information_by_handle(intptr_t h_file, void* file_information)
{
    if (file_information == nullptr || h_file == kInvalidHandle)
    {
        RtSys::set_last_win32_error(kErrorInvalidParameter);
        return false;
    }
    struct stat st;
    if (::fstat(static_cast<int>(h_file), &st) != 0)
    {
        set_last_error_from_errno(errno);
        return false;
    }
    ByHandleFileInformation data;
    fill_handle_information(st, data);
    std::memcpy(file_information, &data, sizeof(data));
    RtSys::set_last_win32_error(0);
    return true;
}

bool Kernel32::get_file_information_by_handle_ex(intptr_t h_file, int32_t file_information_class, void* file_information, uint32_t buffer_size)
{
    if (file_information == nullptr || h_file == kInvalidHandle)
    {
        RtSys::set_last_win32_error(kErrorInvalidParameter);
        return false;
    }
    struct stat st;
    if (::fstat(static_cast<int>(h_file), &st) != 0)
    {
        set_last_error_from_errno(errno);
        return false;
    }

    switch (file_information_class)
    {
    case kFileBasicInfoClass:
    {
        FileBasicInfo info{};
        info.CreationTime = stat_birth_time(st);
        info.LastAccessTime = stat_access_time(st);
        info.LastWriteTime = stat_write_time(st);
        info.ChangeTime = stat_change_time(st);
        info.FileAttributes = attributes_from_stat(st, nullptr);
        return write_sized_file_info(file_information, buffer_size, info);
    }
    case kFileStandardInfoClass:
    {
        FileStandardInfo info{};
#if defined(__APPLE__) || defined(__linux__) || defined(__ANDROID__)
        info.AllocationSize = static_cast<int64_t>(st.st_blocks) * 512;
#else
        info.AllocationSize = static_cast<int64_t>(st.st_size);
#endif
        info.EndOfFile = static_cast<int64_t>(st.st_size);
        info.NumberOfLinks = static_cast<uint32_t>(st.st_nlink);
        info.DeletePending = 0;
        info.Directory = S_ISDIR(st.st_mode) ? 1 : 0;
        return write_sized_file_info(file_information, buffer_size, info);
    }
    case kFileAttributeTagInfoClass:
    {
        FileAttributeTagInfo info{};
        info.FileAttributes = attributes_from_stat(st, nullptr);
        info.ReparseTag = 0;
        return write_sized_file_info(file_information, buffer_size, info);
    }
    default:
        RtSys::set_last_win32_error(kErrorCallNotImplemented);
        return false;
    }
}

bool Kernel32::set_file_information_by_handle(intptr_t h_file, int32_t file_information_class, void* file_information, uint32_t buffer_size)
{
    if (file_information == nullptr || h_file == kInvalidHandle)
    {
        RtSys::set_last_win32_error(kErrorInvalidParameter);
        return false;
    }

    switch (file_information_class)
    {
    case kFileBasicInfoClass:
    {
        if (buffer_size < sizeof(FileBasicInfo))
        {
            RtSys::set_last_win32_error(kErrorInvalidParameter);
            return false;
        }
        FileBasicInfo info;
        std::memcpy(&info, file_information, sizeof(info));
        return set_file_times_by_handle(static_cast<int>(h_file), info);
    }
    case kFileEndOfFileInfoClass:
    {
        if (buffer_size < sizeof(FileEndOfFileInfo))
        {
            RtSys::set_last_win32_error(kErrorInvalidParameter);
            return false;
        }
        FileEndOfFileInfo info;
        std::memcpy(&info, file_information, sizeof(info));
        if (::ftruncate(static_cast<int>(h_file), static_cast<off_t>(info.EndOfFile)) != 0)
        {
            set_last_error_from_errno(errno);
            return false;
        }
        RtSys::set_last_win32_error(0);
        return true;
    }
    case kFileDispositionInfoClass:
    {
        if (buffer_size < sizeof(FileDispositionInfo))
        {
            RtSys::set_last_win32_error(kErrorInvalidParameter);
            return false;
        }
        RtSys::set_last_win32_error(kErrorCallNotImplemented);
        return false;
    }
    default:
        RtSys::set_last_win32_error(kErrorCallNotImplemented);
        return false;
    }
}
#endif
} // namespace platform
} // namespace leanclr

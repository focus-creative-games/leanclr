#include "kernel32.h"

#ifdef LEANCLR_PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "build_config.h"
#include "utils/rt_vector.h"
#include "vm/rt_string.h"

#include <cstring>

namespace leanclr
{
namespace platform
{

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
    (void)reserved;
    if (lib_filename == nullptr)
        return 0;
    HMODULE m = ::LoadLibraryExW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(lib_filename)), nullptr, static_cast<DWORD>(flags));
    return reinterpret_cast<intptr_t>(m);
}

bool Kernel32::close_handle(intptr_t handle)
{
    if (handle == 0 || handle == reinterpret_cast<intptr_t>(INVALID_HANDLE_VALUE))
        return false;
    return ::CloseHandle(reinterpret_cast<HANDLE>(handle)) != 0;
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
    HANDLE h = ::CreateFileW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(name)), static_cast<DWORD>(desired_access), static_cast<DWORD>(share_mode),
                             static_cast<LPSECURITY_ATTRIBUTES>(security_attributes), static_cast<DWORD>(creation_disposition),
                             static_cast<DWORD>(flags_and_attributes), reinterpret_cast<HANDLE>(template_file));
    return reinterpret_cast<intptr_t>(h);
}

bool Kernel32::delete_file_private(vm::RtString* path)
{
    if (path == nullptr)
        return false;
    return ::DeleteFileW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(path))) != 0;
}

bool Kernel32::find_next_file(intptr_t find_handle, void* find_file_data)
{
    if (find_handle == 0 || find_file_data == nullptr)
        return false;
    return ::FindNextFileW(reinterpret_cast<HANDLE>(find_handle), static_cast<LPWIN32_FIND_DATAW>(find_file_data)) != 0;
}

// int32_t Kernel32::format_message(int32_t flags, intptr_t source, uint32_t message_id, int32_t language_id, Utf16Char* buffer, int32_t buffer_chars,
//                                  intptr_t* arguments, int32_t argument_count)
// {
//     if (buffer == nullptr || buffer_chars <= 0)
//         return 0;
//     void* args_ptr = nullptr;
//     leanclr::utils::Vector<DWORD_PTR> arg_storage;
//     if (argument_count > 0 && arguments != nullptr)
//     {
//         arg_storage.resize(static_cast<size_t>(argument_count));
//         for (int32_t i = 0; i < argument_count; ++i)
//             arg_storage[static_cast<size_t>(i)] = static_cast<DWORD_PTR>(arguments[i]);
//         args_ptr = static_cast<void*>(arg_storage.begin());
//     }
//     return static_cast<int32_t>(::FormatMessageW(static_cast<DWORD>(flags), reinterpret_cast<LPCVOID>(source), static_cast<DWORD>(message_id),
//                                                  static_cast<DWORD>(language_id), reinterpret_cast<LPWSTR>(buffer), static_cast<DWORD>(buffer_chars),
//                                                  reinterpret_cast<va_list*>(args_ptr)));
// }

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
} // namespace platform
} // namespace leanclr

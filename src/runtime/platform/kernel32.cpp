#include "kernel32.h"
#include "build_config.h"
#include "vm/rt_string.h"

#ifdef LEANCLR_PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace leanclr
{
namespace platform
{

#ifndef LEANCLR_PLATFORM_WIN
static int32_t s_last_win32_error = 0;
#endif

int32_t Kernel32::get_last_win32_error()
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<int32_t>(::GetLastError());
#else
    return s_last_win32_error;
#endif
}

void Kernel32::set_last_win32_error(int32_t error)
{
#ifdef LEANCLR_PLATFORM_WIN
    ::SetLastError(static_cast<DWORD>(static_cast<uint32_t>(error)));
#else
    s_last_win32_error = error;
#endif
}

bool Kernel32::set_thread_error_mode(uint32_t mode, uint32_t& old_mode)
{
#ifdef LEANCLR_PLATFORM_WIN
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
#else
    (void)mode;
    old_mode = 0;
    return false;
#endif
}

bool Kernel32::get_file_attributes_ex_private(vm::RtString* name, uint32_t file_info_level, void* file_info)
{
#ifdef LEANCLR_PLATFORM_WIN
    return ::GetFileAttributesExW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(name)), static_cast<GET_FILEEX_INFO_LEVELS>(file_info_level),
                                  file_info) != 0;
#else
    (void)name;
    (void)file_info_level;
    (void)file_info;
    return false;
#endif
}

intptr_t Kernel32::find_first_file_ex_private(vm::RtString* lp_file_name, uint32_t f_info_level_id, void* lp_find_file_data, uint32_t f_search_op,
                                              intptr_t lp_search_filter, int32_t dw_additional_flags)
{
#ifdef LEANCLR_PLATFORM_WIN
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfileexw
    if (lp_file_name == nullptr || lp_find_file_data == nullptr)
        return reinterpret_cast<intptr_t>(INVALID_HANDLE_VALUE);

    LPVOID search_filter = (lp_search_filter != 0) ? reinterpret_cast<LPVOID>(lp_search_filter) : nullptr;
    HANDLE h = ::FindFirstFileExW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(lp_file_name)), static_cast<FINDEX_INFO_LEVELS>(f_info_level_id),
                                  lp_find_file_data, static_cast<FINDEX_SEARCH_OPS>(f_search_op), search_filter, static_cast<DWORD>(dw_additional_flags));
    return reinterpret_cast<intptr_t>(h);
#else
    (void)lp_file_name;
    (void)f_info_level_id;
    (void)lp_find_file_data;
    (void)f_search_op;
    (void)lp_search_filter;
    (void)dw_additional_flags;
    return static_cast<intptr_t>(-1);
#endif
}

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

} // namespace platform
} // namespace leanclr

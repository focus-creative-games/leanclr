#include "kernel32.h"
#include "vm/rt_string.h"

#ifdef LEANCLR_PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace leanclr
{
namespace platform
{

bool Kernel32::set_thread_error_mode(uint32_t mode, uint32_t& old_mode)
{
#ifdef LEANCLR_PLATFORM_WIN
    // SetThreadErrorMode (Vista+, kernel32): per-thread error mode; does not
    // change the process-wide default the way SetErrorMode does.
    // https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setthreaderrormode
    //
    // Use DWORD at the API boundary: Win32 typedefs DWORD as unsigned long;
    // uint32_t is often unsigned int — passing &uint32_t where LPDWORD is
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
    return ::GetFileAttributesExW(reinterpret_cast<LPCWSTR>(vm::String::get_chars_ptr(name)), static_cast<GET_FILEEX_INFO_LEVELS>(file_info_level), file_info) != 0;
#else
    (void)name;
    (void)file_info_level;
    (void)file_info;
    return false;
#endif
}

} // namespace platform
} // namespace leanclr

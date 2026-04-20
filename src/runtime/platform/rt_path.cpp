#include "rt_path.h"
#include "rt_io_error_internal.h"

#include "vm/rt_string.h"
#include "vm/settings.h"

#include <cstdlib>
#include <cstring>

#ifdef LEANCLR_PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#else
#include <errno.h>
#include <unistd.h>
#endif

namespace leanclr
{
namespace os
{

namespace
{

// Pull shared error constants and helpers into the unnamed namespace so
// call sites stay unchanged. See rt_io_error_internal.h for why these live
// in a shared header rather than per-file anonymous namespaces.
using io_error_internal::kErrorAccessDenied;
using io_error_internal::kErrorFileNotFound;
using io_error_internal::kErrorGenFailure;
using io_error_internal::kErrorSuccess;
using io_error_internal::set_error;
#ifndef LEANCLR_PLATFORM_WIN
using io_error_internal::errno_to_monoio;
#endif

} // namespace

Utf16Char Path::get_alt_directory_separator_char()
{
    return static_cast<Utf16Char>('/');
}

Utf16Char Path::get_directory_separator_char()
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<Utf16Char>('\\');
#else
    return static_cast<Utf16Char>('/');
#endif
}

Utf16Char Path::get_path_separator()
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<Utf16Char>(';');
#else
    return static_cast<Utf16Char>(':');
#endif
}

Utf16Char Path::get_volume_separator_char()
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<Utf16Char>(':');
#else
    return static_cast<Utf16Char>('/');
#endif
}

vm::RtString* Path::get_temp_path()
{
    const char* temp_dir = vm::Settings::get_temp_dir();
    if (temp_dir && temp_dir[0])
    {
        return vm::String::create_string_from_utf8cstr(temp_dir);
    }
#ifdef LEANCLR_PLATFORM_WIN
    wchar_t buffer[MAX_PATH + 1];
    DWORD len = ::GetTempPathW(MAX_PATH + 1, buffer);
    if (len > 0 && len <= MAX_PATH)
    {
        static_assert(sizeof(wchar_t) == sizeof(Utf16Char), "wchar_t must be 16-bit on Windows");
        return vm::String::create_string_from_utf16chars(reinterpret_cast<const uint16_t*>(buffer), static_cast<int32_t>(len));
    }
    return vm::String::create_string_from_utf8cstr("C:\\Temp\\");
#else
    for (const char* env_var : {"TMPDIR", "TMP", "TEMP"})
    {
        temp_dir = std::getenv(env_var);
        if (temp_dir && temp_dir[0])
        {
            break;
        }
    }
    if (!temp_dir || !temp_dir[0])
    {
#if LEANCLR_PLATFORM_ANDROID
        temp_dir = "/data/local/tmp";
#else
        temp_dir = "/tmp";
#endif
    }
    return vm::String::create_string_from_utf8cstr(temp_dir);
#endif
}

vm::RtString* Path::get_current_directory(int32_t* error)
{
    set_error(error, kErrorSuccess);

#ifdef LEANCLR_PLATFORM_WIN
    DWORD needed = ::GetCurrentDirectoryW(0, nullptr);
    if (needed == 0)
    {
        set_error(error, static_cast<int32_t>(::GetLastError()));
        return vm::String::get_empty_string();
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
            return vm::String::get_empty_string();
        }
        buffer = heap_buf;
    }
    DWORD written = ::GetCurrentDirectoryW(needed, buffer);
    vm::RtString* result;
    if (written == 0)
    {
        set_error(error, static_cast<int32_t>(::GetLastError()));
        result = vm::String::get_empty_string();
    }
    else
    {
        static_assert(sizeof(wchar_t) == sizeof(Utf16Char), "wchar_t must be 16-bit on Windows");
        result = vm::String::create_string_from_utf16chars(reinterpret_cast<const uint16_t*>(buffer), static_cast<int32_t>(written));
    }
    if (heap_buf)
        std::free(heap_buf);
    return result;
#else
    char stack_buf[1024];
    if (::getcwd(stack_buf, sizeof(stack_buf)) != nullptr)
    {
        return vm::String::create_string_from_utf8cstr(stack_buf);
    }
    set_error(error, errno_to_monoio(errno));
    return vm::String::get_empty_string();
#endif
}

} // namespace os
} // namespace leanclr

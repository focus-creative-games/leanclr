#include "extended_services.h"
#include "build_config.h"
#include "platform/rt_console.h"
#include <cstdlib>
#include <cstring>
#include <thread>
#include <cstdio>
#include <cwchar>
#if LEANCLR_PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#elif LEANCLR_PLATFORM_POSIX
#include <unistd.h>
#include <sys/utsname.h>
extern char** environ;
#endif
namespace leanclr
{
namespace host
{
namespace
{
#if LEANCLR_PLATFORM_WIN
int32_t utf8_result(const wchar_t* source, const char** value)
{
    static char utf8[16384];
    if (!source)
    {
        *value = nullptr;
        return 0;
    }
    if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, source, -1, utf8, sizeof(utf8), nullptr, nullptr))
        return static_cast<int32_t>(GetLastError());
    *value = utf8;
    return 0;
}
#endif
int32_t query_string(StringQuery query, const char* name, const char** value)
{
    static char buffer[4096];
    switch (query)
    {
    case StringQuery::EnvironmentVariable:
        if (!name)
            return 87;
#if LEANCLR_PLATFORM_WIN
        {
            wchar_t wide[4096];
            if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name, -1, wide, 4096))
                return 87;
            return utf8_result(_wgetenv(wide), value);
        }
#else
        *value = std::getenv(name);
        return 0;
#endif
    case StringQuery::UserName:
#if LEANCLR_PLATFORM_WIN
        return utf8_result(_wgetenv(L"USERNAME"), value);
#else
        *value = std::getenv("USER");
#endif
        return *value ? 0 : NotSupported;
    case StringQuery::Home:
#if LEANCLR_PLATFORM_WIN
        return utf8_result(_wgetenv(L"USERPROFILE"), value);
#else
        *value = std::getenv("HOME");
#endif
        return *value ? 0 : NotSupported;
    case StringQuery::NewLine:
#if LEANCLR_PLATFORM_WIN
        *value = "\r\n";
#else
        *value = "\n";
#endif
        return 0;
    case StringQuery::MachineName:
#if LEANCLR_PLATFORM_WIN
    {
        DWORD size = sizeof(buffer);
        if (!GetComputerNameA(buffer, &size))
            return static_cast<int32_t>(GetLastError());
    }
#elif LEANCLR_PLATFORM_POSIX
        if (gethostname(buffer, sizeof(buffer) - 1) != 0)
            return 31;
        buffer[sizeof(buffer) - 1] = 0;
#else
        return NotSupported;
#endif
        *value = buffer;
        return 0;
    case StringQuery::CurrentDirectory:
#if LEANCLR_PLATFORM_WIN
    {
        wchar_t wide[4096];
        if (!_wgetcwd(wide, 4096))
            return 31;
        return utf8_result(wide, value);
    }
#elif LEANCLR_PLATFORM_POSIX
        if (!getcwd(buffer, sizeof(buffer)))
            return 31;
#else
        return NotSupported;
#endif
        *value = buffer;
        return 0;
    case StringQuery::TempDirectory:
#if LEANCLR_PLATFORM_WIN
    {
        DWORD size = GetTempPathA(sizeof(buffer), buffer);
        if (!size || size >= sizeof(buffer))
            return 31;
        *value = buffer;
        return 0;
    }
#elif LEANCLR_PLATFORM_POSIX
        *value = std::getenv("TMPDIR");
        if (!*value || !**value)
            *value = "/tmp";
        return 0;
#else
        return NotSupported;
#endif
    case StringQuery::OsVersion:
#if LEANCLR_PLATFORM_POSIX
    {
        struct utsname info;
        if (uname(&info) != 0)
            return 31;
        std::strncpy(buffer, info.release, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = 0;
        *value = buffer;
        return 0;
    }
#elif LEANCLR_PLATFORM_WIN
    {
        using GetVersion = LONG(WINAPI*)(OSVERSIONINFOW*);
        auto module = GetModuleHandleW(L"ntdll.dll");
        auto get_version = module ? reinterpret_cast<GetVersion>(GetProcAddress(module, "RtlGetVersion")) : nullptr;
        OSVERSIONINFOW info = {};
        info.dwOSVersionInfoSize = sizeof(info);
        if (!get_version || get_version(&info) != 0)
            return NotSupported;
        std::snprintf(buffer, sizeof(buffer), "%lu.%lu.%lu", info.dwMajorVersion, info.dwMinorVersion, info.dwBuildNumber);
        *value = buffer;
        return 0;
    }
#else
        return NotSupported;
#endif
    }
    return NotSupported;
}

int32_t query_integer(IntegerQuery query, int64_t* value)
{
    switch (query)
    {
    case IntegerQuery::PlatformId:
#if LEANCLR_PLATFORM_WIN
        *value = 2;
        return 0;
#elif LEANCLR_PLATFORM_POSIX
        *value = 4;
        return 0;
#else
        return NotSupported;
#endif
    case IntegerQuery::ProcessorCount:
        *value = std::thread::hardware_concurrency();
        return *value > 0 ? 0 : NotSupported;
    case IntegerQuery::PageSize:
#if LEANCLR_PLATFORM_WIN
    {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        *value = info.dwPageSize;
        return 0;
    }
#elif LEANCLR_PLATFORM_POSIX
        *value = sysconf(_SC_PAGESIZE);
        return *value > 0 ? 0 : NotSupported;
#else
        return NotSupported;
#endif
    case IntegerQuery::Is64BitOperatingSystem:
#if LEANCLR_PLATFORM_WIN
    {
        SYSTEM_INFO info;
        GetNativeSystemInfo(&info);
        *value = info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64;
        return 0;
    }
#else
        if (sizeof(void*) == 8)
        {
            *value = 1;
            return 0;
        }
        return NotSupported;
#endif
    }
    return NotSupported;
}

int32_t set_environment(const char* name, const char* value)
{
    if (!name || !*name || std::strchr(name, '='))
        return 87;
#if LEANCLR_PLATFORM_WIN
    wchar_t wide_name[4096], wide_value[16384];
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name, -1, wide_name, 4096) ||
        !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value ? value : "", -1, wide_value, 16384))
        return 87;
    return _wputenv_s(wide_name, wide_value) == 0 ? 0 : 31;
#elif LEANCLR_PLATFORM_POSIX
    return (value ? setenv(name, value, 1) : unsetenv(name)) == 0 ? 0 : 31;
#else
    return NotSupported;
#endif
}

int32_t environment_names(void (*visitor)(const char*, void*), void* context)
{
#if LEANCLR_PLATFORM_WIN
    wchar_t* block = GetEnvironmentStringsW();
    if (!block)
        return 31;
    int32_t status = 0;
    for (wchar_t* p = block; *p; p += std::wcslen(p) + 1)
    {
        wchar_t* eq = std::wcschr(p, L'=');
        if (!eq || eq == p)
            continue;
        wchar_t saved = *eq;
        *eq = 0;
        const char* name = nullptr;
        status = utf8_result(p, &name);
        *eq = saved;
        if (status)
            break;
        visitor(name, context);
    }
    FreeEnvironmentStringsW(block);
    return status;
#elif LEANCLR_PLATFORM_POSIX
    char** entries = environ;
#else
    return NotSupported;
#endif
#if LEANCLR_PLATFORM_POSIX
    char name[4096];
    for (char** p = entries; p && *p; ++p)
    {
        const char* eq = std::strchr(*p, '=');
        if (!eq || eq == *p)
            continue;
        size_t length = static_cast<size_t>(eq - *p);
        if (length >= sizeof(name))
            return 206;
        std::memcpy(name, *p, length);
        name[length] = 0;
        visitor(name, context);
    }
    return 0;
#endif
}

void exit_process(int32_t code)
{
    std::exit(code);
}

bool setup_console(const char* keypad, const char* teardown, ConsoleState* result)
{
    platform::RtConsole::TtySetupResult tmp{};
    if (!platform::RtConsole::tty_setup(keypad, teardown, &tmp))
        return false;
    std::memcpy(result->control_chars, tmp.control_chars, sizeof(result->control_chars));
    result->columns_and_lines = platform::RtConsole::get_cols_and_lines_ptr();
    return true;
}
} // namespace

ExtendedCallbacks make_default_extended_services()
{
    ExtendedCallbacks e = {};
    e.query_string = query_string;
    e.query_integer = query_integer;
    e.set_environment = set_environment;
    e.environment_names = environment_names;
    e.exit_process = exit_process;
#if !LEANCLR_PLATFORM_PORTABLE
    e.console = {platform::RtConsole::isatty, platform::RtConsole::internal_key_available, platform::RtConsole::set_echo, platform::RtConsole::set_break,
                 setup_console};
#endif
    return e;
}
} // namespace host
} // namespace leanclr

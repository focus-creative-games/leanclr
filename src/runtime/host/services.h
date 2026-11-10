#pragma once
#include "public/export.h"
#include <cstddef>
#include <cstdint>

namespace leanclr
{
namespace host
{
// Native data only: no managed object, BCL type or OS handle structure leaks
// through this interface. Unshared loader data is released by GeneralAllocation.
struct FileData
{
    const uint8_t* data;
    size_t length;
    bool shared;
};

using FileLoader = bool (*)(const char* assembly_name, const char* extension, FileData& data);
using DebuggerLog = void (*)(int32_t level, const uint16_t* category, size_t category_length, const uint16_t* message, size_t message_length);
enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};
enum class RandomKind
{
    NonCryptographic,
    Cryptographic
};

struct FileCallbacks
{
    intptr_t (*get_stdin)();
    intptr_t (*get_stdout)();
    intptr_t (*get_stderr)();
    bool (*is_standard_handle)(intptr_t);
    intptr_t (*open)(const uint16_t*, int32_t mode, int32_t access, int32_t share, int32_t options, int32_t* error);
    bool (*close)(intptr_t, int32_t* error);
    int32_t (*read)(intptr_t, uint8_t*, int32_t count, int32_t* error);
    int32_t (*write)(intptr_t, const uint8_t*, int32_t count, int32_t* error);
    int64_t (*seek)(intptr_t, int64_t offset, int32_t origin, int32_t* error);
    int64_t (*get_length)(intptr_t, int32_t* error);
    int32_t (*get_file_type)(intptr_t, int32_t* error);
};

// Error values use the same stable I/O domain as FileCallbacks: 0 success,
// 2 not found, 50 not supported, 87 invalid argument, 259 end of enumeration.
// Strings are borrowed until the next callback in that domain; consumers copy.
constexpr int32_t NotSupported = 50;
enum class StringQuery
{
    EnvironmentVariable,
    MachineName,
    UserName,
    OsVersion,
    Home,
    TempDirectory,
    CurrentDirectory,
    NewLine
};
enum class IntegerQuery
{
    PlatformId,
    ProcessorCount,
    PageSize,
    Is64BitOperatingSystem
};

struct FileInfo
{
    int32_t flags, mode;
    uint32_t uid, gid;
    int64_t size, access_seconds, access_nanoseconds, modified_seconds, modified_nanoseconds;
    int64_t changed_seconds, changed_nanoseconds, birth_seconds, birth_nanoseconds, device, inode;
    uint32_t user_flags;
};

struct DirectoryEntry
{
    const char* name;
    int32_t length;
    int32_t type;
};

struct DirectoryCallbacks
{
    int32_t (*open)(const char*, intptr_t*);
    int32_t (*read)(intptr_t, DirectoryEntry*);
    int32_t (*close)(intptr_t);
};

struct ConsoleState
{
    uint8_t control_chars[17];
    int32_t* columns_and_lines;
};

struct ConsoleCallbacks
{
    bool (*isatty)(intptr_t);
    int32_t (*key_available)(int32_t);
    bool (*set_echo)(bool);
    bool (*set_break)(bool);
    bool (*setup)(const char*, const char*, ConsoleState*);
};

struct ExtendedCallbacks
{
    int32_t (*query_string)(StringQuery, const char* name, const char** value);
    int32_t (*query_integer)(IntegerQuery, int64_t* value);
    int32_t (*set_environment)(const char* name, const char* value); // null deletes
    int32_t (*environment_names)(void (*visitor)(const char*, void*), void* context);
    void (*exit_process)(int32_t code); // must not return
    int32_t (*stat)(const char* utf8_path, bool follow_links, FileInfo*);
    int32_t (*fstat)(intptr_t handle, FileInfo*);
    int32_t (*make_directory)(const char*, int32_t mode);
    int32_t (*remove_directory)(const char*);
    DirectoryCallbacks directories; // complete ownership domain, all or none
    ConsoleCallbacks console;
};

struct ServiceCallbacks
{
    FileLoader load_assembly;
    int64_t (*utc_ticks_100ns)();       // 100ns since Unix epoch
    int64_t (*monotonic_nanoseconds)(); // arbitrary steady epoch, never UTC
    bool (*random_bytes)(uint8_t*, size_t, RandomKind);
    void (*log)(LogLevel, const char* utf8_message);
    DebuggerLog debugger_log;
    FileCallbacks files;
    ExtendedCallbacks extended;
};

class LEANCLR_API Services
{
  public:
    // Non-null members override defaults/current callbacks before first use.
    // File callbacks must be supplied as one complete ownership domain.
    static bool configure(const ServiceCallbacks& callbacks);
    static ServiceCallbacks get_callbacks();
    static void freeze();
    static bool set_file_loader(FileLoader loader);
    static bool set_debugger_log(DebuggerLog logger);
    static bool load_assembly(const char* name, const char* extension, FileData& data);
    static int64_t utc_ticks_100ns();
    static int64_t monotonic_nanoseconds();
    static bool random_bytes(uint8_t* buffer, size_t length, RandomKind kind);
    static void log(LogLevel level, const char* message);
    static void logf(LogLevel level, const char* format, ...);
    static void debugger_log(int32_t level, const uint16_t* category, size_t category_length, const uint16_t* message, size_t message_length);
    static const FileCallbacks& files();
    static const ExtendedCallbacks& extended();
    static int32_t query_string(StringQuery query, const char* name, const char** value);
    static int32_t query_integer(IntegerQuery query, int64_t* value);
    static bool has_required_services();
};
} // namespace host
} // namespace leanclr

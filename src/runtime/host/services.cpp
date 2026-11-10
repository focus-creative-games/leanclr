#include "services.h"
#include "default_services.h"
#include <cstdarg>
#include <cstdio>

namespace leanclr
{
namespace host
{
namespace
{
bool locked = false;

ServiceCallbacks& current()
{
    // Function-local initialization avoids cross-TU static initialization order.
    static ServiceCallbacks callbacks = make_default_services();
    return callbacks;
}
} // namespace

void Services::freeze()
{
    locked = true;
}

ServiceCallbacks Services::get_callbacks()
{
    return current();
}

bool Services::configure(const ServiceCallbacks& callbacks)
{
    if (locked)
        return false;
    const auto& f = callbacks.files;
    const bool any_file = f.get_stdin || f.get_stdout || f.get_stderr || f.is_standard_handle || f.open || f.close || f.read || f.write || f.seek ||
                          f.get_length || f.get_file_type;
    const bool all_files = f.get_stdin && f.get_stdout && f.get_stderr && f.is_standard_handle && f.open && f.close && f.read && f.write && f.seek &&
                           f.get_length && f.get_file_type;
    if (any_file && !all_files)
        return false;
    auto& dst = current();
    if (callbacks.load_assembly)
        dst.load_assembly = callbacks.load_assembly;
    if (callbacks.utc_ticks_100ns)
        dst.utc_ticks_100ns = callbacks.utc_ticks_100ns;
    if (callbacks.monotonic_nanoseconds)
        dst.monotonic_nanoseconds = callbacks.monotonic_nanoseconds;
    if (callbacks.random_bytes)
        dst.random_bytes = callbacks.random_bytes;
    if (callbacks.log)
        dst.log = callbacks.log;
    if (callbacks.debugger_log)
        dst.debugger_log = callbacks.debugger_log;
    if (all_files)
        dst.files = f;
    return true;
}

bool Services::set_file_loader(FileLoader loader)
{
    if (locked)
        return false;
    current().load_assembly = loader;
    return true;
}

bool Services::set_debugger_log(DebuggerLog logger)
{
    if (locked)
        return false;
    current().debugger_log = logger;
    return true;
}

bool Services::load_assembly(const char* name, const char* extension, FileData& data)
{
    freeze();
    FileData result = {};
    auto loader = current().load_assembly;
    if (!loader || !loader(name, extension, result) || (!result.data && result.length != 0))
        return false;
    data = result;
    return true;
}

int64_t Services::utc_ticks_100ns()
{
    freeze();
    return current().utc_ticks_100ns();
}

int64_t Services::monotonic_nanoseconds()
{
    freeze();
    return current().monotonic_nanoseconds();
}

bool Services::random_bytes(uint8_t* buffer, size_t length, RandomKind kind)
{
    freeze();
    if (length == 0)
        return true;
    return buffer && current().random_bytes(buffer, length, kind);
}

void Services::log(LogLevel level, const char* message)
{
    freeze();
    current().log(level, message ? message : "");
}

void Services::logf(LogLevel level, const char* format, ...)
{
    char buffer[2048] = {};
    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = 0;
    log(level, buffer);
}

void Services::debugger_log(int32_t level, const uint16_t* category, size_t category_length, const uint16_t* message, size_t message_length)
{
    freeze();
    auto logger = current().debugger_log;
    if (logger)
        logger(level, category, category_length, message, message_length);
}

const FileCallbacks& Services::files()
{
    freeze();
    return current().files;
}
} // namespace host
} // namespace leanclr

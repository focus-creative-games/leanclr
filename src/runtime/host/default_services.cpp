#include "default_services.h"
#include "extended_services.h"
#include "platform/rt_file.h"
#include "platform/rt_time.h"
#include "utils/string_builder.h"
#include <cstdio>
#include <climits>
#if LEANCLR_PLATFORM_WIN
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#endif

namespace leanclr
{
namespace host
{
namespace
{
void default_log(LogLevel level, const char* message)
{
    const char* names[] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};
    std::fprintf(stdout, "[%s] %s\n", names[static_cast<unsigned>(level)], message);
}

void default_debugger_log(int32_t level, const uint16_t* category, size_t category_length, const uint16_t* message, size_t message_length)
{
    utils::Utf8StringBuilder buffer;
    buffer.append_char('[');
    buffer.append_u32(static_cast<uint32_t>(level));
    buffer.append_cstr("] ");
    if (category && category_length)
    {
        buffer.append_utf16_str(category, category_length);
        buffer.append_cstr(" - ");
    }
    if (message && message_length)
        buffer.append_utf16_str(message, message_length);
    std::printf("%s\n", buffer.get_const_chars());
}

bool default_random(uint8_t* buffer, size_t length, RandomKind)
{
#if LEANCLR_PLATFORM_WIN
    while (length)
    {
        ULONG part = static_cast<ULONG>(length > ULONG_MAX ? ULONG_MAX : length);
        if (BCryptGenRandom(nullptr, buffer, part, BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0)
            return false;
        buffer += part;
        length -= part;
    }
    return true;
#else
    // Native POSIX CSPRNG. Sandboxed/embedded hosts without this device install
    // their own callback (for example Web Crypto); never fall back to a PRNG.
    FILE* file = std::fopen("/dev/urandom", "rb");
    if (!file)
        return false;
    const bool ok = std::fread(buffer, 1, length, file) == length;
    std::fclose(file);
    return ok;
#endif
}
} // namespace

ServiceCallbacks make_default_services()
{
    ServiceCallbacks callbacks = {};
    callbacks.utc_ticks_100ns = os::Time::get_ticks_100nanos;
    callbacks.monotonic_nanoseconds = os::Time::get_current_time_nanos;
    callbacks.random_bytes = default_random;
    callbacks.log = default_log;
    callbacks.debugger_log = default_debugger_log;
    callbacks.files = {os::File::get_stdin, os::File::get_stdout, os::File::get_stderr, os::File::is_standard_handle, os::File::open,         os::File::close,
                       os::File::read,      os::File::write,      os::File::seek,       os::File::get_length,         os::File::get_file_type};
    callbacks.extended = make_default_extended_services();
    return callbacks;
}
} // namespace host
} // namespace leanclr

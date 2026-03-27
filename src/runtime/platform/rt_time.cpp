// C++11 chrono-based cross-platform implementation
#include <chrono>
#include "rt_time.h"

using namespace std::chrono;

namespace leanclr
{
namespace os
{

// Returns milliseconds since steady_clock epoch (monotonic, not wall clock)
int32_t Time::get_tick_count()
{
    auto now = steady_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    // Wrap to 32-bit for compatibility (like Windows GetTickCount)
    return static_cast<int32_t>(ms & 0xFFFFFFFF);
}

// Returns milliseconds since Unix epoch (1970-01-01)
int64_t Time::get_current_time_millis()
{
    auto now = system_clock::now();
    return duration_cast<milliseconds>(now.time_since_epoch()).count();
}

// Returns nanoseconds since steady_clock epoch
int64_t Time::get_current_time_nanos()
{
    auto now = steady_clock::now();
    return duration_cast<nanoseconds>(now.time_since_epoch()).count();
}

// Returns 100-nanosecond ticks since Unix epoch
int64_t Time::get_ticks_100nanos()
{
    auto now = system_clock::now();
    using ticks_100ns = duration<int64_t, std::ratio<1, 10000000>>;
    return duration_cast<ticks_100ns>(now.time_since_epoch()).count();
}

// Returns Windows FILETIME (100-nanosecond intervals since 1601-01-01)
int64_t Time::get_system_time_as_file_time()
{
    // FILETIME epoch: 1601-01-01T00:00:00Z
    // system_clock epoch: 1970-01-01T00:00:00Z
    constexpr int64_t WINDOWS_TICK = 10000000LL;         // 1 tick = 100ns
    constexpr int64_t SEC_TO_UNIX_EPOCH = 11644473600LL; // seconds between 1601-01-01 and 1970-01-01
    auto now = system_clock::now();
    using ticks_100ns = duration<int64_t, std::ratio<1, 10000000>>;
    int64_t ticks = duration_cast<ticks_100ns>(now.time_since_epoch()).count();
    return ticks + SEC_TO_UNIX_EPOCH * WINDOWS_TICK;
}
} // namespace os
} // namespace leanclr

#pragma once
#include "host/services.h"
#include <algorithm>
#include <cstring>
#include <cstdio>

// Deterministic, memory-backed host used by both native and managed integration
// tests. It has no filesystem dependency and owns every handle in its table.
namespace FakeHost
{
struct State
{
    int64_t utc = 1234567890;
    int64_t monotonic = INT64_C(987654321000);
    uint8_t bytes[32] = {65, 66, 67};
    size_t length = 3;
    size_t position = 0;
    int opens = 0, closes = 0, reads = 0, writes = 0, random_calls = 0, logs = 0;
    bool fail_random = false;
    char last_log[256] = {};
};

inline State& state()
{
    static State value;
    return value;
}

inline int64_t utc()
{
    return state().utc;
}

inline int64_t monotonic()
{
    return state().monotonic;
}

inline intptr_t stdin_handle()
{
    return 1;
}

inline intptr_t stdout_handle()
{
    return 2;
}

inline intptr_t stderr_handle()
{
    return 3;
}

inline bool is_standard(intptr_t h)
{
    return h >= 1 && h <= 3;
}

inline bool valid(intptr_t h, int32_t* error)
{
    if (error)
        *error = h == 42 ? 0 : 6;
    return h == 42;
}

inline intptr_t open(const uint16_t*, int32_t, int32_t, int32_t, int32_t, int32_t* error)
{
    ++state().opens;
    state().position = 0;
    if (error)
        *error = 0;
    return 42;
}

inline bool close(intptr_t h, int32_t* error)
{
    if (!valid(h, error))
        return false;
    ++state().closes;
    return true;
}

inline int32_t read(intptr_t h, uint8_t* buffer, int32_t count, int32_t* error)
{
    if (!valid(h, error) || count < 0)
        return -1;
    auto& s = state();
    ++s.reads;
    size_t n = std::min(static_cast<size_t>(count), s.position >= s.length ? size_t(0) : s.length - s.position);
    std::memcpy(buffer, s.bytes + s.position, n);
    s.position += n;
    return static_cast<int32_t>(n);
}

inline int32_t write(intptr_t h, const uint8_t* buffer, int32_t count, int32_t* error)
{
    if (!valid(h, error) || count < 0)
        return -1;
    auto& s = state();
    ++s.writes;
    size_t n = std::min(static_cast<size_t>(count), sizeof(s.bytes) - s.position);
    std::memcpy(s.bytes + s.position, buffer, n);
    s.position += n;
    s.length = std::max(s.length, s.position);
    return static_cast<int32_t>(n);
}

inline int64_t seek(intptr_t h, int64_t offset, int32_t origin, int32_t* error)
{
    if (!valid(h, error))
        return -1;
    auto& s = state();
    int64_t pos = offset + (origin == 1 ? static_cast<int64_t>(s.position) : origin == 2 ? static_cast<int64_t>(s.length) : 0);
    if (pos < 0 || pos > static_cast<int64_t>(sizeof(s.bytes)))
    {
        if (error)
            *error = 87;
        return -1;
    }
    s.position = static_cast<size_t>(pos);
    return pos;
}

inline int64_t length(intptr_t h, int32_t* error)
{
    return valid(h, error) ? static_cast<int64_t>(state().length) : -1;
}

inline int32_t file_type(intptr_t h, int32_t* error)
{
    return valid(h, error) ? 1 : 0;
}

inline bool random(uint8_t* data, size_t count, leanclr::host::RandomKind)
{
    auto& s = state();
    ++s.random_calls;
    if (s.fail_random)
        return false;
    for (size_t i = 0; i < count; ++i)
        data[i] = static_cast<uint8_t>(i + 17);
    return true;
}

inline void log(leanclr::host::LogLevel level, const char* text)
{
    if (level == leanclr::host::LogLevel::Fatal || level == leanclr::host::LogLevel::Error)
        std::fprintf(stderr, "%s\n", text);
    ++state().logs;
    std::strncpy(state().last_log, text, sizeof(state().last_log) - 1);
}

inline leanclr::host::ServiceCallbacks callbacks()
{
    leanclr::host::ServiceCallbacks result = {};
    result.utc_ticks_100ns = utc;
    result.monotonic_nanoseconds = monotonic;
    result.random_bytes = random;
    result.log = log;
    result.files = {stdin_handle, stdout_handle, stderr_handle, is_standard, open, close, read, write, seek, length, file_type};
    return result;
}
} // namespace FakeHost

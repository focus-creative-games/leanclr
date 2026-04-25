#pragma once

// Platform-internal header. Provides System.IO.MonoIOError constants and
// error-code translation helpers shared between rt_file.cpp, rt_path.cpp and
// any other platform/ source files that surface MonoIOError values.
//
// Design notes:
//  * Constants are declared `static constexpr`. In C++11 namespace-scope
//    constexpr variables have external linkage by default, which would cause
//    ODR violations when the header is included from multiple TUs. `static`
//    drops them to internal linkage so each TU gets its own private copy.
//    (In C++17 one could write `inline constexpr` instead; when the project
//    moves to C++17 this can be simplified.)
//  * Helper functions are declared `inline` (no `static`). `inline` alone is
//    the idiomatic C++ way to put a function definition in a header: the
//    linker is allowed to merge identical copies across TUs, the function's
//    address is consistent, and il2cpp source-merging is handled correctly.
//  * Do NOT include this header from outside the platform/ directory.

#include <cstdint>

#include "core/rt_base.h"

#ifndef LEANCLR_PLATFORM_WIN
#include <errno.h>
#endif

namespace leanclr
{
namespace os
{
namespace io_error_internal
{

// System.IO.MonoIOError values (see "Design notes" above for why `static`).
static constexpr int32_t kErrorSuccess = 0;
static constexpr int32_t kErrorFileNotFound = 2;
static constexpr int32_t kErrorPathNotFound = 3;
static constexpr int32_t kErrorAccessDenied = 5;
static constexpr int32_t kErrorInvalidHandle = 6;
static constexpr int32_t kErrorGenFailure = 31;
static constexpr int32_t kErrorHandleDiskFull = 39;
static constexpr int32_t kErrorFileExists = 80;
static constexpr int32_t kErrorInvalidParameter = 87;
static constexpr int32_t kErrorDirectory = 267;

// Writes `value` into `*error` when `error` is non-null.
inline void set_error(int32_t* error, int32_t value)
{
    if (error)
        *error = value;
}

#ifdef LEANCLR_PLATFORM_WIN

// The System.IO.MonoIOError enum mirrors Win32 error codes, so no extra
// mapping is required; we still provide the helper for call-site clarity.
// `unsigned long` is used instead of DWORD to avoid pulling in <windows.h>.
inline int32_t win32_error_to_monoio(unsigned long code)
{
    return static_cast<int32_t>(code);
}

#else

inline int32_t errno_to_monoio(int err)
{
    switch (err)
    {
    case 0:
        return kErrorSuccess;
    case ENOENT:
        return kErrorFileNotFound;
    case ENOTDIR:
        return kErrorPathNotFound;
    case EACCES:
    case EPERM:
        return kErrorAccessDenied;
    case EBADF:
        return kErrorInvalidHandle;
    case EEXIST:
        return kErrorFileExists;
    case EINVAL:
        return kErrorInvalidParameter;
    case ENOSPC:
        return kErrorHandleDiskFull;
    case EISDIR:
        return kErrorDirectory;
    default:
        return kErrorGenFailure;
    }
}

#endif

} // namespace io_error_internal
} // namespace os
} // namespace leanclr

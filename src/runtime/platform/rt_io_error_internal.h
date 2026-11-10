#pragma once

// Native host I/O status codes, using Win32-compatible values on all targets.
// BCL adapters translate their own error vocabulary to/from this contract (the
// Mono adapter currently uses the identical numeric values). Shared by the
// default native provider and native-to-managed adapter bridges.

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

inline int32_t errno_to_io_error(int err)
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

#pragma once

#include <cstdint>

#include "core/rt_base.h"

namespace leanclr
{
namespace os
{

// Cross-platform file I/O primitives used by the System.IO.MonoIO icalls.
//
// File handles are represented as `intptr_t`:
//  - On Windows, the value is a Win32 HANDLE.
//  - On POSIX, the value is an `int` file descriptor.
// The special value `kInvalidHandle` (-1) denotes an invalid handle.
//
// All error codes reported via the `error` out-parameter use the values of
// System.IO.MonoIOError (ERROR_SUCCESS = 0, ERROR_FILE_NOT_FOUND = 2, ...).
class File
{
  public:
    static constexpr intptr_t kInvalidHandle = static_cast<intptr_t>(-1);

    // System.IO.MonoFileType values (match Win32 FILE_TYPE_* constants).
    enum FileType : int32_t
    {
        FileTypeUnknown = 0,
        FileTypeDisk = 1,
        FileTypeChar = 2,
        FileTypePipe = 3,
    };

    // Standard stream handles
    static intptr_t get_stdin();
    static intptr_t get_stdout();
    static intptr_t get_stderr();
    static bool is_standard_handle(intptr_t handle);

    // Open / close
    //  - `filename` is a null-terminated UTF-16 string.
    //  - `mode`   uses System.IO.FileMode values
    //  - `access` uses System.IO.FileAccess values
    //  - `share`  uses System.IO.FileShare bit flags (may be ignored on POSIX)
    //  - `options` uses System.IO.FileOptions bit flags (may be ignored on POSIX)
    static intptr_t open(const Utf16Char* filename, int32_t mode, int32_t access, int32_t share, int32_t options, int32_t* error);
    static bool close(intptr_t handle, int32_t* error);

    // Bulk I/O. Return the number of bytes transferred, or -1 on error.
    static int32_t read(intptr_t handle, uint8_t* buffer, int32_t count, int32_t* error);
    static int32_t write(intptr_t handle, const uint8_t* buffer, int32_t count, int32_t* error);

    // Seek (origin uses System.IO.SeekOrigin values: 0=Begin, 1=Current, 2=End).
    // Returns the new absolute position, or -1 on error.
    static int64_t seek(intptr_t handle, int64_t offset, int32_t origin, int32_t* error);

    // Length / type introspection
    static int64_t get_length(intptr_t handle, int32_t* error);
    static int32_t get_file_type(intptr_t handle, int32_t* error);
};

} // namespace os
} // namespace leanclr

#include "rt_portable_io.h"

#include "build_config.h"

#if LEANCLR_PLATFORM_PORTABLE

#include "rt_io_error_internal.h"

#include "utils/string_builder.h"
#include "utils/string_util.h"
#include "vm/rt_string.h"
#include "vm/settings.h"

#include <cerrno>
#include <cstdio>
#include <cstring>

namespace leanclr
{
namespace os
{
namespace portable_io
{

namespace
{

using io_error_internal::kErrorAccessDenied;
using io_error_internal::kErrorFileExists;
using io_error_internal::kErrorFileNotFound;
using io_error_internal::kErrorGenFailure;
using io_error_internal::kErrorInvalidHandle;
using io_error_internal::kErrorInvalidParameter;
using io_error_internal::kErrorSuccess;
using io_error_internal::errno_to_monoio;
using io_error_internal::set_error;

constexpr int32_t kFileModeCreateNew = 1;
constexpr int32_t kFileModeCreate = 2;
constexpr int32_t kFileModeOpen = 3;
constexpr int32_t kFileModeOpenOrCreate = 4;
constexpr int32_t kFileModeTruncate = 5;
constexpr int32_t kFileModeAppend = 6;

constexpr int32_t kFileAccessRead = 1;
constexpr int32_t kFileAccessWrite = 2;
constexpr int32_t kFileAccessReadWrite = 3;

FILE* handle_to_file(intptr_t handle)
{
    return reinterpret_cast<FILE*>(handle);
}

bool is_stdio_file(FILE* file)
{
    return file == stdin || file == stdout || file == stderr;
}

const char* mode_for_open(int32_t mode, int32_t access)
{
    switch (access)
    {
    case kFileAccessRead:
        switch (mode)
        {
        case kFileModeCreateNew:
        case kFileModeCreate:
        case kFileModeTruncate:
        case kFileModeAppend:
            return nullptr;
        default:
            return "rb";
        }
    case kFileAccessWrite:
        switch (mode)
        {
        case kFileModeCreateNew:
            return "wbx";
        case kFileModeCreate:
        case kFileModeTruncate:
            return "wb";
        case kFileModeOpen:
            return "r+b";
        case kFileModeOpenOrCreate:
            return "w+b";
        case kFileModeAppend:
            return "ab";
        default:
            return nullptr;
        }
    case kFileAccessReadWrite:
        switch (mode)
        {
        case kFileModeCreateNew:
            return "w+bx";
        case kFileModeCreate:
        case kFileModeTruncate:
            return "w+b";
        case kFileModeOpen:
            return "r+b";
        case kFileModeOpenOrCreate:
            return "w+b";
        case kFileModeAppend:
            return "a+b";
        default:
            return nullptr;
        }
    default:
        return nullptr;
    }
}

} // namespace

intptr_t file_get_stdin()
{
    return reinterpret_cast<intptr_t>(stdin);
}

intptr_t file_get_stdout()
{
    return reinterpret_cast<intptr_t>(stdout);
}

intptr_t file_get_stderr()
{
    return reinterpret_cast<intptr_t>(stderr);
}

bool file_is_standard_handle(intptr_t handle)
{
    FILE* file = handle_to_file(handle);
    return is_stdio_file(file);
}

intptr_t file_open(const Utf16Char* filename, int32_t mode, int32_t access, int32_t share, int32_t options, int32_t* error)
{
    set_error(error, kErrorSuccess);
    (void)share;
    (void)options;

    if (filename == nullptr)
    {
        set_error(error, kErrorInvalidParameter);
        return File::kInvalidHandle;
    }

    const char* open_mode = mode_for_open(mode, access);
    if (open_mode == nullptr)
    {
        set_error(error, kErrorInvalidParameter);
        return File::kInvalidHandle;
    }

    utils::Utf8StringBuilder path(filename, static_cast<size_t>(utils::StringUtil::get_utf16chars_length(filename)));
    FILE* file = std::fopen(path.get_const_chars(), open_mode);
    if (file == nullptr)
    {
        switch (errno)
        {
        case EEXIST:
            set_error(error, kErrorFileExists);
            break;
        case ENOENT:
            set_error(error, kErrorFileNotFound);
            break;
        case EACCES:
            set_error(error, kErrorAccessDenied);
            break;
        default:
            set_error(error, errno_to_monoio(errno));
            break;
        }
        return File::kInvalidHandle;
    }

    if (mode == kFileModeAppend)
    {
        if (std::fseek(file, 0, SEEK_END) != 0)
        {
            set_error(error, errno_to_monoio(errno));
            std::fclose(file);
            return File::kInvalidHandle;
        }
    }

    return reinterpret_cast<intptr_t>(file);
}

bool file_close(intptr_t handle, int32_t* error)
{
    set_error(error, kErrorSuccess);

    FILE* file = handle_to_file(handle);
    if (file == nullptr)
    {
        set_error(error, kErrorInvalidHandle);
        return false;
    }

    if (is_stdio_file(file))
    {
        return true;
    }

    if (std::fclose(file) != 0)
    {
        set_error(error, errno_to_monoio(errno));
        return false;
    }
    return true;
}

int32_t file_read(intptr_t handle, uint8_t* buffer, int32_t count, int32_t* error)
{
    set_error(error, kErrorSuccess);

    if (count < 0 || buffer == nullptr)
    {
        set_error(error, kErrorInvalidParameter);
        return -1;
    }
    if (count == 0)
    {
        return 0;
    }

    FILE* file = handle_to_file(handle);
    if (file == nullptr)
    {
        set_error(error, kErrorInvalidHandle);
        return -1;
    }

    const size_t n = std::fread(buffer, 1, static_cast<size_t>(count), file);
    if (n == 0 && std::ferror(file))
    {
        set_error(error, errno_to_monoio(errno));
        return -1;
    }
    return static_cast<int32_t>(n);
}

int32_t file_write(intptr_t handle, const uint8_t* buffer, int32_t count, int32_t* error)
{
    set_error(error, kErrorSuccess);

    if (count < 0 || buffer == nullptr)
    {
        set_error(error, kErrorInvalidParameter);
        return -1;
    }
    if (count == 0)
    {
        return 0;
    }

    FILE* file = handle_to_file(handle);
    if (file == nullptr)
    {
        set_error(error, kErrorInvalidHandle);
        return -1;
    }

    const size_t n = std::fwrite(buffer, 1, static_cast<size_t>(count), file);
    if (n < static_cast<size_t>(count) && std::ferror(file))
    {
        set_error(error, errno_to_monoio(errno));
        return n > 0 ? static_cast<int32_t>(n) : -1;
    }
    return static_cast<int32_t>(n);
}

int64_t file_seek(intptr_t handle, int64_t offset, int32_t origin, int32_t* error)
{
    set_error(error, kErrorSuccess);

    int whence = 0;
    switch (origin)
    {
    case 0:
        whence = SEEK_SET;
        break;
    case 1:
        whence = SEEK_CUR;
        break;
    case 2:
        whence = SEEK_END;
        break;
    default:
        set_error(error, kErrorInvalidParameter);
        return -1;
    }

    FILE* file = handle_to_file(handle);
    if (file == nullptr)
    {
        set_error(error, kErrorInvalidHandle);
        return -1;
    }

    if (std::fseek(file, static_cast<long>(offset), whence) != 0)
    {
        set_error(error, errno_to_monoio(errno));
        return -1;
    }

    const long pos = std::ftell(file);
    if (pos < 0)
    {
        set_error(error, errno_to_monoio(errno));
        return -1;
    }
    return static_cast<int64_t>(pos);
}

int64_t file_get_length(intptr_t handle, int32_t* error)
{
    set_error(error, kErrorSuccess);

    FILE* file = handle_to_file(handle);
    if (file == nullptr)
    {
        set_error(error, kErrorInvalidHandle);
        return 0;
    }

    const long current = std::ftell(file);
    if (current < 0)
    {
        set_error(error, errno_to_monoio(errno));
        return 0;
    }

    if (std::fseek(file, 0, SEEK_END) != 0)
    {
        set_error(error, errno_to_monoio(errno));
        return 0;
    }

    const long end = std::ftell(file);
    if (end < 0)
    {
        set_error(error, errno_to_monoio(errno));
        return 0;
    }

    if (std::fseek(file, current, SEEK_SET) != 0)
    {
        set_error(error, errno_to_monoio(errno));
        return 0;
    }

    return static_cast<int64_t>(end);
}

int32_t file_get_file_type(intptr_t handle, int32_t* error)
{
    set_error(error, kErrorSuccess);

    FILE* file = handle_to_file(handle);
    if (file == nullptr)
    {
        set_error(error, kErrorInvalidHandle);
        return File::FileTypeUnknown;
    }

    if (is_stdio_file(file))
    {
        return File::FileTypeChar;
    }
    return File::FileTypeDisk;
}

vm::RtString* path_get_temp_path()
{
    const char* temp_dir = vm::Settings::get_temp_dir();
    if (temp_dir && temp_dir[0])
    {
        return vm::String::create_string_from_utf8cstr(temp_dir);
    }
    return vm::String::create_string_from_utf8cstr("tmp");
}

vm::RtString* path_get_current_directory(int32_t* error)
{
    set_error(error, kErrorSuccess);
    return vm::String::create_string_from_utf8cstr(".");
}

} // namespace portable_io
} // namespace os
} // namespace leanclr

#endif // LEANCLR_PLATFORM_PORTABLE

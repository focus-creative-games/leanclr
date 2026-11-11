#include "bcl/mono/platform/rt_path.h"
#include "platform/rt_io_error_internal.h"

#include "vm/rt_string.h"
#include "vm/settings.h"

#include "bcl/mono/host_result.h"

namespace leanclr
{
namespace os
{

namespace
{

// Pull shared error constants and helpers into the unnamed namespace so
// call sites stay unchanged. See rt_io_error_internal.h for why these live
// in a shared header rather than per-file anonymous namespaces.
using io_error_internal::kErrorAccessDenied;
using io_error_internal::kErrorFileNotFound;
using io_error_internal::kErrorGenFailure;
using io_error_internal::kErrorSuccess;
using io_error_internal::set_error;
#if LEANCLR_PLATFORM_POSIX
using io_error_internal::errno_to_io_error;
#endif

} // namespace

Utf16Char Path::get_alt_directory_separator_char()
{
    return static_cast<Utf16Char>('/');
}

Utf16Char Path::get_directory_separator_char()
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<Utf16Char>('\\');
#else
    return static_cast<Utf16Char>('/');
#endif
}

Utf16Char Path::get_path_separator()
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<Utf16Char>(';');
#else
    return static_cast<Utf16Char>(':');
#endif
}

Utf16Char Path::get_volume_separator_char()
{
#ifdef LEANCLR_PLATFORM_WIN
    return static_cast<Utf16Char>(':');
#else
    return static_cast<Utf16Char>('/');
#endif
}

RtResult<vm::RtString*> Path::get_temp_path()
{
    const char* path = vm::Settings::get_temp_dir();
    if (path && *path)
        return vm::String::create_string_from_utf8cstr(path);
    int32_t error = host::Services::query_string(host::StringQuery::TempDirectory, nullptr, &path);
    if (error)
        return bcl::host_error(error);
    if (!path)
        return RtErr::InvalidOperation;
    return vm::String::create_string_from_utf8cstr(path);
}

vm::RtString* Path::get_current_directory(int32_t* error)
{
    const char* path = nullptr;
    int32_t status = host::Services::query_string(host::StringQuery::CurrentDirectory, nullptr, &path);
    if (!status && !path)
        status = 31;
    set_error(error, status);
    return status ? vm::String::get_empty_string() : vm::String::create_string_from_utf8cstr(path);
}
} // namespace os
} // namespace leanclr

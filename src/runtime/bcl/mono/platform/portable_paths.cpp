#include "portable_paths.h"
#include "build_config.h"
#if LEANCLR_PLATFORM_PORTABLE
#include "vm/settings.h"
#include "vm/rt_string.h"
#include "platform/rt_io_error_internal.h"

namespace leanclr
{
namespace os
{
namespace portable_io
{
using io_error_internal::kErrorSuccess;
using io_error_internal::set_error;

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
#endif

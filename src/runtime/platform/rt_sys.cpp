#include "rt_sys.h"

#include "build_config.h"
#include "vm/rt_string.h"
#include "utils/string_builder.h"
#include "utils/string_util.h"

#ifdef LEANCLR_PLATFORM_POSIX
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace leanclr
{
namespace platform
{
namespace
{
#ifdef LEANCLR_PLATFORM_POSIX
static void rt_string_to_utf8_path(vm::RtString* str, utils::StringBuilder& sb)
{
    if (str)
    {
        utils::StringUtil::utf16_to_utf8(vm::String::get_chars_ptr(str), static_cast<size_t>(vm::String::get_length(str)), sb);
    }
    sb.sure_null_terminator_but_not_append();
}
#endif
} // namespace

int32_t RtSys::ch_mod(vm::RtString* path, int32_t mode)
{
#ifdef LEANCLR_PLATFORM_POSIX
    utils::StringBuilder path_utf8;
    rt_string_to_utf8_path(path, path_utf8);
    int32_t result = 0;
    while ((result = ::chmod(path_utf8.as_cstr(), static_cast<mode_t>(mode))) < 0 && errno == EINTR)
        ;
    return result;
#else
    (void)path;
    (void)mode;
    return -1;
#endif
}

int32_t RtSys::mk_dir(vm::RtString* path, int32_t mode)
{
#ifdef LEANCLR_PLATFORM_POSIX
    utils::StringBuilder path_utf8;
    rt_string_to_utf8_path(path, path_utf8);
    int32_t result = 0;
    while ((result = ::mkdir(path_utf8.as_cstr(), static_cast<mode_t>(mode))) < 0 && errno == EINTR)
        ;
    return result;
#else
    (void)path;
    (void)mode;
    return -1;
#endif
}

int32_t RtSys::rename(vm::RtString* old_path, vm::RtString* new_path)
{
#ifdef LEANCLR_PLATFORM_POSIX
    utils::StringBuilder old_path_utf8;
    utils::StringBuilder new_path_utf8;
    rt_string_to_utf8_path(old_path, old_path_utf8);
    rt_string_to_utf8_path(new_path, new_path_utf8);
    int32_t result = 0;
    while ((result = ::rename(old_path_utf8.as_cstr(), new_path_utf8.as_cstr())) < 0 && errno == EINTR)
        ;
    return result;
#else
    (void)old_path;
    (void)new_path;
    return -1;
#endif
}

int32_t RtSys::rm_dir(vm::RtString* path)
{
#ifdef LEANCLR_PLATFORM_POSIX
    utils::StringBuilder path_utf8;
    rt_string_to_utf8_path(path, path_utf8);
    int32_t result = 0;
    while ((result = ::rmdir(path_utf8.as_cstr())) < 0 && errno == EINTR)
        ;
    return result;
#else
    (void)path;
    return -1;
#endif
}

int32_t RtSys::unlink(vm::RtString* path)
{
#ifdef LEANCLR_PLATFORM_POSIX
    utils::StringBuilder path_utf8;
    rt_string_to_utf8_path(path, path_utf8);
    int32_t result = 0;
    while ((result = ::unlink(path_utf8.as_cstr())) < 0 && errno == EINTR)
        ;
    return result;
#else
    (void)path;
    return -1;
#endif
}

} // namespace platform
} // namespace leanclr

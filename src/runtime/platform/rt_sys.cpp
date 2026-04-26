#include "rt_sys.h"

#include "build_config.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"
#include "utils/string_builder.h"
#include "utils/string_util.h"

#ifdef LEANCLR_PLATFORM_POSIX
#include <dirent.h>
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
struct ManagedDirectoryEntry
{
    char* Name;
    int32_t NameLength;
    int32_t InodeType;
};

struct PosixDirWrapper
{
    DIR* dir;
    ManagedDirectoryEntry* entries;
    size_t cur_index;
    size_t num_entries;
};

static int compare_directory_entry_by_name(const void* a, const void* b)
{
    const ManagedDirectoryEntry* e1 = static_cast<const ManagedDirectoryEntry*>(a);
    const ManagedDirectoryEntry* e2 = static_cast<const ManagedDirectoryEntry*>(b);
    if (e1->Name == e2->Name)
        return 0;
    if (e1->Name == nullptr)
        return 1;
    if (e2->Name == nullptr)
        return -1;
    return std::strcmp(e1->Name, e2->Name);
}

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

intptr_t RtSys::open_dir(vm::RtString* path)
{
#ifdef LEANCLR_PLATFORM_POSIX
    utils::StringBuilder path_utf8;
    rt_string_to_utf8_path(path, path_utf8);
    DIR* dir = ::opendir(path_utf8.as_cstr());
    if (dir == nullptr)
        return 0;

    PosixDirWrapper* wrapper = static_cast<PosixDirWrapper*>(std::malloc(sizeof(PosixDirWrapper)));
    if (wrapper == nullptr)
    {
        ::closedir(dir);
        errno = ENOMEM;
        return 0;
    }
    wrapper->dir = dir;
    wrapper->entries = nullptr;
    wrapper->cur_index = 0;
    wrapper->num_entries = 0;
    return reinterpret_cast<intptr_t>(wrapper);
#else
    (void)path;
    return 0;
#endif
}

int32_t RtSys::close_dir(intptr_t dir)
{
#ifdef LEANCLR_PLATFORM_POSIX
    PosixDirWrapper* wrapper = reinterpret_cast<PosixDirWrapper*>(dir);
    if (wrapper == nullptr)
    {
        errno = EINVAL;
        return -1;
    }

    int32_t ret = ::closedir(wrapper->dir);
    if (wrapper->entries != nullptr)
    {
        for (size_t i = 0; i < wrapper->num_entries; ++i)
        {
            std::free(wrapper->entries[i].Name);
        }
        std::free(wrapper->entries);
    }
    std::free(wrapper);
    return ret;
#else
    (void)dir;
    return -1;
#endif
}

int32_t RtSys::get_read_dir_r_buffer_size()
{
#ifdef LEANCLR_PLATFORM_POSIX
    return 0;
#else
    return 0;
#endif
}

int32_t RtSys::read_dir_r(intptr_t dir, uint8_t* buffer, int32_t buffer_size, void* output_entry)
{
#ifdef LEANCLR_PLATFORM_POSIX
    (void)buffer;
    (void)buffer_size;
    PosixDirWrapper* wrapper = reinterpret_cast<PosixDirWrapper*>(dir);
    ManagedDirectoryEntry* output = static_cast<ManagedDirectoryEntry*>(output_entry);
    if (wrapper == nullptr || output == nullptr || wrapper->dir == nullptr)
    {
        errno = EINVAL;
        if (output != nullptr)
            std::memset(output, 0, sizeof(*output));
        return EINVAL;
    }

    if (wrapper->entries == nullptr)
    {
        errno = 0;
        size_t num_entries = 0;
        dirent* entry = nullptr;
        while ((entry = ::readdir(wrapper->dir)) != nullptr)
            ++num_entries;

        if (num_entries > 0)
        {
            ManagedDirectoryEntry* entries = static_cast<ManagedDirectoryEntry*>(std::calloc(num_entries, sizeof(ManagedDirectoryEntry)));
            if (entries == nullptr)
            {
                std::memset(output, 0, sizeof(*output));
                errno = ENOMEM;
                return ENOMEM;
            }

            ::rewinddir(wrapper->dir);
            size_t index = 0;
            while ((entry = ::readdir(wrapper->dir)) != nullptr && index < num_entries)
            {
                entries[index].Name = const_cast<char*>(utils::StringUtil::strdup(entry->d_name));
#if defined(_DIRENT_HAVE_D_NAMLEN)
                entries[index].NameLength = static_cast<int32_t>(entry->d_namlen);
#else
                entries[index].NameLength = -1;
#endif
#if defined(DT_UNKNOWN)
                entries[index].InodeType = static_cast<int32_t>(entry->d_type);
#else
                entries[index].InodeType = 0;
#endif
                ++index;
            }
            std::qsort(entries, num_entries, sizeof(ManagedDirectoryEntry), compare_directory_entry_by_name);
            wrapper->entries = entries;
            wrapper->num_entries = index;
            wrapper->cur_index = 0;
        }
    }

    if (wrapper->cur_index < wrapper->num_entries)
    {
        *output = wrapper->entries[wrapper->cur_index++];
        return 0;
    }

    std::memset(output, 0, sizeof(*output));
    if (errno != 0)
        return errno;
    return -1;
#else
    (void)dir;
    (void)buffer;
    (void)buffer_size;
    (void)output_entry;
    return -1;
#endif
}

int32_t RtSys::read_link(vm::RtString* path, vm::RtArray* buffer, int32_t buffer_size)
{
#ifdef LEANCLR_PLATFORM_POSIX
    if (buffer == nullptr || buffer_size <= 0)
    {
        errno = EINVAL;
        return -1;
    }
    int32_t array_len = vm::Array::get_array_length(buffer);
    if (buffer_size > array_len)
        buffer_size = array_len;
    if (buffer_size <= 0)
    {
        errno = EINVAL;
        return -1;
    }

    utils::StringBuilder path_utf8;
    rt_string_to_utf8_path(path, path_utf8);
    uint8_t* raw = vm::Array::get_array_data_start_as<uint8_t>(buffer);
    ssize_t count = ::readlink(path_utf8.as_cstr(), reinterpret_cast<char*>(raw), static_cast<size_t>(buffer_size));
    return static_cast<int32_t>(count);
#else
    (void)path;
    (void)buffer;
    (void)buffer_size;
    return -1;
#endif
}

} // namespace platform
} // namespace leanclr

#include "rt_sys.h"

#include "build_config.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"
#include "platform/bcrypt.h"
#include "platform/rt_io_error_internal.h"
#include "utils/string_builder.h"
#include "utils/string_util.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef LEANCLR_PLATFORM_POSIX
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <utime.h>
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

struct ManagedFileStatus
{
    int32_t Flags;
    int32_t Mode;
    uint32_t Uid;
    uint32_t Gid;
    int64_t Size;
    int64_t ATime;
    int64_t ATimeNsec;
    int64_t MTime;
    int64_t MTimeNsec;
    int64_t CTime;
    int64_t CTimeNsec;
    int64_t BirthTime;
    int64_t BirthTimeNsec;
    int64_t Dev;
    int64_t Ino;
    uint32_t UserFlags;
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

static void convert_stat_to_managed_file_status(const struct stat& src, ManagedFileStatus* dst)
{
    dst->Flags = 0;
    dst->Mode = static_cast<int32_t>(src.st_mode);
    dst->Uid = static_cast<uint32_t>(src.st_uid);
    dst->Gid = static_cast<uint32_t>(src.st_gid);
    dst->Size = static_cast<int64_t>(src.st_size);
    dst->ATime = static_cast<int64_t>(src.st_atime);
    dst->MTime = static_cast<int64_t>(src.st_mtime);
    dst->CTime = static_cast<int64_t>(src.st_ctime);
#if defined(__APPLE__)
    dst->ATimeNsec = static_cast<int64_t>(src.st_atimespec.tv_nsec);
    dst->MTimeNsec = static_cast<int64_t>(src.st_mtimespec.tv_nsec);
    dst->CTimeNsec = static_cast<int64_t>(src.st_ctimespec.tv_nsec);
    dst->BirthTime = static_cast<int64_t>(src.st_birthtimespec.tv_sec);
    dst->BirthTimeNsec = static_cast<int64_t>(src.st_birthtimespec.tv_nsec);
    dst->Flags = 1; // FILESTATUS_FLAGS_HAS_BIRTHTIME
#elif defined(__linux__) || defined(__ANDROID__)
    dst->ATimeNsec = static_cast<int64_t>(src.st_atim.tv_nsec);
    dst->MTimeNsec = static_cast<int64_t>(src.st_mtim.tv_nsec);
    dst->CTimeNsec = static_cast<int64_t>(src.st_ctim.tv_nsec);
    dst->BirthTime = 0;
    dst->BirthTimeNsec = 0;
#else
    dst->ATimeNsec = 0;
    dst->MTimeNsec = 0;
    dst->CTimeNsec = 0;
    dst->BirthTime = 0;
    dst->BirthTimeNsec = 0;
#endif
    dst->Dev = static_cast<int64_t>(src.st_dev);
    dst->Ino = static_cast<int64_t>(src.st_ino);
    dst->UserFlags = 0;
}

static intptr_t extract_safe_handle_raw_handle(vm::RtObject* safe_handle_obj)
{
    if (safe_handle_obj == nullptr)
        return static_cast<intptr_t>(-1);

    const metadata::RtClass* safe_handle_klass = safe_handle_obj->klass;
    const metadata::RtFieldInfo* handle_field = vm::Class::get_field_for_name(safe_handle_klass, "handle", static_cast<uint32_t>(std::strlen("handle")), true);
    if (handle_field == nullptr)
        return static_cast<intptr_t>(-1);

    intptr_t handle_value = static_cast<intptr_t>(-1);
    vm::Field::get_instance_value(handle_field, safe_handle_obj, &handle_value);
    return handle_value;
}
#endif
} // namespace

int32_t RtSys::double_to_string(double value, const char* format, char* buffer, int32_t buffer_size)
{
#ifdef LEANCLR_PLATFORM_POSIX
    if (format == nullptr || buffer == nullptr || buffer_size <= 0)
    {
        errno = EINVAL;
        return -1;
    }
    return std::snprintf(buffer, static_cast<size_t>(buffer_size), format, value);
#else
    (void)value;
    (void)format;
    (void)buffer;
    (void)buffer_size;
    return -1;
#endif
}

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

int32_t RtSys::link(vm::RtString* source, vm::RtString* target)
{
#ifdef LEANCLR_PLATFORM_POSIX
    utils::StringBuilder source_utf8;
    utils::StringBuilder target_utf8;
    rt_string_to_utf8_path(source, source_utf8);
    rt_string_to_utf8_path(target, target_utf8);
    int32_t result = 0;
    while ((result = ::link(source_utf8.as_cstr(), target_utf8.as_cstr())) < 0 && errno == EINTR)
        ;
    return result;
#else
    (void)source;
    (void)target;
    return -1;
#endif
}

int32_t RtSys::symlink(vm::RtString* target, vm::RtString* link_path)
{
#ifdef LEANCLR_PLATFORM_POSIX
    utils::StringBuilder target_utf8;
    utils::StringBuilder link_path_utf8;
    rt_string_to_utf8_path(target, target_utf8);
    rt_string_to_utf8_path(link_path, link_path_utf8);
    return ::symlink(target_utf8.as_cstr(), link_path_utf8.as_cstr());
#else
    (void)target;
    (void)link_path;
    return -1;
#endif
}

uint32_t RtSys::get_e_uid()
{
#ifdef LEANCLR_PLATFORM_POSIX
    return static_cast<uint32_t>(::geteuid());
#else
    return 0;
#endif
}

uint32_t RtSys::get_e_gid()
{
#ifdef LEANCLR_PLATFORM_POSIX
    return static_cast<uint32_t>(::getegid());
#else
    return 0;
#endif
}

int32_t RtSys::f_stat(vm::RtObject* fd, void* output)
{
#ifdef LEANCLR_PLATFORM_POSIX
    if (output == nullptr)
    {
        errno = EINVAL;
        return -1;
    }
    intptr_t handle = extract_safe_handle_raw_handle(fd);
    if (handle == static_cast<intptr_t>(-1))
    {
        errno = EINVAL;
        return -1;
    }
    struct stat st{};
    int32_t ret = 0;
    while ((ret = ::fstat(static_cast<int>(handle), &st)) < 0 && errno == EINTR)
        ;
    if (ret == 0)
        convert_stat_to_managed_file_status(st, static_cast<ManagedFileStatus*>(output));
    return ret;
#else
    (void)fd;
    (void)output;
    return -1;
#endif
}

int32_t RtSys::stat_string(vm::RtString* path, void* output)
{
#ifdef LEANCLR_PLATFORM_POSIX
    if (output == nullptr)
    {
        errno = EINVAL;
        return -1;
    }
    utils::StringBuilder path_utf8;
    rt_string_to_utf8_path(path, path_utf8);
    struct stat st{};
    int32_t ret = 0;
    while ((ret = ::stat(path_utf8.as_cstr(), &st)) < 0 && errno == EINTR)
        ;
    if (ret == 0)
        convert_stat_to_managed_file_status(st, static_cast<ManagedFileStatus*>(output));
    return ret;
#else
    (void)path;
    (void)output;
    return -1;
#endif
}

int32_t RtSys::stat_byte(uint8_t* path, void* output)
{
#ifdef LEANCLR_PLATFORM_POSIX
    if (path == nullptr || output == nullptr)
    {
        errno = EINVAL;
        return -1;
    }
    struct stat st{};
    int32_t ret = 0;
    while ((ret = ::stat(reinterpret_cast<const char*>(path), &st)) < 0 && errno == EINTR)
        ;
    if (ret == 0)
        convert_stat_to_managed_file_status(st, static_cast<ManagedFileStatus*>(output));
    return ret;
#else
    (void)path;
    (void)output;
    return -1;
#endif
}

int32_t RtSys::lstat_string(vm::RtString* path, void* output)
{
#ifdef LEANCLR_PLATFORM_POSIX
    if (output == nullptr)
    {
        errno = EINVAL;
        return -1;
    }
    utils::StringBuilder path_utf8;
    rt_string_to_utf8_path(path, path_utf8);
    struct stat st{};
    int32_t ret = ::lstat(path_utf8.as_cstr(), &st);
    if (ret == 0)
        convert_stat_to_managed_file_status(st, static_cast<ManagedFileStatus*>(output));
    return ret;
#else
    (void)path;
    (void)output;
    return -1;
#endif
}

int32_t RtSys::lstat_byte(uint8_t* path, void* output)
{
#ifdef LEANCLR_PLATFORM_POSIX
    if (path == nullptr || output == nullptr)
    {
        errno = EINVAL;
        return -1;
    }
    struct stat st{};
    int32_t ret = ::lstat(reinterpret_cast<const char*>(path), &st);
    if (ret == 0)
        convert_stat_to_managed_file_status(st, static_cast<ManagedFileStatus*>(output));
    return ret;
#else
    (void)path;
    (void)output;
    return -1;
#endif
}

int32_t RtSys::convert_error_pal_to_platform(int32_t error)
{
#ifdef LEANCLR_PLATFORM_POSIX
    using namespace os::io_error_internal;
    switch (error)
    {
    case kErrorSuccess:
        return 0;
    case kErrorFileNotFound:
        return ENOENT;
    case kErrorPathNotFound:
        return ENOTDIR;
    case kErrorAccessDenied:
        return EACCES;
    case kErrorInvalidHandle:
        return EBADF;
    case kErrorFileExists:
        return EEXIST;
    case kErrorInvalidParameter:
        return EINVAL;
    case kErrorHandleDiskFull:
        return ENOSPC;
    case kErrorDirectory:
        return EISDIR;
    default:
        return EIO;
    }
#else
    (void)error;
    return 0;
#endif
}

int32_t RtSys::convert_error_platform_to_pal(int32_t error)
{
#ifdef LEANCLR_PLATFORM_POSIX
    return os::io_error_internal::errno_to_monoio(error);
#else
    return error;
#endif
}

uint8_t* RtSys::str_error_r(int32_t error, uint8_t* buffer, int32_t buffer_size)
{
#ifdef LEANCLR_PLATFORM_POSIX
    if (buffer == nullptr || buffer_size <= 0)
        return nullptr;

    char* out = reinterpret_cast<char*>(buffer);
#if defined(__GLIBC__) && !defined(_GNU_SOURCE)
    int rc = ::strerror_r(error, out, static_cast<size_t>(buffer_size));
    if (rc != 0)
        return nullptr;
    return buffer;
#elif defined(__GLIBC__)
    char* msg = ::strerror_r(error, out, static_cast<size_t>(buffer_size));
    if (msg == nullptr)
        return nullptr;
    if (msg != out)
    {
        std::snprintf(out, static_cast<size_t>(buffer_size), "%s", msg);
    }
    return buffer;
#else
    int rc = ::strerror_r(error, out, static_cast<size_t>(buffer_size));
    if (rc != 0)
        return nullptr;
    return buffer;
#endif
#else
    (void)error;
    (void)buffer;
    (void)buffer_size;
    return nullptr;
#endif
}

void RtSys::get_non_cryptographically_secure_random_bytes(uint8_t* buffer, int32_t length)
{
    platform::Bcrypt::gen_random(0, buffer, length, 0);
}

int32_t RtSys::copy_file(vm::RtObject* source, vm::RtObject* destination)
{
#ifdef LEANCLR_PLATFORM_POSIX
    intptr_t source_handle = extract_safe_handle_raw_handle(source);
    intptr_t destination_handle = extract_safe_handle_raw_handle(destination);
    if (source_handle == static_cast<intptr_t>(-1) || destination_handle == static_cast<intptr_t>(-1))
    {
        errno = EINVAL;
        return -1;
    }

    constexpr size_t kBufferLen = 80 * 1024;
    char* buf = static_cast<char*>(std::malloc(kBufferLen));
    if (buf == nullptr)
    {
        errno = ENOMEM;
        return -1;
    }

    int in_fd = static_cast<int>(source_handle);
    int out_fd = static_cast<int>(destination_handle);
    int32_t ret = 0;
    while (true)
    {
        ssize_t bytes_read = 0;
        while ((bytes_read = ::read(in_fd, buf, kBufferLen)) < 0 && errno == EINTR)
            ;
        if (bytes_read < 0)
        {
            ret = -1;
            break;
        }
        if (bytes_read == 0)
            break;

        ssize_t offset = 0;
        while (bytes_read > 0)
        {
            ssize_t bytes_written = 0;
            while ((bytes_written = ::write(out_fd, buf + offset, static_cast<size_t>(bytes_read))) < 0 && errno == EINTR)
                ;
            if (bytes_written < 0)
            {
                ret = -1;
                goto cleanup;
            }
            offset += bytes_written;
            bytes_read -= bytes_written;
        }
    }

cleanup:
    std::free(buf);
    return ret;
#else
    (void)source;
    (void)destination;
    return -1;
#endif
}

int32_t RtSys::lchflags(vm::RtString* path, uint32_t flags)
{
#ifdef LEANCLR_PLATFORM_POSIX
    (void)path;
    (void)flags;
#if defined(__APPLE__)
    utils::StringBuilder path_utf8;
    rt_string_to_utf8_path(path, path_utf8);
    int32_t result = 0;
    while ((result = ::lchflags(path_utf8.as_cstr(), flags)) < 0 && errno == EINTR)
        ;
    return result;
#else
    errno = ENOTSUP;
    return -1;
#endif
#else
    (void)path;
    (void)flags;
    return -1;
#endif
}

int32_t RtSys::lchflags_can_set_hidden_flag()
{
#if LEANCLR_PLATFORM_POSIX && defined(__APPLE__)
    return 1;
#else
    return 0;
#endif
}

int32_t RtSys::utime(vm::RtString* path, void* time_buffer)
{
#ifdef LEANCLR_PLATFORM_POSIX
    (void)time_buffer;
    // IL2CPP PAL currently treats this as unsupported path.
    utils::StringBuilder path_utf8;
    rt_string_to_utf8_path(path, path_utf8);
    (void)path_utf8;
    errno = ENOTSUP;
    return -1;
#else
    (void)path;
    (void)time_buffer;
    return -1;
#endif
}

int32_t RtSys::utimes(vm::RtString* path, void* time_value_pair)
{
#ifdef LEANCLR_PLATFORM_POSIX
    if (time_value_pair == nullptr)
    {
        errno = EINVAL;
        return -1;
    }
    utils::StringBuilder path_utf8;
    rt_string_to_utf8_path(path, path_utf8);
    timeval* times = reinterpret_cast<timeval*>(time_value_pair);
    int32_t result = 0;
    while ((result = ::utimes(path_utf8.as_cstr(), times)) < 0 && errno == EINTR)
        ;
    return result;
#else
    (void)path;
    (void)time_value_pair;
    return -1;
#endif
}

int32_t RtSys::globalization_get_time_zone_display_name(vm::RtString* locale_name, vm::RtString* time_zone_id, int32_t type, vm::RtObject* result,
                                                        int32_t result_length)
{
    (void)locale_name;
    (void)time_zone_id;
    (void)type;
    (void)result;
    (void)result_length;
#ifdef LEANCLR_PLATFORM_POSIX
    // Keep parity with IL2CPP PAL fallback (pal_unused.cpp): unsupported.
    return -1;
#else
    return -1;
#endif
}

} // namespace platform
} // namespace leanclr

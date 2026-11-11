#include "host/services.h"
#include "vm/environment.h"
#include "icalls/system_environment.h"
#include "icalls/system_consoleriver.h"
#include "bcl/mono/platform/rt_sys.h"
#include <cstdlib>
#include <cstring>
using namespace leanclr;

static void check(bool b)
{
    if (!b)
        std::abort();
}

static int exits, reads, closes;

static int32_t query(host::StringQuery, const char*, const char**)
{
    return host::NotSupported;
}

static int32_t integer(host::IntegerQuery, int64_t* value)
{
    *value = 7;
    return 0;
}

static int32_t open_dir(const char*, intptr_t* value)
{
    *value = 71;
    return 0;
}

static int32_t read_dir(intptr_t handle, host::DirectoryEntry* value)
{
    check(handle == 71);
    ++reads;
    *value = {"entry", 5, 8};
    return 0;
}

static int32_t close_dir(intptr_t handle)
{
    check(handle == 71);
    ++closes;
    return 0;
}

static int32_t stat_path(const char* path, bool follow, host::FileInfo* value)
{
    check(std::strcmp(path, "mem") == 0 && follow);
    value->size = 123;
    value->mode = 8;
    return 0;
}

static void returning_exit(int32_t code)
{
    check(code == 19);
    ++exits;
}

int main(int argc, char** argv)
{
    if (argc > 1 && std::strcmp(argv[1], "exit") == 0)
    {
        auto r = icalls::SystemEnvironment::exit(23);
        (void)r.is_ok();
        return 99;
    }
#if LEANCLR_CUSTOM_HOST
    const char* unchanged = "unchanged";
    check(host::Services::query_string(host::StringQuery::CurrentDirectory, nullptr, &unchanged) == host::NotSupported &&
          std::strcmp(unchanged, "unchanged") == 0);
    int64_t n = 99;
    check(host::Services::query_integer(host::IntegerQuery::PageSize, &n) == host::NotSupported && n == 99);
    int32_t error = 0;
    check(host::Services::files().open(nullptr, 0, 0, 0, 0, &error) == -1 && error == host::NotSupported);
    auto exit = icalls::SystemEnvironment::exit(19);
    check(exit.is_err() && exit.unwrap_err() == RtErr::NotSupported);
    auto echo = icalls::SystemConsoleDriver::set_echo(true);
    check(echo.is_err() && echo.unwrap_err() == RtErr::NotSupported);
    check(platform::RtSys::stat_byte(nullptr, nullptr) == -1 && platform::RtSys::get_last_win32_error() == host::NotSupported);
#else
    host::ServiceCallbacks cb = {};
    cb.extended.directories.open = open_dir;
    check(!host::Services::configure(cb));
    cb.extended.directories = {open_dir, read_dir, close_dir};
    cb.extended.query_string = query;
    cb.extended.query_integer = integer;
    cb.extended.stat = stat_path;
    cb.extended.exit_process = returning_exit;
    check(host::Services::configure(cb));
    const char* unchanged = "unchanged";
    check(host::Services::query_string(host::StringQuery::CurrentDirectory, nullptr, &unchanged) == 50 && std::strcmp(unchanged, "unchanged") == 0);
    auto count = icalls::SystemEnvironment::get_processor_count();
    check(count.is_ok() && count.unwrap() == 7);
    auto handle = platform::RtSys::open_dir(nullptr);
    check(handle == 71);

    struct Entry
    {
        char* name;
        int32_t length, type;
    };

    alignas(Entry) unsigned char entry_bytes[sizeof(Entry)] = {};
    check(platform::RtSys::read_dir_r(handle, nullptr, 0, entry_bytes) == 0);
    Entry entry;
    std::memcpy(&entry, entry_bytes, sizeof(entry));
    check(std::strcmp(entry.name, "entry") == 0 && entry.length == 5 && reads == 1);
    check(platform::RtSys::close_dir(handle) == 0 && closes == 1);
    alignas(host::FileInfo) unsigned char bytes[sizeof(host::FileInfo)] = {};
    check(platform::RtSys::stat_byte(reinterpret_cast<uint8_t*>(const_cast<char*>("mem")), bytes) == 0);
    host::FileInfo info;
    std::memcpy(&info, bytes, sizeof(info));
    check(info.size == 123 && info.mode == 8);
    auto exit = icalls::SystemEnvironment::exit(19);
    check(exit.is_err() && exit.unwrap_err() == RtErr::InvalidOperation && exits == 1);
    check(!host::Services::configure(cb));
#endif
    return 0;
}

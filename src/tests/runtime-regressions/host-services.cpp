#include "fake-host-services.h"
#include "icalls/system_datetime.h"
#include "icalls/system_diagnostics_stopwatch.h"
#include "icalls/system_threading_timer.h"
#include "icalls/system_io_monoio.h"
#include "icalls/interop.h"
#include "log/internal_logger.h"
#include "vm/debugger.h"
#include <cstdio>
#include <cstdlib>
using namespace leanclr;

static void check(bool value, const char* message)
{
    if (!value)
    {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

static bool loader_success = true;

static bool load(const char*, const char*, host::FileData& data)
{
    static const uint8_t bytes[] = {1, 2, 3};
    data = {bytes, sizeof(bytes), true};
    return loader_success;
}

static int debugger_logs;

static void debugger_log(int32_t level, const uint16_t* category, size_t category_length, const uint16_t* message, size_t message_length)
{
    check(level == 2 && category_length == 1 && message_length == 1 && category[0] == 'C' && message[0] == 'M',
          "debugger bridge preserves native text and lengths");
    ++debugger_logs;
}

int main()
{
    const auto before = host::Services::get_callbacks();
    host::ServiceCallbacks invalid = {};
    invalid.utc_ticks_100ns = FakeHost::utc;
    invalid.files.open = FakeHost::open;
    check(!host::Services::configure(invalid) && host::Services::get_callbacks().utc_ticks_100ns == before.utc_ticks_100ns,
          "partial file domain must be rejected transactionally");
    check(host::Services::set_debugger_log(nullptr) && host::Services::get_callbacks().debugger_log == nullptr, "debugger logging can be disabled at startup");
    auto callbacks = FakeHost::callbacks();
    callbacks.debugger_log = debugger_log;
    callbacks.load_assembly = load;
    check(host::Services::configure(callbacks), "configure a native-only host");
    host::FileData data = {};
    check(host::Services::load_assembly("fixture", "dll", data) && data.length == 3 && data.shared, "assembly loader routing");
    const auto original = data;
    loader_success = false;
    check(!host::Services::load_assembly("missing", "dll", data) && data.data == original.data && data.length == original.length,
          "failed loader preserves output");
    check(!host::Services::configure(callbacks) && !host::Services::set_file_loader(load), "services lock after first use");
    auto date = icalls::SystemDateTime::get_system_time_as_file_time();
    check(date.is_ok() && date.unwrap() == FakeHost::state().utc + INT64_C(116444736000000000), "BCL FILETIME epoch mapping");
    auto timestamp = icalls::SystemDiagnosticsStopwatch::get_timestamp();
    check(timestamp.is_ok() && timestamp.unwrap() == FakeHost::state().monotonic / 100, "Stopwatch uses host monotonic clock");
    FakeHost::state().utc -= 100000;
    auto timer = icalls::SystemThreadingTimer::get_time_monotonic();
    check(timer.is_ok() && timer.unwrap() == FakeHost::state().monotonic / 100, "wall clock jumps cannot affect timer");
    const uint16_t path[] = {109, 101, 109, 0};
    int32_t error = -1;
    auto opened = icalls::SystemIOMonoIO::open(path, 3, 3, 0, 0, &error);
    check(opened.is_ok() && opened.unwrap() == 42 && error == 0, "BCL file open routes to host");
    uint8_t bytes[4] = {};
    const auto& files = host::Services::files();
    check(files.read(42, bytes, 3, &error) == 3 && bytes[2] == 67, "native host file read");
    auto offset = icalls::SystemIOMonoIO::seek(42, 0, 0, &error);
    check(offset.is_ok() && offset.unwrap() == 0, "BCL seek routing");
    const uint8_t written[] = {90, 91};
    check(files.write(42, written, 2, &error) == 2, "native host file write");
    auto size = icalls::SystemIOMonoIO::get_length(42, &error);
    check(size.is_ok() && size.unwrap() == 3, "BCL length routing");
    auto closed = icalls::SystemIOMonoIO::close(42, &error);
    check(closed.is_ok() && closed.unwrap() && FakeHost::state().closes == 1, "file handle released to owning backend");
    check(host::Services::random_bytes(bytes, sizeof(bytes), host::RandomKind::Cryptographic) && bytes[3] == 20, "host random routing");
    FakeHost::state().fail_random = true;
    check(!host::Services::random_bytes(bytes, sizeof(bytes), host::RandomKind::Cryptographic), "entropy failure propagates");
    auto status = icalls::Interop::bcrypt_gen_random(0, bytes, sizeof(bytes), 0);
    check(status.is_ok() && status.unwrap() != 0, "BCL entropy failure is not reported as success");
    const int calls = FakeHost::state().random_calls;
    check(host::Services::random_bytes(nullptr, 0, host::RandomKind::Cryptographic) && FakeHost::state().random_calls == calls, "empty random request");
    host::Services::logf(host::LogLevel::Error, "custom %d", 7);
    check(FakeHost::state().logs == 1 && std::strcmp(FakeHost::state().last_log, "custom 7") == 0, "host log routing");
    log::InternalLogger::error("core diagnostic");
    check(FakeHost::state().logs == 2 && std::strcmp(FakeHost::state().last_log, "core diagnostic") == 0, "core diagnostics use host sink");
    vm::RtString category = {}, message = {};
    category.length = message.length = 1;
    category.first_char = 'C';
    message.first_char = 'M';
    vm::Debugger::log(2, &category, &message);
    check(debugger_logs == 1, "core debugger routes to host without BCL layout");
    std::puts("host services passed");
}

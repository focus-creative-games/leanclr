#include "vm/settings.h"
#include "alloc/general_allocation.h"
#include <cstdlib>
#include <cstring>
using namespace leanclr;
static alloc::MemoryCallbacks original;
static int remaining = -1;
static size_t live;

static bool failing()
{
    if (remaining < 0)
        return false;
    if (!remaining)
        return true;
    --remaining;
    return false;
}

static void* allocate(size_t n)
{
    if (failing())
        return nullptr;
    void* p = original.malloc(n);
    if (p)
        ++live;
    return p;
}

static void* zero(size_t n, size_t s)
{
    if (failing())
        return nullptr;
    void* p = original.calloc(n, s);
    if (p)
        ++live;
    return p;
}

static void release(void* p)
{
    --live;
    original.free(p);
}

static void check(bool b)
{
    if (!b)
        std::abort();
}

static void ok(RtResultVoid r)
{
    check(r.is_ok());
}

static void error(RtResultVoid r, RtErr e)
{
    check(r.is_err() && r.unwrap_err() == e);
}

static void preserved(const char* previous, size_t count)
{
    int32_t argc;
    const char** argv;
    vm::Settings::get_command_line_arguments(argc, argv);
    check(argc == 1 && argv[0] == previous && std::strcmp(argv[0], "old") == 0 && live == count);
}

int main()
{
    original = alloc::GeneralAllocation::get_memory_callbacks();
    auto cb = original;
    cb.malloc = allocate;
    cb.calloc = zero;
    cb.free = release;
    check(alloc::GeneralAllocation::set_memory_callbacks(cb));
    ok(vm::Settings::try_set_command_line_arguments(0, nullptr));
    const char* old[] = {"old"};
    ok(vm::Settings::try_set_command_line_arguments(1, old));
    int32_t argc;
    const char** argv;
    vm::Settings::get_command_line_arguments(argc, argv);
    const char* previous = argv[0];
    error(vm::Settings::try_set_command_line_arguments(-1, old), RtErr::ArgumentOutOfRange);
    error(vm::Settings::try_set_command_line_arguments(1025, old), RtErr::ArgumentOutOfRange);
    error(vm::Settings::try_set_command_line_arguments(INT32_MAX, old), RtErr::ArgumentOutOfRange);
    error(vm::Settings::try_set_command_line_arguments(1, nullptr), RtErr::ArgumentNull);
    const char* bad[] = {"first", nullptr};
    error(vm::Settings::try_set_command_line_arguments(2, bad), RtErr::ArgumentNull);
    preserved(previous, 1);
    const char* many[1024];
    const Utf16Char a[] = {0x4e2d, 0xd83d, 0xde00, 0};
    const Utf16Char* wide[1024];
    for (int i = 0; i < 1024; ++i)
    {
        many[i] = "new";
        wide[i] = a;
    }
    for (int i = 0; i <= 1024; ++i)
    {
        remaining = i;
        error(vm::Settings::try_set_command_line_arguments(1024, many), RtErr::OutOfMemory);
        preserved(previous, 1);
    }
    for (int i = 0; i <= 1024; ++i)
    {
        remaining = i;
        error(vm::Settings::try_set_command_line_arguments_utf16(1024, wide), RtErr::OutOfMemory);
        preserved(previous, 1);
    }
    remaining = -1;
    error(vm::Settings::try_set_command_line_arguments_utf16(-1, wide), RtErr::ArgumentOutOfRange);
    error(vm::Settings::try_set_command_line_arguments_utf16(1025, wide), RtErr::ArgumentOutOfRange);
    error(vm::Settings::try_set_command_line_arguments_utf16(1, nullptr), RtErr::ArgumentNull);
    const Utf16Char invalid[] = {0xd800, 0};
    const Utf16Char* invalid_args[] = {invalid};
    error(vm::Settings::try_set_command_line_arguments_utf16(1, invalid_args), RtErr::Argument);
    preserved(previous, 1);
    ok(vm::Settings::try_set_command_line_arguments(1024, many));
    check(live == 1024);
    ok(vm::Settings::try_set_command_line_arguments_utf16(1024, wide));
    check(live == 1024);
    vm::Settings::get_command_line_arguments(argc, argv);
    check(argc == 1024 && std::strcmp(argv[1023], "\xe4\xb8\xad\xf0\x9f\x98\x80") == 0);
    vm::Settings::set_command_line_arguments(1, argv);
    check(live == 1);
    ok(vm::Settings::try_set_command_line_arguments_utf16(0, nullptr));
    check(live == 0);
    vm::Settings::get_command_line_arguments(argc, argv);
    check(argc == 0 && argv[0] == nullptr);
    return 0;
}

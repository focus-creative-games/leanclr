#include "host/services.h"
#include "vm/runtime.h"
#include "vm/environment.h"
#include <cstdlib>
using namespace leanclr;
static int attempts;

static bool loader(const char*, const char*, host::FileData&)
{
    ++attempts;
    auto reentrant = vm::Runtime::initialize();
    if (!reentrant.is_err() || reentrant.unwrap_err() != RtErr::InvalidOperation)
        std::abort();
    vm::Runtime::shutdown(); // ignored while initialization is on the stack
    return false;
}

static void check(bool b)
{
    if (!b)
        std::abort();
}

int main(int argc, char**)
{
    check(vm::Runtime::get_state() == vm::Runtime::State::Uninitialized);
    if (argc > 1)
    {
        vm::Runtime::shutdown();
        check(vm::Runtime::get_state() == vm::Runtime::State::Stopped);
    }
    else
    {
        if (!host::Services::set_file_loader(loader))
            leanclr::panic("Configure file loader before host services are used");
        auto failed = vm::Runtime::initialize();
        check(failed.is_err());
        check(vm::Runtime::get_state() == vm::Runtime::State::Failed && attempts == 1);
    }
    auto again = vm::Runtime::initialize();
    check(again.is_err() && again.unwrap_err() == RtErr::InvalidOperation);
    auto invoke = vm::Runtime::invoke_with_run_cctor(nullptr, nullptr, nullptr);
    check(invoke.is_err() && invoke.unwrap_err() == RtErr::InvalidOperation);
    vm::Runtime::shutdown();
    vm::Runtime::shutdown();
    check(vm::Runtime::get_state() == vm::Runtime::State::Stopped && vm::Environment::has_shutdown_started());
    return 0;
}

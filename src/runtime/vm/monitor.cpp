#include "monitor.h"

namespace leanclr
{
namespace vm
{

void Monitor::enter(RtObject* obj)
{
}

void Monitor::exit(RtObject* obj)
{
}

bool Monitor::monitor_test_synchronized(RtObject* obj)
{
    return true;
}

void Monitor::monitor_pulse(RtObject* obj)
{
}

void Monitor::monitor_pulse_all(RtObject* obj)
{
}

bool Monitor::monitor_wait(RtObject* obj, int32_t milliseconds_timeout)
{
    return true;
}

bool Monitor::monitor_try_enter(RtObject* obj, int32_t timeout)
{
    return true;
}

void Monitor::monitor_try_enter_with_atomic_var(RtObject* obj, int32_t timeout, bool* lock_taken)
{
    *lock_taken = true;
}

bool Monitor::monitor_test_owner(RtObject* obj)
{
    return true;
}

} // namespace vm
} // namespace leanclr

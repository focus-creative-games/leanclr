#include "monitor.h"
#include "utils/hashmap.h"

namespace leanclr
{
namespace vm
{

static utils::HashMap<RtObject*, int32_t>& get_g_monitor_count()
{
    static utils::HashMap<RtObject*, int32_t> value;
    return value;
}

void Monitor::enter(RtObject* obj)
{
    // FIXME
    get_g_monitor_count()[obj]++;
}

void Monitor::exit(RtObject* obj)
{
    // FIXME
    auto it = get_g_monitor_count().find(obj);
    if (it == get_g_monitor_count().end())
    {
        return;
    }
    it->second--;
    if (it->second == 0)
    {
        get_g_monitor_count().erase(it);
    }
}

bool Monitor::monitor_test_synchronized(RtObject* obj)
{
    // FIXME
    auto it = get_g_monitor_count().find(obj);
    return it != get_g_monitor_count().end();
}

void Monitor::monitor_pulse(RtObject* obj)
{
    // FIXME
}

void Monitor::monitor_pulse_all(RtObject* obj)
{
    // FIXME
}

bool Monitor::monitor_wait(RtObject* obj, int32_t milliseconds_timeout)
{
    // FIXME
    auto it = get_g_monitor_count().find(obj);
    return it != get_g_monitor_count().end();
}

bool Monitor::monitor_try_enter(RtObject* obj, int32_t timeout)
{
    // FIXME
    get_g_monitor_count()[obj]++;
    return true;
}

void Monitor::monitor_try_enter_with_atomic_var(RtObject* obj, int32_t timeout, bool* lock_taken)
{
    // FIXME
    get_g_monitor_count()[obj]++;
    *lock_taken = true;
}

bool Monitor::monitor_test_owner(RtObject* obj)
{
    // FIXME
    auto it = get_g_monitor_count().find(obj);
    return it != get_g_monitor_count().end();
}

} // namespace vm
} // namespace leanclr

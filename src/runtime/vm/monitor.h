#pragma once

#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{
class Monitor
{
  public:
    // Monitor functions would be declared here
    static void enter(RtObject* obj);
    static void exit(RtObject* obj);
    static bool monitor_test_synchronized(RtObject* obj);
    static void monitor_pulse(RtObject* obj);
    static void monitor_pulse_all(RtObject* obj);
    static bool monitor_wait(RtObject* obj, int32_t milliseconds_timeout);
    static bool monitor_try_enter(RtObject* obj, int32_t timeout);
    static void monitor_try_enter_with_atomic_var(RtObject* obj, int32_t timeout, bool* lock_taken);
    static bool monitor_test_owner(RtObject* obj);
};
} // namespace vm
} // namespace leanclr

#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemThreadingMonitor
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // Enter the monitor
    static RtResultVoid enter(vm::RtObject* monitor);

    // Exit the monitor
    static RtResultVoid exit(vm::RtObject* monitor);

    // Test if synchronized
    static RtResult<bool> monitor_test_synchronized(vm::RtObject* monitor);

    // Pulse the monitor
    static RtResultVoid monitor_pulse(vm::RtObject* monitor);

    // Pulse all waiting threads
    static RtResultVoid monitor_pulse_all(vm::RtObject* monitor);

    // Wait on the monitor
    static RtResult<bool> monitor_wait(vm::RtObject* monitor, int32_t milliseconds_timeout);

    // Try to enter the monitor with atomic variable
    static RtResultVoid monitor_try_enter_with_atomic_var(vm::RtObject* monitor, int32_t timeout, bool* lock_taken);

    // Test if current thread owns the monitor
    static RtResult<bool> monitor_test_owner(vm::RtObject* monitor);
};

} // namespace icalls
} // namespace leanclr

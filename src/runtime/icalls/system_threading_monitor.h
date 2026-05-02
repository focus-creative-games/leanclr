#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemThreadingMonitor
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Enter the monitor
    static RtResultVoid enter(vm::RtObject* monitor) noexcept;

    // Exit the monitor
    static RtResultVoid exit(vm::RtObject* monitor) noexcept;

    // Test if synchronized
    static RtResult<bool> monitor_test_synchronized(vm::RtObject* monitor) noexcept;

    // Pulse the monitor
    static RtResultVoid monitor_pulse(vm::RtObject* monitor) noexcept;

    // Pulse all waiting threads
    static RtResultVoid monitor_pulse_all(vm::RtObject* monitor) noexcept;

    // Wait on the monitor
    static RtResult<bool> monitor_wait(vm::RtObject* monitor, int32_t milliseconds_timeout) noexcept;

    // Try to enter the monitor with atomic variable
    static RtResultVoid monitor_try_enter_with_atomic_var(vm::RtObject* monitor, int32_t timeout, bool* lock_taken) noexcept;

    // Test if current thread owns the monitor
    static RtResult<bool> monitor_test_owner(vm::RtObject* monitor) noexcept;
};

} // namespace icalls
} // namespace leanclr

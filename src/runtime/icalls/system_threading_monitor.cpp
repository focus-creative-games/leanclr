#include "system_threading_monitor.h"
#include "icall_base.h"
#include "vm/monitor.h"

namespace leanclr
{
namespace icalls
{

RtResultVoid SystemThreadingMonitor::enter(vm::RtObject* monitor) noexcept
{
    vm::Monitor::enter(monitor);
    RET_VOID_OK();
}

/// @icall: System.Threading.Monitor::Enter
static RtResultVoid enter_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                  interp::RtStackObject*) noexcept
{
    auto monitor = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    return SystemThreadingMonitor::enter(monitor);
}

RtResultVoid SystemThreadingMonitor::exit(vm::RtObject* monitor) noexcept
{
    vm::Monitor::exit(monitor);
    RET_VOID_OK();
}

/// @icall: System.Threading.Monitor::Exit
static RtResultVoid exit_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                 interp::RtStackObject*) noexcept
{
    auto monitor = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    return SystemThreadingMonitor::exit(monitor);
}

RtResult<bool> SystemThreadingMonitor::monitor_test_synchronized(vm::RtObject* monitor) noexcept
{
    RET_OK(vm::Monitor::monitor_test_synchronized(monitor));
}

/// @icall: System.Threading.Monitor::Monitor_test_synchronised
static RtResultVoid monitor_test_synchronized_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                      interp::RtStackObject* ret) noexcept
{
    auto monitor = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemThreadingMonitor::monitor_test_synchronized(monitor));
    EvalStackOp::set_return(ret, (int32_t)result);
    RET_VOID_OK();
}

RtResultVoid SystemThreadingMonitor::monitor_pulse(vm::RtObject* monitor) noexcept
{
    vm::Monitor::monitor_pulse(monitor);
    RET_VOID_OK();
}

/// @icall: System.Threading.Monitor::Monitor_pulse
static RtResultVoid monitor_pulse_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                          interp::RtStackObject*) noexcept
{
    auto monitor = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    return SystemThreadingMonitor::monitor_pulse(monitor);
}

RtResultVoid SystemThreadingMonitor::monitor_pulse_all(vm::RtObject* monitor) noexcept
{
    vm::Monitor::monitor_pulse_all(monitor);
    RET_VOID_OK();
}

/// @icall: System.Threading.Monitor::Monitor_pulse_all
static RtResultVoid monitor_pulse_all_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject*) noexcept
{
    auto monitor = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    return SystemThreadingMonitor::monitor_pulse_all(monitor);
}

RtResult<bool> SystemThreadingMonitor::monitor_wait(vm::RtObject* monitor, int32_t milliseconds_timeout) noexcept
{
    RET_OK(vm::Monitor::monitor_wait(monitor, milliseconds_timeout));
}

/// @icall: System.Threading.Monitor::Monitor_wait
static RtResultVoid monitor_wait_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                         interp::RtStackObject* ret) noexcept
{
    auto monitor = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    auto milliseconds_timeout = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemThreadingMonitor::monitor_wait(monitor, milliseconds_timeout));
    EvalStackOp::set_return(ret, (int32_t)result);
    RET_VOID_OK();
}

RtResultVoid SystemThreadingMonitor::monitor_try_enter_with_atomic_var(vm::RtObject* monitor, int32_t timeout, bool* lock_taken) noexcept
{
    vm::Monitor::monitor_try_enter_with_atomic_var(monitor, timeout, lock_taken);
    RET_VOID_OK();
}

/// @icall: System.Threading.Monitor::Monitor_try_enter_with_atomic_var
static RtResultVoid monitor_try_enter_with_atomic_var_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                              const interp::RtStackObject* params, interp::RtStackObject*) noexcept
{
    auto monitor = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    auto timeout = EvalStackOp::get_param<int32_t>(params, 1);
    auto lock_taken = EvalStackOp::get_param<bool*>(params, 2);
    return SystemThreadingMonitor::monitor_try_enter_with_atomic_var(monitor, timeout, lock_taken);
}

RtResult<bool> SystemThreadingMonitor::monitor_test_owner(vm::RtObject* monitor) noexcept
{
    RET_OK(vm::Monitor::monitor_test_owner(monitor));
}

/// @icall: System.Threading.Monitor::Monitor_test_owner
static RtResultVoid monitor_test_owner_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    auto monitor = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemThreadingMonitor::monitor_test_owner(monitor));
    EvalStackOp::set_return(ret, (int32_t)result);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_threading_monitor[] = {
    {"System.Threading.Monitor::Enter", (vm::InternalCallFunction)&SystemThreadingMonitor::enter, enter_invoker},
    {"System.Threading.Monitor::Exit", (vm::InternalCallFunction)&SystemThreadingMonitor::exit, exit_invoker},
    {"System.Threading.Monitor::Monitor_test_synchronised", (vm::InternalCallFunction)&SystemThreadingMonitor::monitor_test_synchronized,
     monitor_test_synchronized_invoker},
    {"System.Threading.Monitor::Monitor_pulse", (vm::InternalCallFunction)&SystemThreadingMonitor::monitor_pulse, monitor_pulse_invoker},
    {"System.Threading.Monitor::Monitor_pulse_all", (vm::InternalCallFunction)&SystemThreadingMonitor::monitor_pulse_all, monitor_pulse_all_invoker},
    {"System.Threading.Monitor::Monitor_wait", (vm::InternalCallFunction)&SystemThreadingMonitor::monitor_wait, monitor_wait_invoker},
    {"System.Threading.Monitor::Monitor_try_enter_with_atomic_var", (vm::InternalCallFunction)&SystemThreadingMonitor::monitor_try_enter_with_atomic_var,
     monitor_try_enter_with_atomic_var_invoker},
    {"System.Threading.Monitor::Monitor_test_owner", (vm::InternalCallFunction)&SystemThreadingMonitor::monitor_test_owner, monitor_test_owner_invoker},
};

utils::Span<vm::InternalCallEntry> SystemThreadingMonitor::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_threading_monitor,
                                              sizeof(s_internal_call_entries_system_threading_monitor) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

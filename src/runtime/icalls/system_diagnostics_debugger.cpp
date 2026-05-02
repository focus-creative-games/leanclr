#include "system_diagnostics_debugger.h"

#include "icall_base.h"
#include "vm/debugger.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace icalls
{

RtResult<bool> SystemDiagnosticsDebugger::is_attached_internal() noexcept
{
    RET_OK(false);
}

/// @icall: System.Diagnostics.Debugger::IsAttached_internal()
static RtResultVoid is_attached_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemDiagnosticsDebugger::is_attached_internal());
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

RtResult<bool> SystemDiagnosticsDebugger::is_logging() noexcept
{
    RET_OK(false);
}

/// @icall: System.Diagnostics.Debugger::IsLogging()
static RtResultVoid is_logging_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemDiagnosticsDebugger::is_logging());
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

RtResultVoid SystemDiagnosticsDebugger::log_icall(int32_t level, vm::RtString** category, vm::RtString** message) noexcept
{
    vm::Debugger::log(level, *category, *message);
    RET_VOID_OK();
}

/// @icall: System.Diagnostics.Debugger::Log_icall(System.Int32,System.String&,System.String&)
static RtResultVoid log_icall_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto level = EvalStackOp::get_param<int32_t>(params, 0);
    auto category = EvalStackOp::get_param<vm::RtString**>(params, 1);
    auto message = EvalStackOp::get_param<vm::RtString**>(params, 2);
    RET_ERR_ON_FAIL(SystemDiagnosticsDebugger::log_icall(level, category, message));
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemDiagnosticsDebugger::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Diagnostics.Debugger::IsAttached_internal()", (vm::InternalCallFunction)&SystemDiagnosticsDebugger::is_attached_internal,
         is_attached_internal_invoker},
        {"System.Diagnostics.Debugger::IsLogging()", (vm::InternalCallFunction)&SystemDiagnosticsDebugger::is_logging, is_logging_invoker},
        {"System.Diagnostics.Debugger::Log_icall(System.Int32,System.String&,System.String&)", (vm::InternalCallFunction)&SystemDiagnosticsDebugger::log_icall,
         log_icall_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

#include "system_diagnostics_stacktrace.h"

#include "icall_base.h"
#include "vm/stacktrace.h"

namespace leanclr
{
namespace icalls
{

RtResult<vm::RtArray*> SystemDiagnosticsStackTrace::get_trace(vm::RtException* ex, int32_t skip_frames, bool need_file_info) noexcept
{
    return vm::StackTrace::get_stack_trace(ex, skip_frames, need_file_info);
}

/// @icall: System.Diagnostics.StackTrace::get_trace(System.Exception,System.Int32,System.Boolean)
static RtResultVoid get_trace_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto exception = EvalStackOp::get_param<vm::RtException*>(params, 0);
    auto skip_frames = EvalStackOp::get_param<int32_t>(params, 1);
    auto need_file_info = EvalStackOp::get_param<bool>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result, SystemDiagnosticsStackTrace::get_trace(exception, skip_frames, need_file_info));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemDiagnosticsStackTrace::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Diagnostics.StackTrace::get_trace(System.Exception,System.Int32,System.Boolean)",
         (vm::InternalCallFunction)&SystemDiagnosticsStackTrace::get_trace, get_trace_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

#include "system_diagnostics_stopwatch.h"

#include "platform/rt_time.h"

namespace leanclr
{
namespace icalls
{

RtResult<int64_t> SystemDiagnosticsStopwatch::get_timestamp() noexcept
{
    RET_OK(os::Time::get_ticks_100nanos());
}

/// @icall: System.Diagnostics.Stopwatch::GetTimestamp()
static RtResultVoid get_timestamp_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, result, SystemDiagnosticsStopwatch::get_timestamp());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemDiagnosticsStopwatch::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Diagnostics.Stopwatch::GetTimestamp()", (vm::InternalCallFunction)&SystemDiagnosticsStopwatch::get_timestamp, get_timestamp_invoker},
    };

    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

#include <iterator>
#include "bcl/mono/managed_layouts.h"
#include "system_threading_timer.h"

#include "host/services.h"

namespace leanclr
{
namespace icalls
{

RtResult<int64_t> SystemThreadingTimer::get_time_monotonic() noexcept
{
    RET_OK(host::Services::monotonic_nanoseconds() / 100);
}

/// @icall: System.Threading.Timer::GetTimeMonotonic()
static RtResultVoid get_time_monotonic_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, result, SystemThreadingTimer::get_time_monotonic());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemThreadingTimer::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Threading.Timer::GetTimeMonotonic()", (vm::InternalCallFunction)&SystemThreadingTimer::get_time_monotonic, get_time_monotonic_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, std::size(s_entries));
}

} // namespace icalls
} // namespace leanclr

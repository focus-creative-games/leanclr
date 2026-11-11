#include <iterator>
#include "bcl/mono/managed_layouts.h"
#include "system_datetime.h"

#include "icall_base.h"
#include "host/services.h"

namespace leanclr
{
namespace icalls
{

RtResult<int64_t> SystemDateTime::get_system_time_as_file_time() noexcept
{
    RET_OK(host::Services::utc_ticks_100ns() + INT64_C(116444736000000000));
}

/// @icall: System.DateTime::GetSystemTimeAsFileTime()
static RtResultVoid get_system_time_as_file_time_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, file_time, SystemDateTime::get_system_time_as_file_time());
    EvalStackOp::set_return(ret, file_time);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemDateTime::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.DateTime::GetSystemTimeAsFileTime()", (vm::InternalCallFunction)&SystemDateTime::get_system_time_as_file_time,
         get_system_time_as_file_time_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, std::size(s_entries));
}

} // namespace icalls
} // namespace leanclr

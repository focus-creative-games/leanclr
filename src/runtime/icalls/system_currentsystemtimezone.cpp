#include "system_currentsystemtimezone.h"

#include "icall_base.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"
#include "vm/class.h"

namespace leanclr
{
namespace icalls
{

#if defined(LEANCLR_PLATFORM_POSIX)
RtResult<bool> SystemCurrentSystemTimeZone::get_time_zone_data(int32_t year, vm::RtArray** data, vm::RtArray** names, bool* daylight)
{
    (void)year;
    (void)data;
    (void)names;
    (void)daylight;
    RET_ERR(RtErr::NotImplemented);
}

/// @icall: System.CurrentSystemTimeZone::GetTimeZoneData(System.Int32,System.Int64[]&,System.String[]&,System.Boolean&)
static RtResultVoid get_time_zone_data_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto year = EvalStackOp::get_param<int32_t>(params, 0);
    auto data = EvalStackOp::get_param<vm::RtArray**>(params, 1);
    auto names = EvalStackOp::get_param<vm::RtArray**>(params, 2);
    auto daylight = EvalStackOp::get_param<bool*>(params, 3);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemCurrentSystemTimeZone::get_time_zone_data(year, data, names, daylight));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemCurrentSystemTimeZone::get_internal_call_entries()
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.CurrentSystemTimeZone::GetTimeZoneData(System.Int32,System.Int64[]&,System.String[]&,System.Boolean&)",
         (vm::InternalCallFunction)&SystemCurrentSystemTimeZone::get_time_zone_data, get_time_zone_data_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}
#else
utils::Span<vm::InternalCallEntry> SystemCurrentSystemTimeZone::get_internal_call_entries()
{
    return utils::Span<vm::InternalCallEntry>();
}
#endif

} // namespace icalls
} // namespace leanclr

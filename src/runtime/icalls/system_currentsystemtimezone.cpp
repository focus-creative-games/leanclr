#include "system_currentsystemtimezone.h"

#include "icall_base.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"
#include "vm/class.h"

namespace leanclr
{
namespace icalls
{

#if LEANCLR_PLATFORM_POSIX
RtResult<bool> SystemCurrentSystemTimeZone::get_time_zone_data(int32_t year, vm::RtArray** data, vm::RtArray** names, bool* daylight) noexcept
{
    (void)year;
    auto corlib_types = vm::Class::get_corlib_types();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, date_arr, vm::Array::new_szarray_from_ele_klass(corlib_types.cls_int64, 4));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, names_arr, vm::Array::new_szarray_from_ele_klass(corlib_types.cls_string, 2));

    vm::RtString* utc = vm::String::create_string_from_utf8cstr("UTC");
    vm::Array::set_array_data_at<vm::RtString*>(names_arr, 0, utc);
    vm::Array::set_array_data_at<vm::RtString*>(names_arr, 1, utc);
    vm::Array::set_array_data_at<int64_t>(date_arr, 0, 0);
    vm::Array::set_array_data_at<int64_t>(date_arr, 1, 0);
    vm::Array::set_array_data_at<int64_t>(date_arr, 2, 0);
    vm::Array::set_array_data_at<int64_t>(date_arr, 3, 0);

    *data = date_arr;
    *names = names_arr;
    *daylight = false;
    RET_OK(true);
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

utils::Span<vm::InternalCallEntry> SystemCurrentSystemTimeZone::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.CurrentSystemTimeZone::GetTimeZoneData(System.Int32,System.Int64[]&,System.String[]&,System.Boolean&)",
         (vm::InternalCallFunction)&SystemCurrentSystemTimeZone::get_time_zone_data, get_time_zone_data_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}
#else
utils::Span<vm::InternalCallEntry> SystemCurrentSystemTimeZone::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>();
}
#endif

} // namespace icalls
} // namespace leanclr

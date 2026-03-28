#include "system_currentsystemtimezone.h"

#include "icall_base.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"
#include "vm/class.h"

namespace leanclr
{
namespace icalls
{

RtResult<bool> SystemCurrentSystemTimeZone::get_time_zone_data(int32_t year, vm::RtArray** data, vm::RtArray** names, bool* daylight)
{
    (void)year;
    // TODO: implement proper time zone data retrieval.
    // In WebAssembly, we do not have access to the system time zone data.
    // We return empty arrays and false for daylight saving time.

    auto corlib_types = vm::Class::get_corlib_types();

    // Allocate data array (4 int64 values: Bias, StandardBias, DaylightBias, StandardDate/DaylightDate)
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, date_arr, vm::Array::new_szarray_from_ele_klass(corlib_types.cls_int64, 4));

    // Allocate names array (2 string values: StandardName, DaylightName)
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, names_arr, vm::Array::new_szarray_from_ele_klass(corlib_types.cls_string, 2));

    // Create default timezone name "UTC"
    auto default_tz_name = vm::String::create_string_from_utf8cstr("UTC");

    // Set array data
    vm::Array::set_array_data_at<vm::RtString*>(names_arr, 0, default_tz_name);
    vm::Array::set_array_data_at<vm::RtString*>(names_arr, 1, default_tz_name);

    vm::Array::set_array_data_at<int64_t>(date_arr, 0, 0); // Bias
    vm::Array::set_array_data_at<int64_t>(date_arr, 1, 0); // StandardBias
    vm::Array::set_array_data_at<int64_t>(date_arr, 2, 0); // DaylightBias
    vm::Array::set_array_data_at<int64_t>(date_arr, 3, 0); // StandardDate/DaylightDate

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

utils::Span<vm::InternalCallEntry> SystemCurrentSystemTimeZone::get_internal_call_entries()
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.CurrentSystemTimeZone::GetTimeZoneData(System.Int32,System.Int64[]&,System.String[]&,System.Boolean&)",
         (vm::InternalCallFunction)&SystemCurrentSystemTimeZone::get_time_zone_data, get_time_zone_data_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

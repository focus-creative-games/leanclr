#include "system_globalization_calendardata.h"
#include "icall_base.h"

#include "utils/string_builder.h"
#include "utils/string_util.h"
#include "vm/class.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace icalls
{

namespace
{

struct CalendarCultureFillData
{
    const char* native_calendar_name;
    const char* const* short_dates;
    int32_t short_dates_count;
    const char* const* year_months;
    int32_t year_months_count;
    const char* const* long_dates;
    int32_t long_dates_count;
    const char* month_day;
    const char* const* era_names;
    int32_t era_names_count;
    const char* const* abbrev_era_names;
    int32_t abbrev_era_names_count;
    const char* const* abbrev_english_era_names;
    int32_t abbrev_english_era_names_count;
    const char* const* day_names;
    int32_t day_names_count;
    const char* const* abbrev_day_names;
    int32_t abbrev_day_names_count;
    const char* const* super_short_day_names;
    int32_t super_short_day_names_count;
    const char* const* month_names;
    int32_t month_names_count;
    const char* const* abbrev_month_names;
    int32_t abbrev_month_names_count;
    int32_t two_digit_year_max;
    int32_t current_era;
};

static const char* s_en_short_dates[] = {"MM/dd/yyyy", "yyyy-MM-dd"};
static const char* s_en_long_dates[] = {"dddd, dd MMMM yyyy"};
static const char* s_en_year_months[] = {"yyyy MMMM"};
static const char* s_en_era_names[] = {"A.D."};
static const char* s_en_abbrev_era_names[] = {"AD"};
static const char* s_en_day_names[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
static const char* s_en_abbrev_day_names[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char* s_en_super_short_day_names[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
static const char* s_en_month_names[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September",
                                         "October", "November", "December", ""};
static const char* s_en_abbrev_month_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", ""};

static const char* s_zh_short_dates[] = {"yyyy/M/d", "yyyy-MM-dd"};
static const char* s_zh_long_dates[] = {"yyyy'\u5e74'M'\u6708'd'\u65e5' dddd"};
static const char* s_zh_year_months[] = {"yyyy'\u5e74'M'\u6708'"};
static const char* s_zh_era_names[] = {"\xe5\x85\xac\xe5\x85\x83"};
static const char* s_zh_abbrev_era_names[] = {"\xe5\x85\xac\xe5\x85\x83"};
static const char* s_zh_day_names[] = {"\xe6\x98\x9f\xe6\x9c\x9f\xe6\x97\xa5", "\xe6\x98\x9f\xe6\x9c\x9f\xe4\xb8\x80", "\xe6\x98\x9f\xe6\x9c\x9f\xe4\xba\x8c",
                                       "\xe6\x98\x9f\xe6\x9c\x9f\xe4\xb8\x89", "\xe6\x98\x9f\xe6\x9c\x9f\xe5\x9b\x9b", "\xe6\x98\x9f\xe6\x9c\x9f\xe4\xba\x94",
                                       "\xe6\x98\x9f\xe6\x9c\x9f\xe5\x85\xad"};
static const char* s_zh_abbrev_day_names[] = {"\xe5\x91\xa8\xe6\x97\xa5", "\xe5\x91\xa8\xe4\xb8\x80", "\xe5\x91\xa8\xe4\xba\x8c", "\xe5\x91\xa8\xe4\xb8\x89",
                                             "\xe5\x91\xa8\xe5\x9b\x9b", "\xe5\x91\xa8\xe4\xba\x94", "\xe5\x91\xa8\xe5\x85\xad"};
static const char* s_zh_super_short_day_names[] = {"\xe6\x97\xa5", "\xe4\xb8\x80", "\xe4\xba\x8c", "\xe4\xb8\x89", "\xe5\x9b\x9b", "\xe4\xba\x94", "\xe5\x85\xad"};
static const char* s_zh_month_names[] = {"\xe4\xb8\x80\xe6\x9c\x88", "\xe4\xba\x8c\xe6\x9c\x88", "\xe4\xb8\x89\xe6\x9c\x88", "\xe5\x9b\x9b\xe6\x9c\x88",
                                        "\xe4\xba\x94\xe6\x9c\x88", "\xe5\x85\xad\xe6\x9c\x88", "\xe4\xb8\x83\xe6\x9c\x88", "\xe5\x85\xab\xe6\x9c\x88",
                                        "\xe4\xb9\x9d\xe6\x9c\x88", "\xe5\x8d\x81\xe6\x9c\x88", "\xe5\x8d\x81\xe4\xb8\x80\xe6\x9c\x88", "\xe5\x8d\x81\xe4\xba\x8c\xe6\x9c\x88", ""};
static const char* s_zh_abbrev_month_names[] = {"\xe4\xb8\x80\xe6\x9c\x88", "\xe4\xba\x8c\xe6\x9c\x88", "\xe4\xb8\x89\xe6\x9c\x88", "\xe5\x9b\x9b\xe6\x9c\x88",
                                                "\xe4\xba\x94\xe6\x9c\x88", "\xe5\x85\xad\xe6\x9c\x88", "\xe4\xb8\x83\xe6\x9c\x88", "\xe5\x85\xab\xe6\x9c\x88",
                                                "\xe4\xb9\x9d\xe6\x9c\x88", "\xe5\x8d\x81\xe6\x9c\x88", "\xe5\x8d\x81\xe4\xb8\x80\xe6\x9c\x88", "\xe5\x8d\x81\xe4\xba\x8c\xe6\x9c\x88", ""};

static const CalendarCultureFillData s_calendar_en = {
    "Gregorian Calendar",
    s_en_short_dates,
    2,
    s_en_year_months,
    1,
    s_en_long_dates,
    1,
    "MMMM dd",
    s_en_era_names,
    1,
    s_en_abbrev_era_names,
    1,
    s_en_abbrev_era_names,
    1,
    s_en_day_names,
    7,
    s_en_abbrev_day_names,
    7,
    s_en_super_short_day_names,
    7,
    s_en_month_names,
    13,
    s_en_abbrev_month_names,
    13,
    2029,
    1,
};

static const CalendarCultureFillData s_calendar_zh = {
    "\xe5\x85\xac\xe5\x8e\x86",
    s_zh_short_dates,
    2,
    s_zh_year_months,
    1,
    s_zh_long_dates,
    1,
    "M'\u6708'd'\u65e5'",
    s_zh_era_names,
    1,
    s_zh_abbrev_era_names,
    1,
    s_en_abbrev_era_names,
    1,
    s_zh_day_names,
    7,
    s_zh_abbrev_day_names,
    7,
    s_zh_super_short_day_names,
    7,
    s_zh_month_names,
    13,
    s_zh_abbrev_month_names,
    13,
    2029,
    1,
};

static RtResult<vm::RtArray*> create_string_array(const char* const* items, int32_t count) noexcept
{
    auto cls_string = vm::Class::get_corlib_types().cls_string;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(cls_string, count, "fill_calendar_data"));
    for (int32_t i = 0; i < count; ++i)
    {
        vm::Array::set_array_data_at<vm::RtString*>(arr, i, vm::String::create_string_from_utf8cstr(items[i]));
    }
    RET_OK(arr);
}

static bool is_gregorian_calendar_id(int32_t calendar_id)
{
    return calendar_id == 1 || calendar_id == 2 || calendar_id == 257;
}

static bool is_zh_locale(const char* locale_name)
{
    return utils::StringUtil::equals_ignorecase(locale_name, "zh") || utils::StringUtil::equals_ignorecase(locale_name, "zh-cn");
}

static bool is_known_locale(const char* locale_name)
{
    if (locale_name == nullptr || locale_name[0] == '\0')
    {
        return false;
    }
    return is_zh_locale(locale_name) || utils::StringUtil::equals_ignorecase(locale_name, "en") ||
           utils::StringUtil::equals_ignorecase(locale_name, "en-us");
}

static const CalendarCultureFillData* find_calendar_culture_data(const char* locale_name)
{
    if (!is_known_locale(locale_name))
    {
        return nullptr;
    }
    if (is_zh_locale(locale_name))
    {
        return &s_calendar_zh;
    }
    return &s_calendar_en;
}

static void assign_optional_string_array(vm::RtArray** dest, const char* const* items, int32_t count)
{
    auto arr = create_string_array(items, count);
    if (arr.is_ok())
    {
        *dest = arr.unwrap();
    }
}

static void assign_calendar_culture_data(vm::RtCalendarData* calendar_data, const CalendarCultureFillData& data)
{
    calendar_data->native_calendar_name = vm::String::create_string_from_utf8cstr(data.native_calendar_name);
    calendar_data->month_day = vm::String::create_string_from_utf8cstr(data.month_day);
    calendar_data->two_digit_year_max = data.two_digit_year_max;
    calendar_data->current_era = data.current_era;

    assign_optional_string_array(&calendar_data->short_date_patterns, data.short_dates, data.short_dates_count);
    assign_optional_string_array(&calendar_data->year_month_patterns, data.year_months, data.year_months_count);
    assign_optional_string_array(&calendar_data->long_date_patterns, data.long_dates, data.long_dates_count);
    assign_optional_string_array(&calendar_data->era_names, data.era_names, data.era_names_count);
    assign_optional_string_array(&calendar_data->abbreviated_era_names, data.abbrev_era_names, data.abbrev_era_names_count);
    assign_optional_string_array(&calendar_data->abbreviated_english_era_names, data.abbrev_english_era_names, data.abbrev_english_era_names_count);
    assign_optional_string_array(&calendar_data->day_names, data.day_names, data.day_names_count);
    assign_optional_string_array(&calendar_data->abbreviated_day_names, data.abbrev_day_names, data.abbrev_day_names_count);
    assign_optional_string_array(&calendar_data->super_short_day_names, data.super_short_day_names, data.super_short_day_names_count);
    assign_optional_string_array(&calendar_data->month_names, data.month_names, data.month_names_count);
    assign_optional_string_array(&calendar_data->abbreviated_month_names, data.abbrev_month_names, data.abbrev_month_names_count);
    assign_optional_string_array(&calendar_data->month_genitive_names, data.month_names, data.month_names_count);
    assign_optional_string_array(&calendar_data->abbreviated_month_genitive_names, data.abbrev_month_names, data.abbrev_month_names_count);
    assign_optional_string_array(&calendar_data->leap_year_month_names, data.month_names, data.month_names_count);
}

} // namespace

RtResult<bool> SystemGlobalizationCalendarData::fill_calendar_data(vm::RtCalendarData* _this, vm::RtString* locale_name, int32_t calendar_id) noexcept
{
    if (_this == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    assert (locale_name != nullptr);

    if (!is_gregorian_calendar_id(calendar_id))
    {
        RET_OK(false);
    }

    utils::Utf8StringBuilder locale_utf8(vm::String::get_chars_ptr(locale_name), static_cast<size_t>(vm::String::get_length(locale_name)));
    const CalendarCultureFillData* data = find_calendar_culture_data(locale_utf8.get_const_chars());
    if (data == nullptr)
    {
        RET_OK(false);
    }

    assign_calendar_culture_data(_this, *data);
    RET_OK(true);
}

/// @icall: System.Globalization.CalendarData::fill_calendar_data(System.String,System.Int32)
static RtResultVoid fill_calendar_data_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto _this = EvalStackOp::get_param<vm::RtCalendarData*>(params, 0);
    auto locale_name = EvalStackOp::get_param<vm::RtString*>(params, 1);
    auto calendar_id = EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemGlobalizationCalendarData::fill_calendar_data(_this, locale_name, calendar_id));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_globalization_calendardata[] = {
    {"System.Globalization.CalendarData::fill_calendar_data(System.String,System.Int32)",
     (vm::InternalCallFunction)&SystemGlobalizationCalendarData::fill_calendar_data, fill_calendar_data_invoker},
};

utils::Span<vm::InternalCallEntry> SystemGlobalizationCalendarData::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_globalization_calendardata,
                                              sizeof(s_internal_call_entries_system_globalization_calendardata) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

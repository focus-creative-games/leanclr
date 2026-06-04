#include "system_globalization_culturedata.h"
#include "icall_base.h"

#include "vm/class.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace icalls
{

namespace
{

struct DateTimeCultureFillData
{
    const char* am_designator;
    const char* pm_designator;
    const char* time_separator;
    const char* const* long_times;
    int32_t long_times_count;
    const char* const* short_times;
    int32_t short_times_count;
    int32_t first_day_of_week;
    int32_t first_week_of_year;
};

static const char* s_en_long_times[] = {"h:mm:ss tt", "HH:mm:ss", "H:mm:ss", "HH:mm:ss"};
static const char* s_en_short_times[] = {"h:mm tt", "hh:mm tt", "H:mm", "HH:mm"};
static const char* s_zh_long_times[] = {"HH:mm:ss"};
static const char* s_zh_short_times[] = {"tt h:mm", "tt hh:mm", "H:mm", "HH:mm"};

static const DateTimeCultureFillData s_datetime_en = {
    "AM", "PM", ":", s_en_long_times, 4, s_en_short_times, 4, 0, 0,
};
static const DateTimeCultureFillData s_datetime_zh = {
    "\xe4\xb8\x8a\xe5\x8d\x88", "\xe4\xb8\x8b\xe5\x8d\x88", ":", s_zh_long_times, 1, s_zh_short_times, 4, 1, 0,
};

static const DateTimeCultureFillData* find_datetime_culture_data(int32_t datetime_index)
{
    switch (datetime_index)
    {
    case 9:  // en (mono culture-info-tables)
    case 105:
    case 119: // en-US
        return &s_datetime_en;
    case 187:
    case 217: // zh-CN
    case 268:
    case 325: // zh
        return &s_datetime_zh;
    default:
        return nullptr;
    }
}

static RtResult<vm::RtArray*> create_string_array(const char* const* items, int32_t count) noexcept
{
    auto cls_string = vm::Class::get_corlib_types().cls_string;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(cls_string, count, "fill_culture_data"));
    for (int32_t i = 0; i < count; ++i)
    {
        vm::Array::set_array_data_at<vm::RtString*>(arr, i, vm::String::create_string_from_utf8cstr(items[i]));
    }
    RET_OK(arr);
}

// Layout must match System.Globalization.CultureData/NumberFormatEntryManaged in corlib.
struct NumberFormatEntryManaged
{
    int32_t currency_decimal_digits;
    int32_t currency_decimal_separator;
    int32_t currency_group_separator;
    int32_t currency_group_sizes0;
    int32_t currency_group_sizes1;
    int32_t currency_negative_pattern;
    int32_t currency_positive_pattern;
    int32_t currency_symbol;
    int32_t nan_symbol;
    int32_t negative_infinity_symbol;
    int32_t negative_sign;
    int32_t number_decimal_digits;
    int32_t number_decimal_separator;
    int32_t number_group_separator;
    int32_t number_group_sizes0;
    int32_t number_group_sizes1;
    int32_t number_negative_pattern;
    int32_t per_mille_symbol;
    int32_t percent_negative_pattern;
    int32_t percent_positive_pattern;
    int32_t percent_symbol;
    int32_t positive_infinity_symbol;
    int32_t positive_sign;
};

// Layout aligned with mono/metadata/culture-info-tables.h (NumberFormatEntry).
struct NumberFormatEntryNative
{
    int16_t currency_decimal_separator;
    int16_t currency_group_separator;
    int16_t number_decimal_separator;
    int16_t number_group_separator;
    int16_t currency_symbol;
    int16_t percent_symbol;
    int16_t nan_symbol;
    int16_t per_mille_symbol;
    int16_t negative_infinity_symbol;
    int16_t positive_infinity_symbol;
    int16_t negative_sign;
    int16_t positive_sign;
    int8_t currency_negative_pattern;
    int8_t currency_positive_pattern;
    int8_t percent_negative_pattern;
    int8_t percent_positive_pattern;
    int8_t number_negative_pattern;
    int8_t currency_decimal_digits;
    int8_t number_decimal_digits;
    int32_t currency_group_sizes[2];
    int32_t number_group_sizes[2];
};

#include "culture_locale_strings.inc"

// Values from mono number_format_entries; leanclr number_index maps via find_number_format_entry().
static const NumberFormatEntryNative s_number_format_en = {
    11, 1278, 11, 1278, 1462, 1345, 1347, 1351, 1313, 1323, 55, 1355, 0, 0, 0, 0, 1, 2, 2, {3, -1}, {3, -1},
};
static const NumberFormatEntryNative s_number_format_en_us = s_number_format_en;
static const NumberFormatEntryNative s_number_format_zh_cn = {
    11, 1278, 11, 1278, 1537, 1345, 1347, 1310, 1313, 1323, 55, 1355, 2, 0, 1, 1, 1, 2, 2, {3, -1}, {3, -1},
};
static const NumberFormatEntryNative s_number_format_zh = {
    11, 1278, 11, 1278, 1378, 1345, 1347, 1310, 1313, 1323, 55, 1355, 2, 0, 1, 1, 1, 2, 2, {3, -1}, {3, -1},
};

static const NumberFormatEntryNative* find_number_format_entry(int32_t number_index)
{
    switch (number_index)
    {
    case 9:
        return &s_number_format_en;
    case 105:
        return &s_number_format_en_us;
    case 187:
        return &s_number_format_zh_cn;
    case 268:
        return &s_number_format_zh;
    default:
        return nullptr;
    }
}

static void assign_number_format_entry(NumberFormatEntryManaged* managed, const NumberFormatEntryNative& native)
{
    managed->currency_decimal_digits = native.currency_decimal_digits;
    managed->currency_decimal_separator = native.currency_decimal_separator;
    managed->currency_group_separator = native.currency_group_separator;
    managed->currency_group_sizes0 = native.currency_group_sizes[0];
    managed->currency_group_sizes1 = native.currency_group_sizes[1];
    managed->currency_negative_pattern = native.currency_negative_pattern;
    managed->currency_positive_pattern = native.currency_positive_pattern;
    managed->currency_symbol = native.currency_symbol;
    managed->nan_symbol = native.nan_symbol;
    managed->negative_infinity_symbol = native.negative_infinity_symbol;
    managed->negative_sign = native.negative_sign;
    managed->number_decimal_digits = native.number_decimal_digits;
    managed->number_decimal_separator = native.number_decimal_separator;
    managed->number_group_separator = native.number_group_separator;
    managed->number_group_sizes0 = native.number_group_sizes[0];
    managed->number_group_sizes1 = native.number_group_sizes[1];
    managed->number_negative_pattern = native.number_negative_pattern;
    managed->per_mille_symbol = native.per_mille_symbol;
    managed->percent_negative_pattern = native.percent_negative_pattern;
    managed->percent_positive_pattern = native.percent_positive_pattern;
    managed->percent_symbol = native.percent_symbol;
    managed->positive_infinity_symbol = native.positive_infinity_symbol;
    managed->positive_sign = native.positive_sign;
}

static void assign_datetime_culture_data(vm::RtCultureData* culture_data, const DateTimeCultureFillData& data)
{
    culture_data->sm1159 = vm::String::create_string_from_utf8cstr(data.am_designator);
    culture_data->pm2359 = vm::String::create_string_from_utf8cstr(data.pm_designator);
    culture_data->time_separator = vm::String::create_string_from_utf8cstr(data.time_separator);
    culture_data->first_day_of_week = data.first_day_of_week;
    culture_data->first_week_of_year = data.first_week_of_year;

    auto long_times = create_string_array(data.long_times, data.long_times_count);
    if (long_times.is_ok())
    {
        culture_data->long_times = long_times.unwrap();
    }
    auto short_times = create_string_array(data.short_times, data.short_times_count);
    if (short_times.is_ok())
    {
        culture_data->short_times = short_times.unwrap();
    }
}

} // namespace

RtResult<const uint8_t*> SystemGlobalizationCultureData::fill_number_data(int32_t number_index, void* managed) noexcept
{
    auto* managed_entry = reinterpret_cast<NumberFormatEntryManaged*>(managed);
    if (managed_entry == nullptr)
    {
        RET_OK(reinterpret_cast<const uint8_t*>(s_locale_strings));
    }

    const NumberFormatEntryNative* native = find_number_format_entry(number_index);
    if (native == nullptr)
    {
        native = &s_number_format_en;
    }

    assign_number_format_entry(managed_entry, *native);
    RET_OK(reinterpret_cast<const uint8_t*>(s_locale_strings));
}

RtResultVoid SystemGlobalizationCultureData::fill_culture_data(vm::RtCultureData* _this, int32_t datetime_index) noexcept
{
    if (_this == nullptr)
    {
        RET_VOID_OK();
    }

    const DateTimeCultureFillData* data = find_datetime_culture_data(datetime_index);
    if (data == nullptr)
    {
        data = &s_datetime_en;
    }

    assign_datetime_culture_data(_this, *data);
    RET_VOID_OK();
}

/// @icall: System.Globalization.CultureData::fill_culture_data(System.Int32)
static RtResultVoid fill_culture_data_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* /*ret*/) noexcept
{
    auto _this = EvalStackOp::get_param<vm::RtCultureData*>(params, 0);
    auto datetime_index = EvalStackOp::get_param<int32_t>(params, 1);
    return SystemGlobalizationCultureData::fill_culture_data(_this, datetime_index);
}

/// @icall: System.Globalization.CultureData::fill_number_data(System.Int32,System.Globalization.CultureData/NumberFormatEntryManaged&)
static RtResultVoid fill_number_data_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto number_index = EvalStackOp::get_param<int32_t>(params, 0);
    auto managed = EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const uint8_t*, locale_strings, SystemGlobalizationCultureData::fill_number_data(number_index, managed));
    EvalStackOp::set_return(ret, locale_strings);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_globalization_culturedata[] = {
    {"System.Globalization.CultureData::fill_culture_data(System.Int32)",
     (vm::InternalCallFunction)&SystemGlobalizationCultureData::fill_culture_data, fill_culture_data_invoker},
    {"System.Globalization.CultureData::fill_number_data(System.Int32,System.Globalization.CultureData/NumberFormatEntryManaged&)",
     (vm::InternalCallFunction)&SystemGlobalizationCultureData::fill_number_data, fill_number_data_invoker},
};

utils::Span<vm::InternalCallEntry> SystemGlobalizationCultureData::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_globalization_culturedata,
                                              sizeof(s_internal_call_entries_system_globalization_culturedata) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

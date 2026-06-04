#include "system_globalization_cultureinfo.h"
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

// Layout must match System.Globalization.CultureInfo/Data in corlib.
struct CultureTextInfoData
{
    int32_t ansi;
    int32_t ebcdic;
    int32_t mac;
    int32_t oem;
    bool right_to_left;
    uint8_t list_sep;
};

struct KnownCulture
{
    const char* match_name;
    const char* canonical_name;
    int32_t lcid;
    int32_t parent_lcid;
    int32_t datetime_index;
    int32_t number_index;
    int32_t default_calendar_type;
    const char* english_name;
    const char* native_name;
    const char* iso2;
    const char* iso3;
    const char* win3;
    const char* territory;
    const CultureTextInfoData* text_info;
};

static const CultureTextInfoData s_text_info_en = {1252, 37, 10000, 437, false, ','};
static const CultureTextInfoData s_text_info_en_us = {1252, 37, 10000, 437, false, ','};
static const CultureTextInfoData s_text_info_zh = {936, 500, 10008, 936, false, ','};
static const CultureTextInfoData s_text_info_zh_cn = {936, 500, 10008, 936, false, ','};

// Indices and LCIDs aligned with mono/metadata/culture-info-tables.h (culture_entries).
static const KnownCulture s_known_cultures[] = {
    {"en", "en", 0x0009, 0x007F, 9, 9, 257, "English", "English", "en", "eng", "ENU", "", &s_text_info_en},
    {"en-us", "en-US", 0x0409, 0x0009, 105, 105, 257, "English (United States)", "English (United States)", "en", "eng", "ENU", "US",
     &s_text_info_en_us},
    {"zh", "zh", 0x7804, 0x007F, 268, 268, 257, "Chinese", "Chinese", "zh", "zho", "CHS", "", &s_text_info_zh},
    {"zh-cn", "zh-CN", 0x0804, 0x0004, 187, 187, 257, "Chinese (Simplified)", "Chinese (Simplified)", "zh", "zho", "CHS", "CN", &s_text_info_zh_cn},
};

static void assign_known_culture(vm::RtCultureInfo* ci, const KnownCulture& culture)
{
    ci->lcid = culture.lcid;
    ci->parent_lcid = culture.parent_lcid;
    ci->datetime_index = culture.datetime_index;
    ci->number_index = culture.number_index;
    ci->default_calendar_type = culture.default_calendar_type;
    ci->text_info_data = reinterpret_cast<const char*>(culture.text_info);

    ci->name = vm::String::create_string_from_utf8cstr(culture.canonical_name);
    ci->englishname = vm::String::create_string_from_utf8cstr(culture.english_name);
    ci->nativename = vm::String::create_string_from_utf8cstr(culture.native_name);
    ci->iso2lang = vm::String::create_string_from_utf8cstr(culture.iso2);
    ci->iso3lang = vm::String::create_string_from_utf8cstr(culture.iso3);
    ci->win3lang = vm::String::create_string_from_utf8cstr(culture.win3);
    ci->territory = culture.territory[0] != '\0' ? vm::String::create_string_from_utf8cstr(culture.territory) : nullptr;
}

static const KnownCulture* find_known_culture_by_name(const char* name)
{
    if (name == nullptr || name[0] == '\0')
    {
        return nullptr;
    }

    for (const KnownCulture& culture : s_known_cultures)
    {
        if (utils::StringUtil::equals_ignorecase(name, culture.match_name))
        {
            return &culture;
        }
    }
    return nullptr;
}

static const KnownCulture* find_known_culture_by_lcid(int32_t lcid)
{
    for (const KnownCulture& culture : s_known_cultures)
    {
        if (culture.lcid == lcid)
        {
            return &culture;
        }
    }
    return nullptr;
}

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

RtResult<bool> SystemGlobalizationCultureInfo::construct_internal_locale_from_lcid(vm::RtCultureInfo* _this, int32_t culture_lcid) noexcept
{
    if (_this == nullptr)
    {
        RET_OK(false);
    }

    const KnownCulture* culture = find_known_culture_by_lcid(culture_lcid);
    if (culture == nullptr)
    {
        RET_OK(false);
    }

    assign_known_culture(_this, *culture);
    RET_OK(true);
}

/// @icall: System.Globalization.CultureInfo::construct_internal_locale_from_lcid(System.Int32)
static RtResultVoid construct_internal_locale_from_lcid_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto _this = EvalStackOp::get_param<vm::RtCultureInfo*>(params, 0);
    auto culture_lcid = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemGlobalizationCultureInfo::construct_internal_locale_from_lcid(_this, culture_lcid));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

RtResult<bool> SystemGlobalizationCultureInfo::construct_internal_locale_from_name(vm::RtCultureInfo* _this, vm::RtString* name) noexcept
{
    if (_this == nullptr || name == nullptr)
    {
        RET_OK(false);
    }

    utils::Utf8StringBuilder name_utf8(vm::String::get_chars_ptr(name), static_cast<size_t>(vm::String::get_length(name)));
    const KnownCulture* culture = find_known_culture_by_name(name_utf8.get_const_chars());
    if (culture == nullptr)
    {
        RET_OK(false);
    }

    assign_known_culture(_this, *culture);
    RET_OK(true);
}

/// @icall: System.Globalization.CultureInfo::construct_internal_locale_from_name(System.String)
static RtResultVoid construct_internal_locale_from_name_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto _this = EvalStackOp::get_param<vm::RtCultureInfo*>(params, 0);
    auto name = EvalStackOp::get_param<vm::RtString*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemGlobalizationCultureInfo::construct_internal_locale_from_name(_this, name));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemGlobalizationCultureInfo::get_current_locale_name() noexcept
{
    RET_OK(vm::String::get_empty_string());
}

/// @icall: System.Globalization.CultureInfo::get_current_locale_name
static RtResultVoid get_current_locale_name_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                    const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, locale_name, SystemGlobalizationCultureInfo::get_current_locale_name());
    EvalStackOp::set_return(ret, locale_name);
    RET_VOID_OK();
}

RtResult<vm::RtArray*> SystemGlobalizationCultureInfo::internal_get_cultures(bool /*neutral*/, bool /*specific*/, bool /*installed*/) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResultVoid SystemGlobalizationCultureInfo::set_user_preferred_culture_info_in_app_x(vm::RtCultureInfo* /*_this*/, vm::RtString* /*name*/) noexcept
{
    RET_VOID_OK();
}

RtResultVoid SystemGlobalizationCultureInfo::initialize_user_preferred_culture_info_in_app_x(
    vm::RtMulticastDelegate* /*onCultureInfoChangedInAppX*/) noexcept
{
    WARN_NOT_IMPLEMENTED_ERROR_THEN_RETURN_VOID("System.Globalization.CultureInfo::initialize_user_preferred_culture_info_in_app_x");
}

RtResult<const uint8_t*> SystemGlobalizationCultureInfo::fill_number_data(int32_t number_index, void* managed) noexcept
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

RtResultVoid SystemGlobalizationCultureInfo::fill_culture_data(vm::RtCultureData* _this, int32_t datetime_index) noexcept
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
    return SystemGlobalizationCultureInfo::fill_culture_data(_this, datetime_index);
}

/// @icall: System.Globalization.CultureData::fill_number_data(System.Int32,System.Globalization.CultureData/NumberFormatEntryManaged&)
static RtResultVoid fill_number_data_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto number_index = EvalStackOp::get_param<int32_t>(params, 0);
    auto managed = EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const uint8_t*, locale_strings, SystemGlobalizationCultureInfo::fill_number_data(number_index, managed));
    EvalStackOp::set_return(ret, locale_strings);
    RET_VOID_OK();
}

/// @icall: System.Globalization.CultureInfo::SetUserPreferredCultureInfoInAppX(System.String)
static RtResultVoid set_user_preferred_culture_info_in_app_x_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                     const interp::RtStackObject* params, interp::RtStackObject* /*ret*/) noexcept
{
    auto _this = EvalStackOp::get_param<vm::RtCultureInfo*>(params, 0);
    auto name = EvalStackOp::get_param<vm::RtString*>(params, 1);
    return SystemGlobalizationCultureInfo::set_user_preferred_culture_info_in_app_x(_this, name);
}

/// @icall: System.Globalization.CultureInfo::InitializeUserPreferredCultureInfoInAppX(System.Globalization.CultureInfo/OnCultureInfoChangedDelegate)
static RtResultVoid initialize_user_preferred_culture_info_in_app_x_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                            const interp::RtStackObject* params, interp::RtStackObject* /*ret*/) noexcept
{
    auto on_changed = EvalStackOp::get_param<vm::RtMulticastDelegate*>(params, 0);
    return SystemGlobalizationCultureInfo::initialize_user_preferred_culture_info_in_app_x(on_changed);
}

/// @icall: System.Globalization.CultureInfo::internal_get_cultures(System.Boolean,System.Boolean,System.Boolean)
static RtResultVoid internal_get_cultures_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto neutral = EvalStackOp::get_param<bool>(params, 0);
    auto specific = EvalStackOp::get_param<bool>(params, 1);
    auto installed = EvalStackOp::get_param<bool>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, cultures, SystemGlobalizationCultureInfo::internal_get_cultures(neutral, specific, installed));
    EvalStackOp::set_return(ret, cultures);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_globalization_cultureinfo[] = {
    {"System.Globalization.CultureData::fill_culture_data(System.Int32)",
     (vm::InternalCallFunction)&SystemGlobalizationCultureInfo::fill_culture_data, fill_culture_data_invoker},
    {"System.Globalization.CultureData::fill_number_data(System.Int32,System.Globalization.CultureData/NumberFormatEntryManaged&)",
     (vm::InternalCallFunction)&SystemGlobalizationCultureInfo::fill_number_data, fill_number_data_invoker},
    {"System.Globalization.CultureInfo::construct_internal_locale_from_lcid(System.Int32)",
     (vm::InternalCallFunction)&SystemGlobalizationCultureInfo::construct_internal_locale_from_lcid, construct_internal_locale_from_lcid_invoker},
    {"System.Globalization.CultureInfo::construct_internal_locale_from_name(System.String)",
     (vm::InternalCallFunction)&SystemGlobalizationCultureInfo::construct_internal_locale_from_name, construct_internal_locale_from_name_invoker},
    {"System.Globalization.CultureInfo::get_current_locale_name", (vm::InternalCallFunction)&SystemGlobalizationCultureInfo::get_current_locale_name,
     get_current_locale_name_invoker},
    {"System.Globalization.CultureInfo::internal_get_cultures(System.Boolean,System.Boolean,System.Boolean)",
     (vm::InternalCallFunction)&SystemGlobalizationCultureInfo::internal_get_cultures, internal_get_cultures_invoker},
    {"System.Globalization.CultureInfo::SetUserPreferredCultureInfoInAppX(System.String)",
     (vm::InternalCallFunction)&SystemGlobalizationCultureInfo::set_user_preferred_culture_info_in_app_x, set_user_preferred_culture_info_in_app_x_invoker},
    {"System.Globalization.CultureInfo::InitializeUserPreferredCultureInfoInAppX(System.Globalization.CultureInfo/OnCultureInfoChangedDelegate)",
     (vm::InternalCallFunction)&SystemGlobalizationCultureInfo::initialize_user_preferred_culture_info_in_app_x,
     initialize_user_preferred_culture_info_in_app_x_invoker},
};

utils::Span<vm::InternalCallEntry> SystemGlobalizationCultureInfo::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_globalization_cultureinfo,
                                              sizeof(s_internal_call_entries_system_globalization_cultureinfo) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

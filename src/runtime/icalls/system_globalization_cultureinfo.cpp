#include "system_globalization_cultureinfo.h"
#include "icall_base.h"

#include "utils/string_builder.h"
#include "utils/string_util.h"
#include "vm/class.h"
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

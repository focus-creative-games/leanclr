#include "system_globalization_cultureinfo.h"
#include "icall_base.h"

#include "vm/rt_string.h"

namespace leanclr
{
namespace icalls
{

RtResult<bool> SystemGlobalizationCultureInfo::construct_internal_locale_from_lcid(vm::RtCultureInfo* _this, int32_t /*culture_lcid*/) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
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

RtResult<bool> SystemGlobalizationCultureInfo::construct_internal_locale_from_name(vm::RtCultureInfo* _this, vm::RtString* /*name*/) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
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

/// @icall: System.Globalization.CultureInfo::SetUserPreferredCultureInfoInAppX(System.String)
static RtResultVoid set_user_preferred_culture_info_in_app_x_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                     const interp::RtStackObject* params, interp::RtStackObject* /*ret*/) noexcept
{
    auto _this = EvalStackOp::get_param<vm::RtCultureInfo*>(params, 0);
    auto name = EvalStackOp::get_param<vm::RtString*>(params, 1);
    return SystemGlobalizationCultureInfo::set_user_preferred_culture_info_in_app_x(_this, name);
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
};

utils::Span<vm::InternalCallEntry> SystemGlobalizationCultureInfo::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_globalization_cultureinfo,
                                              sizeof(s_internal_call_entries_system_globalization_cultureinfo) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

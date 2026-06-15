#include "leanclr_profile.h"

#include "profile/profile.h"
#include "utils/string_builder.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace icalls
{

RtResultVoid LeanCLRProfile::reset_period_stats() noexcept
{
    profile::Profile::reset_period();
    RET_VOID_OK();
}

RtResultVoid LeanCLRProfile::end_period_stats() noexcept
{
    profile::Profile::end_period();
    RET_VOID_OK();
}

RtResultVoid LeanCLRProfile::reset_global_stats() noexcept
{
    profile::Profile::reset_global();
    RET_VOID_OK();
}

RtResult<vm::RtString*> LeanCLRProfile::get_period_stats() noexcept
{
    return get_period_stats_json();
}

RtResult<vm::RtString*> LeanCLRProfile::get_global_stats() noexcept
{
    return get_global_stats_json();
}

RtResult<vm::RtString*> LeanCLRProfile::get_period_stats_json() noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::string, json, profile::Profile::get_period_stats_json());
    RET_OK(vm::String::create_string_from_utf8cstr(json.c_str()));
}

RtResult<vm::RtString*> LeanCLRProfile::get_global_stats_json() noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::string, json, profile::Profile::get_global_stats_json());
    RET_OK(vm::String::create_string_from_utf8cstr(json.c_str()));
}

RtResultVoid LeanCLRProfile::export_period_stats_json(vm::RtString* path) noexcept
{
    if (path == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    utils::Utf8StringBuilder path_utf8(vm::String::get_chars_ptr(path), static_cast<size_t>(vm::String::get_length(path)));
    return profile::Profile::export_period_stats_json(path_utf8.get_const_chars());
}

RtResultVoid LeanCLRProfile::export_global_stats_json(vm::RtString* path) noexcept
{
    if (path == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    utils::Utf8StringBuilder path_utf8(vm::String::get_chars_ptr(path), static_cast<size_t>(vm::String::get_length(path)));
    return profile::Profile::export_global_stats_json(path_utf8.get_const_chars());
}

/// @icall: LeanCLR.Profile::ResetPeriodStats
static RtResultVoid reset_period_stats_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                               interp::RtStackObject*) noexcept
{
    return LeanCLRProfile::reset_period_stats();
}

/// @icall: LeanCLR.Profile::EndPeriodStats
static RtResultVoid end_period_stats_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                             interp::RtStackObject*) noexcept
{
    return LeanCLRProfile::end_period_stats();
}

/// @icall: LeanCLR.Profile::ResetGlobalStats
static RtResultVoid reset_global_stats_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                               interp::RtStackObject*) noexcept
{
    return LeanCLRProfile::reset_global_stats();
}

/// @icall: LeanCLR.Profile::GetPeriodStats
static RtResultVoid get_period_stats_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                             interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, json, LeanCLRProfile::get_period_stats());
    EvalStackOp::set_return(ret, json);
    RET_VOID_OK();
}

/// @icall: LeanCLR.Profile::GetGlobalStats
static RtResultVoid get_global_stats_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                             interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, json, LeanCLRProfile::get_global_stats());
    EvalStackOp::set_return(ret, json);
    RET_VOID_OK();
}

/// @icall: LeanCLR.Profile::GetPeriodStatsJson
static RtResultVoid get_period_stats_json_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                                  interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, json, LeanCLRProfile::get_period_stats_json());
    EvalStackOp::set_return(ret, json);
    RET_VOID_OK();
}

/// @icall: LeanCLR.Profile::ExportPeriodStatsJson
static RtResultVoid export_period_stats_json_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                     interp::RtStackObject*) noexcept
{
    vm::RtString* path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    return LeanCLRProfile::export_period_stats_json(path);
}

/// @icall: LeanCLR.Profile::GetGlobalStatsJson
static RtResultVoid get_global_stats_json_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject*,
                                                  interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, json, LeanCLRProfile::get_global_stats_json());
    EvalStackOp::set_return(ret, json);
    RET_VOID_OK();
}

/// @icall: LeanCLR.Profile::ExportGlobalStatsJson
static RtResultVoid export_global_stats_json_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                     interp::RtStackObject*) noexcept
{
    vm::RtString* path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    return LeanCLRProfile::export_global_stats_json(path);
}

static vm::InternalCallEntry s_internal_call_entries_leanclr_profile[] = {
    {"LeanCLR.Profile::ResetPeriodStats", (vm::InternalCallFunction)&LeanCLRProfile::reset_period_stats, reset_period_stats_invoker},
    {"LeanCLR.Profile::EndPeriodStats", (vm::InternalCallFunction)&LeanCLRProfile::end_period_stats, end_period_stats_invoker},
    {"LeanCLR.Profile::ResetGlobalStats", (vm::InternalCallFunction)&LeanCLRProfile::reset_global_stats, reset_global_stats_invoker},
    {"LeanCLR.Profile::GetPeriodStats", (vm::InternalCallFunction)&LeanCLRProfile::get_period_stats, get_period_stats_invoker},
    {"LeanCLR.Profile::GetGlobalStats", (vm::InternalCallFunction)&LeanCLRProfile::get_global_stats, get_global_stats_invoker},
    {"LeanCLR.Profile::GetPeriodStatsJson", (vm::InternalCallFunction)&LeanCLRProfile::get_period_stats_json, get_period_stats_json_invoker},
    {"LeanCLR.Profile::ExportPeriodStatsJson", (vm::InternalCallFunction)&LeanCLRProfile::export_period_stats_json, export_period_stats_json_invoker},
    {"LeanCLR.Profile::GetGlobalStatsJson", (vm::InternalCallFunction)&LeanCLRProfile::get_global_stats_json, get_global_stats_json_invoker},
    {"LeanCLR.Profile::ExportGlobalStatsJson", (vm::InternalCallFunction)&LeanCLRProfile::export_global_stats_json, export_global_stats_json_invoker},
};

utils::Span<vm::InternalCallEntry> LeanCLRProfile::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_leanclr_profile,
                                              sizeof(s_internal_call_entries_leanclr_profile) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

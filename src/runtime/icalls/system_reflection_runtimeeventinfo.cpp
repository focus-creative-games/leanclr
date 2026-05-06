#include "system_reflection_runtimeeventinfo.h"

#include "vm/class.h"
#include "vm/reflection.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace icalls
{

RtResultVoid SystemReflectionRuntimeEventInfo::get_event_info(vm::RtReflectionEventInfo* ref_event, vm::RtReflectionMonoEventInfo* ref_event_info) noexcept
{
    const metadata::RtEventInfo* event_info = ref_event->event;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, reflected_klass, vm::Class::get_class_from_typesig(ref_event->ref_type->type_handle));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, declaring_type, vm::Reflection::get_klass_reflection_object(event_info->parent));
    ref_event_info->declaring_type = declaring_type;

    ref_event_info->name = vm::String::create_string_from_utf8cstr(event_info->name);

    if (event_info->add_method != nullptr)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, add_method,
                                                vm::Reflection::get_method_reflection_object(event_info->add_method, reflected_klass));
        ref_event_info->add_method = add_method;
    }
    else
    {
        ref_event_info->add_method = nullptr;
    }

    if (event_info->remove_method != nullptr)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, remove_method,
                                                vm::Reflection::get_method_reflection_object(event_info->remove_method, reflected_klass));
        ref_event_info->remove_method = remove_method;
    }
    else
    {
        ref_event_info->remove_method = nullptr;
    }

    if (event_info->raise_method != nullptr)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, raise_method,
                                                vm::Reflection::get_method_reflection_object(event_info->raise_method, reflected_klass));
        ref_event_info->raise_method = raise_method;
    }
    else
    {
        ref_event_info->raise_method = nullptr;
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, other_methods,
                                            vm::Array::new_empty_szarray_by_ele_klass(vm::Class::get_corlib_types().cls_reflection_method));
    ref_event_info->other_methods = other_methods;

    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeEventInfo::get_event_info(System.Reflection.RuntimeEventInfo,System.Reflection.MonoEventInfo&)
static RtResultVoid get_event_info_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto evt = EvalStackOp::get_param<vm::RtReflectionEventInfo*>(params, 0);
    auto out_info = EvalStackOp::get_param<vm::RtReflectionMonoEventInfo*>(params, 1);
    RET_ERR_ON_FAIL(SystemReflectionRuntimeEventInfo::get_event_info(evt, out_info));
    RET_VOID_OK();
}

RtResult<int32_t> SystemReflectionRuntimeEventInfo::get_metadata_token(vm::RtReflectionEventInfo* event_info) noexcept
{
    const metadata::RtEventInfo* event = event_info->event;
    RET_OK(static_cast<int32_t>(event->token));
}

/// @icall: System.Reflection.RuntimeEventInfo::get_metadata_token(System.Reflection.RuntimeEventInfo)
static RtResultVoid get_metadata_token_invoker_system_reflection_runtimeeventinfo(metadata::RtManagedMethodPointer methodPtr,
                                                                                  const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                                                                  interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto evt = EvalStackOp::get_param<vm::RtReflectionEventInfo*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, token, SystemReflectionRuntimeEventInfo::get_metadata_token(evt));
    EvalStackOp::set_return(ret, token);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemReflectionRuntimeEventInfo::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Reflection.RuntimeEventInfo::get_event_info(System.Reflection.RuntimeEventInfo,System.Reflection.MonoEventInfo&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeEventInfo::get_event_info, get_event_info_invoker},
        {"System.Reflection.RuntimeEventInfo::get_metadata_token(System.Reflection.RuntimeEventInfo)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeEventInfo::get_metadata_token, get_metadata_token_invoker_system_reflection_runtimeeventinfo},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

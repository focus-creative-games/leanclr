#include "system_reflection_eventinfo.h"

#include "vm/class.h"
#include "vm/reflection.h"

namespace leanclr
{
namespace icalls
{

RtResult<vm::RtReflectionEventInfo*> SystemReflectionEventInfo::internal_from_handle_type(metadata::RtEventInfo* event,
                                                                                          const metadata::RtTypeSig* type_sig) noexcept
{
    if (event == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    const metadata::RtClass* klass;
    if (!type_sig)
    {
        klass = event->parent;
    }
    else
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, resolved, vm::Class::get_class_from_typesig(type_sig));
        RET_ERR_ON_FAIL(vm::Class::initialize_super_types(resolved));
        if (!vm::Class::has_class_parent_fast(resolved, event->parent))
        {
            RET_OK(nullptr);
        }
        klass = resolved;
    }
    return vm::Reflection::get_event_reflection_object(event, klass);
}

/// @icall: System.Reflection.EventInfo::internal_from_handle_type
static RtResultVoid internal_from_handle_type_invoker_eventinfo(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto event_arg = EvalStackOp::get_param<const void*>(params, 0);
    auto type_arg = EvalStackOp::get_param<const void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtEventInfo*, event, vm::Reflection::get_event_info_from_handle_arg(event_arg));
    const metadata::RtTypeSig* type_sig = nullptr;
    if (type_arg != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(type_sig, vm::Reflection::get_type_sig_from_runtime_type_handle_arg(type_arg));
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionEventInfo*, ref_event, SystemReflectionEventInfo::internal_from_handle_type(event, type_sig));
    EvalStackOp::set_return(ret, ref_event);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemReflectionEventInfo::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Reflection.EventInfo::internal_from_handle_type", (vm::InternalCallFunction)&SystemReflectionEventInfo::internal_from_handle_type,
         internal_from_handle_type_invoker_eventinfo},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

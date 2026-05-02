#include "system_type.h"

#include "icall_base.h"
#include "vm/class.h"
#include "vm/reflection.h"

namespace leanclr
{
namespace icalls
{

RtResult<vm::RtReflectionType*> SystemType::internal_from_handle(intptr_t handle) noexcept
{
    const metadata::RtTypeSig* type_sig = reinterpret_cast<const metadata::RtTypeSig*>(handle);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, reflection_obj, vm::Reflection::get_klass_reflection_object(klass));
    RET_OK(reflection_obj);
}

/// @icall: System.Type::internal_from_handle
static RtResultVoid internal_from_handle_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto handle = EvalStackOp::get_param<intptr_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, type_obj, SystemType::internal_from_handle(handle));
    EvalStackOp::set_return(ret, type_obj);
    RET_VOID_OK();
}

// Internal call registry
static vm::InternalCallEntry s_internal_call_entries_system_type[] = {
    {"System.Type::internal_from_handle", (vm::InternalCallFunction)&SystemType::internal_from_handle, internal_from_handle_invoker},
};

utils::Span<vm::InternalCallEntry> SystemType::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_type, sizeof(s_internal_call_entries_system_type) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

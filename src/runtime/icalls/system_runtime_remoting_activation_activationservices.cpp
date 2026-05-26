#include "system_runtime_remoting_activation_activationservices.h"

#include "vm/class.h"
#include "vm/object.h"

namespace leanclr
{
namespace icalls
{

RtResult<vm::RtObject*> SystemRuntimeRemotingActivationActivationServices::allocate_uninitialized_class_instance(vm::RtReflectionRuntimeType* type) noexcept
{
    if (type == nullptr)
    {
        RET_OK(nullptr);
    }

    const metadata::RtTypeSig* type_sig = type->reflection_type.type_handle;
    return LEANCLR_CREATE_INSTANCE_INTERNAL(type_sig, "ActivationServices::AllocateUninitializedClassInstance");
}

/// @icall: System.Runtime.Remoting.Activation.ActivationServices::AllocateUninitializedClassInstance(System.Type)
static RtResultVoid allocate_uninitialized_class_instance_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, SystemRuntimeRemotingActivationActivationServices::allocate_uninitialized_class_instance(type));
    EvalStackOp::set_return(ret, obj);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_runtime_remoting_activation_activationservices[] = {
    {"System.Runtime.Remoting.Activation.ActivationServices::AllocateUninitializedClassInstance(System.Type)",
     (vm::InternalCallFunction)&SystemRuntimeRemotingActivationActivationServices::allocate_uninitialized_class_instance,
     allocate_uninitialized_class_instance_invoker},
};

utils::Span<vm::InternalCallEntry> SystemRuntimeRemotingActivationActivationServices::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_runtime_remoting_activation_activationservices,
                                              sizeof(s_internal_call_entries_system_runtime_remoting_activation_activationservices) /
                                                  sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

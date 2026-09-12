#include "system_delegate.h"

#include "icall_base.h"
#include "vm/rt_managed_types.h"
#include "vm/delegate.h"
#include "vm/method.h"
#include "vm/object.h"
#include "vm/class.h"
#include "vm/reflection.h"

namespace leanclr
{
namespace icalls
{

RtResult<vm::RtReflectionMethod*> SystemDelegate::get_virtual_method_internal(vm::RtDelegate* this_delegate) noexcept
{
    const metadata::RtMethodInfo* delegate_method = vm::Delegate::get_target_method(this_delegate);
    if (delegate_method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    // If there's a target object, get the virtual method implementation from it
    if (this_delegate->target != nullptr)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, virtual_method,
                                                vm::Method::get_virtual_method_impl(this_delegate->target, delegate_method));
        delegate_method = virtual_method;
    }

    // Get the reflection object for the virtual method
    return vm::Reflection::get_method_reflection_object(delegate_method, delegate_method->parent);
}

/// @icall: System.Delegate::GetVirtualMethod_internal()
static RtResultVoid get_virtual_method_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto this_delegate = EvalStackOp::get_param<vm::RtDelegate*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, reflection_method, SystemDelegate::get_virtual_method_internal(this_delegate));
    EvalStackOp::set_return(ret, reflection_method);
    RET_VOID_OK();
}

RtResult<vm::RtMulticastDelegate*> SystemDelegate::create_delegate_internal(vm::RtReflectionType* delegate_type, vm::RtObject* target,
                                                                            vm::RtReflectionMethod* method, bool throw_on_bind) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method_info,
                                            vm::Reflection::get_method_info_from_reflection_object(method));
    return vm::Delegate::create_delegate_from_reflection(delegate_type, target, method_info, throw_on_bind);
}

/// @icall: System.Delegate::CreateDelegate_internal(System.Type,System.Object,System.Reflection.MethodInfo,System.Boolean)
static RtResultVoid create_delegate_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto delegate_type = EvalStackOp::get_param<vm::RtReflectionType*>(params, 0);
    auto target = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    auto method_info = EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 2);
    auto throw_on_bind = EvalStackOp::get_param<bool>(params, 3);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(vm::RtMulticastDelegate*, ret_del,
                                             SystemDelegate::create_delegate_internal(delegate_type, target, method_info, throw_on_bind));
    EvalStackOp::set_return(ret, ret_del);
    RET_VOID_OK();
}

RtResult<vm::RtMulticastDelegate*> SystemDelegate::alloc_delegate_like_internal(vm::RtDelegate* source) noexcept
{
    // Get the delegate's class from the object header
    const metadata::RtClass* del_klass = source->klass;

    // Allocate a new object of the same class
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, new_obj, LEANCLR_NEWOBJ_INTERNAL(del_klass, "SystemDelegate::alloc_delegate_like_internal"));
    vm::RtMulticastDelegate* new_del = reinterpret_cast<vm::RtMulticastDelegate*>(new_obj);

    // Copy the runtime-visible delegate fields. CoreLib may overwrite them after allocation.
    new_del->dele.target = source->target;
    new_del->dele.method_base = source->method_base;
    new_del->dele.method_ptr = source->method_ptr;
    new_del->dele.method_ptr_aux = source->method_ptr_aux;
    auto source_multicast = reinterpret_cast<vm::RtMulticastDelegate*>(source);
    new_del->invocation_list = source_multicast->invocation_list;
    new_del->invocation_count = source_multicast->invocation_count;

    RET_OK(new_del);
}

RtResult<void*> SystemDelegate::get_multicast_invoke(const metadata::RtClass* delegate_klass) noexcept
{
    if (delegate_klass == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(reinterpret_cast<void*>(&vm::Delegate::invoke_delegate_invoker));
}

RtResult<void*> SystemDelegate::get_invoke_method(const metadata::RtClass* delegate_klass) noexcept
{
    if (delegate_klass == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(reinterpret_cast<void*>(&vm::Delegate::invoke_delegate_invoker));
}

/// @icall: System.Delegate::AllocDelegateLike_internal(System.Delegate)
static RtResultVoid alloc_delegate_like_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto source = EvalStackOp::get_param<vm::RtDelegate*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtMulticastDelegate*, new_delegate, SystemDelegate::alloc_delegate_like_internal(source));
    EvalStackOp::set_return(ret, new_delegate);
    RET_VOID_OK();
}

/// @icall: System.Delegate::GetMulticastInvoke(System.Runtime.CompilerServices.MethodTable*)
static RtResultVoid get_multicast_invoke_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    auto method_table = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtClass*, delegate_klass,
                                            vm::Reflection::get_class_from_net10_method_table(method_table));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, invoke, SystemDelegate::get_multicast_invoke(delegate_klass));
    EvalStackOp::set_return(ret, invoke);
    RET_VOID_OK();
}

/// @icall: System.Delegate::GetInvokeMethod()
static RtResultVoid get_invoke_method_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    auto method_table = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtClass*, delegate_klass,
                                            vm::Reflection::get_class_from_net10_method_table(method_table));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, invoke, SystemDelegate::get_invoke_method(delegate_klass));
    EvalStackOp::set_return(ret, invoke);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemDelegate::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Delegate::GetVirtualMethod_internal()", (vm::InternalCallFunction)&SystemDelegate::get_virtual_method_internal,
         get_virtual_method_internal_invoker},
        {"System.Delegate::CreateDelegate_internal(System.Type,System.Object,System.Reflection.MethodInfo,System.Boolean)",
         (vm::InternalCallFunction)&SystemDelegate::create_delegate_internal, create_delegate_internal_invoker},
        {"System.Delegate::AllocDelegateLike_internal(System.Delegate)", (vm::InternalCallFunction)&SystemDelegate::alloc_delegate_like_internal,
         alloc_delegate_like_internal_invoker},
        {"System.Delegate::GetMulticastInvoke(System.Runtime.CompilerServices.MethodTable*)",
         (vm::InternalCallFunction)&SystemDelegate::get_multicast_invoke, get_multicast_invoke_invoker},
        {"System.Delegate::GetInvokeMethod(System.Runtime.CompilerServices.MethodTable*)",
         (vm::InternalCallFunction)&SystemDelegate::get_invoke_method, get_invoke_method_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

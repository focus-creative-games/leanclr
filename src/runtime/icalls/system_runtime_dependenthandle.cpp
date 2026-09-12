#include "system_runtime_dependenthandle.h"

#include "vm/gchandle.h"

namespace leanclr
{
namespace icalls
{
using namespace leanclr::interp;
using namespace leanclr::metadata;

RtResult<void*> SystemRuntimeDependentHandle::internal_alloc(vm::RtObject* target, vm::RtObject* dependent) noexcept
{
    RET_OK(vm::GCHandle::new_dependent_handle(target, dependent));
}

RtResult<vm::RtObject*> SystemRuntimeDependentHandle::internal_get_dependent(void* handle) noexcept
{
    RET_OK(vm::GCHandle::get_dependent_handle_dependent(handle));
}

RtResult<vm::RtObject*> SystemRuntimeDependentHandle::internal_get_target_and_dependent(void* handle, vm::RtObject** dependent) noexcept
{
    RET_OK(vm::GCHandle::get_dependent_handle_target_and_dependent(handle, dependent));
}

RtResultVoid SystemRuntimeDependentHandle::internal_set_target_to_null(void* handle) noexcept
{
    vm::GCHandle::set_dependent_handle_target_to_null(handle);
    RET_VOID_OK();
}

RtResultVoid SystemRuntimeDependentHandle::internal_set_dependent(void* handle, vm::RtObject* dependent) noexcept
{
    vm::GCHandle::set_dependent_handle_dependent(handle, dependent);
    RET_VOID_OK();
}

RtResult<bool> SystemRuntimeDependentHandle::internal_free(void* handle) noexcept
{
    RET_OK(vm::GCHandle::free_dependent_handle(handle));
}

/// @icall: System.Runtime.DependentHandle::InternalAlloc
static RtResultVoid internal_alloc_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    vm::RtObject* target = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    vm::RtObject* dependent = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, handle, SystemRuntimeDependentHandle::internal_alloc(target, dependent));
    EvalStackOp::set_return(ret, handle);
    RET_VOID_OK();
}

/// @icall: System.Runtime.DependentHandle::InternalGetDependent
static RtResultVoid internal_get_dependent_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    void* handle = EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, dependent, SystemRuntimeDependentHandle::internal_get_dependent(handle));
    EvalStackOp::set_return(ret, dependent);
    RET_VOID_OK();
}

/// @icall: System.Runtime.DependentHandle::InternalGetTargetAndDependent
static RtResultVoid internal_get_target_and_dependent_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params,
                                                              RtStackObject* ret) noexcept
{
    void* handle = EvalStackOp::get_param<void*>(params, 0);
    vm::RtObject** dependent = EvalStackOp::get_param<vm::RtObject**>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, target,
                                            SystemRuntimeDependentHandle::internal_get_target_and_dependent(handle, dependent));
    EvalStackOp::set_return(ret, target);
    RET_VOID_OK();
}

/// @icall: System.Runtime.DependentHandle::InternalSetTargetToNull
static RtResultVoid internal_set_target_to_null_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params,
                                                        RtStackObject* ret) noexcept
{
    void* handle = EvalStackOp::get_param<void*>(params, 0);
    (void)ret;
    return SystemRuntimeDependentHandle::internal_set_target_to_null(handle);
}

/// @icall: System.Runtime.DependentHandle::InternalSetDependent
static RtResultVoid internal_set_dependent_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    void* handle = EvalStackOp::get_param<void*>(params, 0);
    vm::RtObject* dependent = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    (void)ret;
    return SystemRuntimeDependentHandle::internal_set_dependent(handle, dependent);
}

/// @icall: System.Runtime.DependentHandle::InternalFree
static RtResultVoid internal_free_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    void* handle = EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeDependentHandle::internal_free(handle));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemRuntimeDependentHandle::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Runtime.DependentHandle::InternalAlloc", (vm::InternalCallFunction)&SystemRuntimeDependentHandle::internal_alloc,
         internal_alloc_invoker},
        {"System.Runtime.DependentHandle::InternalAlloc(System.Object,System.Object)",
         (vm::InternalCallFunction)&SystemRuntimeDependentHandle::internal_alloc, internal_alloc_invoker},
        {"System.Runtime.DependentHandle::InternalGetDependent", (vm::InternalCallFunction)&SystemRuntimeDependentHandle::internal_get_dependent,
         internal_get_dependent_invoker},
        {"System.Runtime.DependentHandle::InternalGetDependent(System.IntPtr)",
         (vm::InternalCallFunction)&SystemRuntimeDependentHandle::internal_get_dependent, internal_get_dependent_invoker},
        {"System.Runtime.DependentHandle::InternalGetTargetAndDependent",
         (vm::InternalCallFunction)&SystemRuntimeDependentHandle::internal_get_target_and_dependent, internal_get_target_and_dependent_invoker},
        {"System.Runtime.DependentHandle::InternalGetTargetAndDependent(System.IntPtr,System.Object&)",
         (vm::InternalCallFunction)&SystemRuntimeDependentHandle::internal_get_target_and_dependent, internal_get_target_and_dependent_invoker},
        {"System.Runtime.DependentHandle::InternalSetTargetToNull",
         (vm::InternalCallFunction)&SystemRuntimeDependentHandle::internal_set_target_to_null, internal_set_target_to_null_invoker},
        {"System.Runtime.DependentHandle::InternalSetTargetToNull(System.IntPtr)",
         (vm::InternalCallFunction)&SystemRuntimeDependentHandle::internal_set_target_to_null, internal_set_target_to_null_invoker},
        {"System.Runtime.DependentHandle::InternalSetDependent", (vm::InternalCallFunction)&SystemRuntimeDependentHandle::internal_set_dependent,
         internal_set_dependent_invoker},
        {"System.Runtime.DependentHandle::InternalSetDependent(System.IntPtr,System.Object)",
         (vm::InternalCallFunction)&SystemRuntimeDependentHandle::internal_set_dependent, internal_set_dependent_invoker},
        {"System.Runtime.DependentHandle::InternalFree", (vm::InternalCallFunction)&SystemRuntimeDependentHandle::internal_free, internal_free_invoker},
        {"System.Runtime.DependentHandle::InternalFree(System.IntPtr)",
         (vm::InternalCallFunction)&SystemRuntimeDependentHandle::internal_free, internal_free_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

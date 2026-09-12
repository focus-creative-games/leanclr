#include "system_runtime_interopservices_gchandle.h"
#include "vm/gchandle.h"

namespace leanclr
{
namespace icalls
{

RtResult<bool> SystemRuntimeInteropServicesGCHandle::check_current_domain(vm::GCHandleId handle) noexcept
{
    (void)handle;
    // In WebAssembly, there is only a single AppDomain.
    RET_OK(true);
}

RtResult<vm::RtObject*> SystemRuntimeInteropServicesGCHandle::get_target(vm::GCHandleId handle) noexcept
{
    RET_OK(vm::GCHandle::get_target(vm::GCHandle::get_handle_by_id(handle)));
}

RtResult<vm::GCHandleId> SystemRuntimeInteropServicesGCHandle::get_target_handle(vm::RtObject* obj, vm::GCHandleId handle, int32_t handle_type) noexcept
{
    void* handle_ptr = vm::GCHandle::get_handle_by_id(handle);
    void* result = vm::GCHandle::get_target_handle(obj, handle_ptr, handle_type);
    if (result == nullptr)
    {
        RET_OK(0);
    }
    RET_OK(vm::GCHandle::get_handle_id(result));
}

RtResultVoid SystemRuntimeInteropServicesGCHandle::free_handle(vm::GCHandleId handle) noexcept
{
    vm::GCHandle::free_handle(vm::GCHandle::get_handle_by_id(handle));
    RET_VOID_OK();
}

RtResult<void*> SystemRuntimeInteropServicesGCHandle::get_addr_of_pinned_object(vm::GCHandleId handle) noexcept
{
    RET_OK(vm::GCHandle::get_addr_of_pinned_object(vm::GCHandle::get_handle_by_id(handle)));
}

RtResult<void*> SystemRuntimeInteropServicesGCHandle::internal_alloc(vm::RtObject* obj, int32_t handle_type) noexcept
{
    RET_OK(vm::GCHandle::get_target_handle(obj, nullptr, handle_type));
}

RtResult<bool> SystemRuntimeInteropServicesGCHandle::internal_free(void* handle) noexcept
{
    vm::GCHandle::free_handle(handle);
    RET_OK(true);
}

RtResultVoid SystemRuntimeInteropServicesGCHandle::internal_set(void* handle, vm::RtObject* value) noexcept
{
    auto slot = reinterpret_cast<vm::RtObject**>(handle);
    if (slot == nullptr)
    {
        RET_ERR(RtErr::Argument);
    }
    *slot = value;
    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemRuntimeInteropServicesGCHandle::internal_compare_exchange(void* handle, vm::RtObject* value,
                                                                                        vm::RtObject* old_value) noexcept
{
    auto slot = reinterpret_cast<vm::RtObject**>(handle);
    if (slot == nullptr)
    {
        RET_ERR(RtErr::Argument);
    }

    vm::RtObject* current = *slot;
    if (current == old_value)
    {
        *slot = value;
    }
    RET_OK(current);
}

/// @icall: System.Runtime.InteropServices.GCHandle::CheckCurrentDomain
static RtResultVoid check_current_domain_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto handle = EvalStackOp::get_param<vm::GCHandleId>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeInteropServicesGCHandle::check_current_domain(handle));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.GCHandle::GetTarget
static RtResultVoid get_target_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto handle = EvalStackOp::get_param<vm::GCHandleId>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, SystemRuntimeInteropServicesGCHandle::get_target(handle));
    EvalStackOp::set_return(ret, obj);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.GCHandle::GetTargetHandle
static RtResultVoid get_target_handle_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    auto handle = EvalStackOp::get_param<vm::GCHandleId>(params, 1);
    auto handle_type = EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::GCHandleId, new_handle, SystemRuntimeInteropServicesGCHandle::get_target_handle(obj, handle, handle_type));
    EvalStackOp::set_return(ret, new_handle);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.GCHandle::FreeHandle
static RtResultVoid free_handle_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto handle = EvalStackOp::get_param<int32_t>(params, 0);
    RET_ERR_ON_FAIL(SystemRuntimeInteropServicesGCHandle::free_handle(handle));
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.GCHandle::GetAddrOfPinnedObject(System.Int32)
static RtResultVoid get_addr_of_pinned_object_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                      const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto handle = EvalStackOp::get_param<int32_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, addr, SystemRuntimeInteropServicesGCHandle::get_addr_of_pinned_object(handle));
    EvalStackOp::set_return(ret, addr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.GCHandle::_InternalAlloc
static RtResultVoid internal_alloc_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    auto handle_type = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, handle, SystemRuntimeInteropServicesGCHandle::internal_alloc(obj, handle_type));
    EvalStackOp::set_return(ret, handle);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.GCHandle::_InternalFree
static RtResultVoid internal_free_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                          const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto handle = EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeInteropServicesGCHandle::internal_free(handle));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.GCHandle::InternalSet
static RtResultVoid internal_set_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto handle = EvalStackOp::get_param<void*>(params, 0);
    auto value = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    RET_ERR_ON_FAIL(SystemRuntimeInteropServicesGCHandle::internal_set(handle, value));
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.GCHandle::InternalCompareExchange
static RtResultVoid internal_compare_exchange_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                      const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto handle = EvalStackOp::get_param<void*>(params, 0);
    auto value = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    auto old_value = EvalStackOp::get_param<vm::RtObject*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, current,
                                            SystemRuntimeInteropServicesGCHandle::internal_compare_exchange(handle, value, old_value));
    EvalStackOp::set_return(ret, current);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemRuntimeInteropServicesGCHandle::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Runtime.InteropServices.GCHandle::CheckCurrentDomain(System.Int32)",
         (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::check_current_domain, check_current_domain_invoker},
        {"System.Runtime.InteropServices.GCHandle::GetTarget(System.Int32)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::get_target,
         get_target_invoker},
        {"System.Runtime.InteropServices.GCHandle::GetTargetHandle(System.Object,System.Int32,System.Runtime.InteropServices.GCHandleType)",
         (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::get_target_handle, get_target_handle_invoker},
        {"System.Runtime.InteropServices.GCHandle::FreeHandle(System.Int32)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::free_handle,
         free_handle_invoker},
        {"System.Runtime.InteropServices.GCHandle::GetAddrOfPinnedObject(System.Int32)",
         (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::get_addr_of_pinned_object, get_addr_of_pinned_object_invoker},
        {"System.Runtime.InteropServices.GCHandle::_InternalAlloc", (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::internal_alloc,
         internal_alloc_invoker},
        {"System.Runtime.InteropServices.GCHandle::_InternalAlloc(System.Object,System.Runtime.InteropServices.GCHandleType)",
         (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::internal_alloc, internal_alloc_invoker},
        {"System.Runtime.InteropServices.GCHandle::_InternalFree", (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::internal_free,
         internal_free_invoker},
        {"System.Runtime.InteropServices.GCHandle::_InternalFree(System.IntPtr)",
         (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::internal_free, internal_free_invoker},
        {"System.Runtime.InteropServices.GCHandle::InternalSet", (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::internal_set,
         internal_set_invoker},
        {"System.Runtime.InteropServices.GCHandle::InternalSet(System.IntPtr,System.Object)",
         (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::internal_set, internal_set_invoker},
        {"System.Runtime.InteropServices.GCHandle::InternalCompareExchange",
         (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::internal_compare_exchange, internal_compare_exchange_invoker},
        {"System.Runtime.InteropServices.GCHandle::InternalCompareExchange(System.IntPtr,System.Object,System.Object)",
         (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::internal_compare_exchange, internal_compare_exchange_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

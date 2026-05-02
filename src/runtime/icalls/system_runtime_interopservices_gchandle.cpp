#include "system_runtime_interopservices_gchandle.h"
#include "vm/gchandle.h"

namespace leanclr
{
namespace icalls
{

RtResult<bool> SystemRuntimeInteropServicesGCHandle::check_current_domain(void* handle) noexcept
{
    (void)handle;
    // In WebAssembly, there is only a single AppDomain.
    RET_OK(true);
}

RtResult<vm::RtObject*> SystemRuntimeInteropServicesGCHandle::get_target(void* handle) noexcept
{
    RET_OK(vm::GCHandle::get_target(handle));
}

RtResult<void*> SystemRuntimeInteropServicesGCHandle::get_target_handle(vm::RtObject* obj, void* handle, int32_t handle_type) noexcept
{
    RET_OK(vm::GCHandle::get_target_handle(obj, handle, handle_type));
}

RtResultVoid SystemRuntimeInteropServicesGCHandle::free_handle(void* handle) noexcept
{
    vm::GCHandle::free_handle(handle);
    RET_VOID_OK();
}

RtResult<void*> SystemRuntimeInteropServicesGCHandle::get_addr_of_pinned_object(void* handle) noexcept
{
    RET_OK(vm::GCHandle::get_addr_of_pinned_object(handle));
}

/// @icall: System.Runtime.InteropServices.GCHandle::CheckCurrentDomain(System.IntPtr)
static RtResultVoid check_current_domain_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto handle = EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeInteropServicesGCHandle::check_current_domain(handle));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.GCHandle::GetTarget(System.IntPtr)
static RtResultVoid get_target_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto handle = EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, SystemRuntimeInteropServicesGCHandle::get_target(handle));
    EvalStackOp::set_return(ret, obj);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.GCHandle::GetTargetHandle(System.Object,System.IntPtr,System.Runtime.InteropServices.GCHandleType)
static RtResultVoid get_target_handle_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    auto handle = EvalStackOp::get_param<void*>(params, 1);
    auto handle_type = EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, new_handle, SystemRuntimeInteropServicesGCHandle::get_target_handle(obj, handle, handle_type));
    EvalStackOp::set_return(ret, new_handle);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.GCHandle::FreeHandle(System.IntPtr)
static RtResultVoid free_handle_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto handle = EvalStackOp::get_param<void*>(params, 0);
    RET_ERR_ON_FAIL(SystemRuntimeInteropServicesGCHandle::free_handle(handle));
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.GCHandle::GetAddrOfPinnedObject(System.IntPtr)
static RtResultVoid get_addr_of_pinned_object_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                      const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto handle = EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, addr, SystemRuntimeInteropServicesGCHandle::get_addr_of_pinned_object(handle));
    EvalStackOp::set_return(ret, addr);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemRuntimeInteropServicesGCHandle::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Runtime.InteropServices.GCHandle::CheckCurrentDomain(System.IntPtr)",
         (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::check_current_domain, check_current_domain_invoker},
        {"System.Runtime.InteropServices.GCHandle::GetTarget(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::get_target,
         get_target_invoker},
        {"System.Runtime.InteropServices.GCHandle::GetTargetHandle(System.Object,System.IntPtr,System.Runtime.InteropServices.GCHandleType)",
         (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::get_target_handle, get_target_handle_invoker},
        {"System.Runtime.InteropServices.GCHandle::FreeHandle(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::free_handle,
         free_handle_invoker},
        {"System.Runtime.InteropServices.GCHandle::GetAddrOfPinnedObject(System.IntPtr)",
         (vm::InternalCallFunction)&SystemRuntimeInteropServicesGCHandle::get_addr_of_pinned_object, get_addr_of_pinned_object_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

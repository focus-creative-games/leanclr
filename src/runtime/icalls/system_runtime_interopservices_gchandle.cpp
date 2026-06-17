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
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

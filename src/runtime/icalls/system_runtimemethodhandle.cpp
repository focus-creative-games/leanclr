#include "system_runtimemethodhandle.h"

#include "vm/shim.h"

namespace leanclr
{
namespace icalls
{

RtResult<intptr_t> SystemRuntimeMethodHandle::get_function_pointer(intptr_t method) noexcept
{
    if (method == 0)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    const metadata::RtMethodInfo* method_info = reinterpret_cast<const metadata::RtMethodInfo*>(method);
    RET_OK(reinterpret_cast<intptr_t>(method_info->method_ptr));
}

/// @icall: System.RuntimeMethodHandle::GetFunctionPointer(System.IntPtr)
static RtResultVoid get_function_pointer_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    intptr_t method = EvalStackOp::get_param<intptr_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, result, SystemRuntimeMethodHandle::get_function_pointer(method));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_runtimemethodhandle[] = {
    {"System.RuntimeMethodHandle::GetFunctionPointer(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_function_pointer,
     get_function_pointer_invoker},
};

utils::Span<vm::InternalCallEntry> SystemRuntimeMethodHandle::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_runtimemethodhandle,
                                              sizeof(s_internal_call_entries_system_runtimemethodhandle) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

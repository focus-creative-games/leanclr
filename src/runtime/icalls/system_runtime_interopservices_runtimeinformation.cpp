#include "system_runtime_interopservices_runtimeinformation.h"

#include "platform/architecture.h"

namespace leanclr
{
namespace icalls
{

RtResult<vm::RtString*> SystemRuntimeInteropServicesRuntimeInformation::get_runtime_architecture() noexcept
{
    RET_OK(os::Architecture::get_architecture_name());
}

RtResult<vm::RtString*> SystemRuntimeInteropServicesRuntimeInformation::get_os_name() noexcept
{
    RET_OK(os::Architecture::get_os_name());
}

/// @icall: System.Runtime.InteropServices.RuntimeInformation::GetRuntimeArchitecture
static RtResultVoid get_runtime_architecture_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemRuntimeInteropServicesRuntimeInformation::get_runtime_architecture());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.RuntimeInformation::GetOSName
static RtResultVoid get_os_name_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemRuntimeInteropServicesRuntimeInformation::get_os_name());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemRuntimeInteropServicesRuntimeInformation::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Runtime.InteropServices.RuntimeInformation::GetRuntimeArchitecture",
         (vm::InternalCallFunction)&SystemRuntimeInteropServicesRuntimeInformation::get_runtime_architecture, get_runtime_architecture_invoker},
        {"System.Runtime.InteropServices.RuntimeInformation::GetOSName", (vm::InternalCallFunction)&SystemRuntimeInteropServicesRuntimeInformation::get_os_name,
         get_os_name_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

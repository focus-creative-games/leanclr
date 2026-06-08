#include "system_threading_osspecificsynchronizationcontext.h"

namespace leanclr
{
namespace icalls
{

RtResult<vm::RtObject*> SystemThreadingOSSpecificSynchronizationContext::get_os_context() noexcept
{
    RET_OK(nullptr);
}

/// @icall: System.Threading.OSSpecificSynchronizationContext::GetOSContext()
static RtResultVoid get_os_context_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemThreadingOSSpecificSynchronizationContext::get_os_context());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemThreadingOSSpecificSynchronizationContext::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Threading.OSSpecificSynchronizationContext::GetOSContext()",
         (vm::InternalCallFunction)&SystemThreadingOSSpecificSynchronizationContext::get_os_context, get_os_context_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

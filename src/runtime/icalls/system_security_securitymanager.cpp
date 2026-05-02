#include "system_security_securitymanager.h"

namespace leanclr
{
namespace icalls
{

RtResult<bool> SystemSecuritySecurityManager::get_security_enabled() noexcept
{
    RET_OK(false);
}

RtResultVoid get_security_enabled_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                          const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemSecuritySecurityManager::get_security_enabled());
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemSecuritySecurityManager::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Security.SecurityManager::get_SecurityEnabled", (vm::InternalCallFunction)&SystemSecuritySecurityManager::get_security_enabled,
         get_security_enabled_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr
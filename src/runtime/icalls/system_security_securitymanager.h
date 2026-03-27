#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{
class SystemSecuritySecurityManager
{
  public:
    // @icall: System.Security.SecurityManager::get_SecurityEnabled()
    static RtResult<bool> get_security_enabled();

    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();
};
} // namespace icalls
} // namespace leanclr
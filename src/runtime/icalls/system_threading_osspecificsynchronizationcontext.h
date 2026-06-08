#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemThreadingOSSpecificSynchronizationContext
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<vm::RtObject*> get_os_context() noexcept;
};

} // namespace icalls
} // namespace leanclr

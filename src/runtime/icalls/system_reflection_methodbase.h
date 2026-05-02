#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionMethodBase
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get the currently executing method
    static RtResult<vm::RtReflectionMethod*> get_current_method() noexcept;
};

} // namespace icalls
} // namespace leanclr

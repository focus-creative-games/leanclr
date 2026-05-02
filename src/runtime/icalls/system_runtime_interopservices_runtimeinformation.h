#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeInteropServicesRuntimeInformation
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<vm::RtString*> get_runtime_architecture() noexcept;
    static RtResult<vm::RtString*> get_os_name() noexcept;
};

} // namespace icalls
} // namespace leanclr

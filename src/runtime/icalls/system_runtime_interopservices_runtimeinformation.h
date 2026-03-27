#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeInteropServicesRuntimeInformation
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    static RtResult<vm::RtString*> get_runtime_architecture();
    static RtResult<vm::RtString*> get_os_name();
};

} // namespace icalls
} // namespace leanclr

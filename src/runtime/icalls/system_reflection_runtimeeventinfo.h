#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionRuntimeEventInfo
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResultVoid get_event_info(vm::RtReflectionEventInfo* ref_event, vm::RtReflectionMonoEventInfo* ref_event_info) noexcept;
    static RtResult<int32_t> get_metadata_token(vm::RtReflectionEventInfo* event_info) noexcept;
};

} // namespace icalls
} // namespace leanclr

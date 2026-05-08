#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemThreadingNativeEventCalls
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<intptr_t> create_event_icall(bool manual, bool initial, const Utf16Char* name, int32_t name_length, int32_t* error_code) noexcept;
    static RtResult<bool> set_event_internal(intptr_t handle) noexcept;
    static RtResult<bool> reset_event_internal(intptr_t handle) noexcept;
    static RtResultVoid close_event_internal(intptr_t handle) noexcept;
};

} // namespace icalls
} // namespace leanclr

#pragma once

#include "vm/internal_calls.h"

namespace leanclr
{
namespace icalls
{

class SystemGlobalizationCalendarData
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<bool> fill_calendar_data(vm::RtCalendarData* _this, vm::RtString* locale_name, int32_t calendar_id) noexcept;
};

} // namespace icalls
} // namespace leanclr

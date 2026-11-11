#pragma once

#include "build_config.h"
#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemCurrentSystemTimeZone
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;
#if LEANCLR_PLATFORM_POSIX || LEANCLR_PLATFORM_PORTABLE
    static RtResult<bool> get_time_zone_data(int32_t year, vm::RtArray** data, vm::RtArray** names, bool* daylight) noexcept;
#endif
};

} // namespace icalls
} // namespace leanclr

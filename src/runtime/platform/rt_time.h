#pragma once

#include "core/rt_base.h"

namespace leanclr
{
namespace os
{
class Time
{
  public:
    static int32_t get_tick_count();
    static int64_t get_current_time_millis();
    static int64_t get_current_time_nanos();
    static int64_t get_ticks_100nanos();
    static int64_t get_system_time_as_file_time();
};
} // namespace os
} // namespace leanclr

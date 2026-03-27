#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemDateTime
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // Get system time as file time (Windows FILETIME format)
    static RtResult<int64_t> get_system_time_as_file_time();
};

} // namespace icalls
} // namespace leanclr

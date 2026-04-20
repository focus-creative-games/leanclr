#pragma once

#include "rt_base.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace platform
{
class Kernel32
{
  public:
    static bool set_thread_error_mode(uint32_t mode, uint32_t& old_mode);
    static bool get_file_attributes_ex_private(vm::RtString* name, uint32_t file_info_level, void* file_info);
};
} // namespace platform
} // namespace leanclr
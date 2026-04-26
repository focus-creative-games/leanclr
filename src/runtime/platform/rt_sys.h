#pragma once

#include "core/rt_base.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace platform
{
class RtSys
{
  public:
    static int32_t ch_mod(vm::RtString* path, int32_t mode);
    static int32_t mk_dir(vm::RtString* path, int32_t mode);
    static int32_t rename(vm::RtString* old_path, vm::RtString* new_path);
    static int32_t rm_dir(vm::RtString* path);
    static int32_t unlink(vm::RtString* path);
    static intptr_t open_dir(vm::RtString* path);
    static int32_t close_dir(intptr_t dir);
    static int32_t get_read_dir_r_buffer_size();
    static int32_t read_dir_r(intptr_t dir, uint8_t* buffer, int32_t buffer_size, void* output_entry);
    static int32_t read_link(vm::RtString* path, vm::RtArray* buffer, int32_t buffer_size);
};
} // namespace platform
} // namespace leanclr

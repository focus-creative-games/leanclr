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
    static int32_t double_to_string(double value, const char* format, char* buffer, int32_t buffer_size);
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
    static int32_t link(vm::RtString* source, vm::RtString* target);
    static int32_t symlink(vm::RtString* target, vm::RtString* link_path);
    static uint32_t get_e_uid();
    static uint32_t get_e_gid();
    static int32_t f_stat(vm::RtObject* fd, void* output);
    static int32_t stat_string(vm::RtString* path, void* output);
    static int32_t stat_byte(uint8_t* path, void* output);
    static int32_t lstat_string(vm::RtString* path, void* output);
    static int32_t lstat_byte(uint8_t* path, void* output);
};
} // namespace platform
} // namespace leanclr

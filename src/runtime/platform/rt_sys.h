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
    static int32_t convert_error_pal_to_platform(int32_t error);
    static int32_t convert_error_platform_to_pal(int32_t error);
    static uint8_t* str_error_r(int32_t error, uint8_t* buffer, int32_t buffer_size);
    static void get_non_cryptographically_secure_random_bytes(uint8_t* buffer, int32_t length);
    static int32_t copy_file(vm::RtObject* source, vm::RtObject* destination);
    static int32_t lchflags(vm::RtString* path, uint32_t flags);
    static int32_t lchflags_can_set_hidden_flag();
    static int32_t utime(vm::RtString* path, void* time_buffer);
    static int32_t utimes(vm::RtString* path, void* time_value_pair);
    static int32_t globalization_get_time_zone_display_name(vm::RtString* locale_name, vm::RtString* time_zone_id, int32_t type, vm::RtObject* result,
                                                            int32_t result_length);
    static int32_t f_stat(vm::RtObject* fd, void* output);
    static int32_t stat_string(vm::RtString* path, void* output);
    static int32_t stat_byte(uint8_t* path, void* output);
    static int32_t lstat_string(vm::RtString* path, void* output);
    static int32_t lstat_byte(uint8_t* path, void* output);
};
} // namespace platform
} // namespace leanclr

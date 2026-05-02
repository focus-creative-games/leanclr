#pragma once

#include "build_config.h"
#include "icall_base.h"

namespace leanclr
{
namespace icalls
{
class Interop
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

#if LEANCLR_PLATFORM_POSIX
    static RtResult<int32_t> double_to_string(double value, const char* format, char* buffer, int32_t buffer_size) noexcept;
    static RtResult<int32_t> sys_lchflags_can_set_hidden_flag() noexcept;
    static RtResult<int32_t> globalization_get_time_zone_display_name(vm::RtString* locale_name, vm::RtString* time_zone_id, int32_t type, vm::RtObject* result,
                                                                      int32_t result_length) noexcept;
    static RtResult<int32_t> sys_ch_mod(vm::RtString* path, int32_t mode) noexcept;
    static RtResult<int32_t> sys_close_dir(intptr_t dir) noexcept;
    static RtResult<int32_t> sys_convert_error_pal_to_platform(int32_t error) noexcept;
    static RtResult<int32_t> sys_convert_error_platform_to_pal(int32_t error) noexcept;
    static RtResult<int32_t> sys_copy_file(vm::RtObject* source, vm::RtObject* destination) noexcept;
    static RtResult<int32_t> sys_f_stat(vm::RtObject* fd, void* output) noexcept;
    static RtResult<uint32_t> sys_get_e_gid() noexcept;
    static RtResult<uint32_t> sys_get_e_uid() noexcept;
    static RtResultVoid sys_get_non_cryptographically_secure_random_bytes(uint8_t* buffer, int32_t length) noexcept;
    static RtResult<int32_t> sys_get_read_dir_r_buffer_size() noexcept;
    static RtResult<int32_t> sys_lchflags(vm::RtString* path, uint32_t flags) noexcept;
    static RtResult<int32_t> sys_link(vm::RtString* source, vm::RtString* target) noexcept;
    static RtResult<int32_t> sys_lstat_byte(uint8_t* path, void* output) noexcept;
    static RtResult<int32_t> sys_lstat_string(vm::RtString* path, void* output) noexcept;
    static RtResult<int32_t> sys_mkdir(vm::RtString* path, int32_t mode) noexcept;
    static RtResult<intptr_t> sys_open_dir(vm::RtString* path) noexcept;
    static RtResult<int32_t> sys_read_dir_r(intptr_t dir, uint8_t* buffer, int32_t buffer_size, void* output_entry) noexcept;
    static RtResult<int32_t> sys_read_link(vm::RtString* path, vm::RtArray* buffer, int32_t buffer_size) noexcept;
    static RtResult<int32_t> sys_rename(vm::RtString* old_path, vm::RtString* new_path) noexcept;
    static RtResult<int32_t> sys_rmdir(vm::RtString* path) noexcept;
    static RtResult<int32_t> sys_stat_byte(uint8_t* path, void* output) noexcept;
    static RtResult<int32_t> sys_stat_string(vm::RtString* path, void* output) noexcept;
    static RtResult<uint8_t*> sys_str_error_r(int32_t error, uint8_t* buffer, int32_t buffer_size) noexcept;
    static RtResult<int32_t> sys_symlink(vm::RtString* target, vm::RtString* link_path) noexcept;
    static RtResult<int32_t> sys_unlink(vm::RtString* path) noexcept;
    static RtResult<int32_t> sys_utime(vm::RtString* path, void* time_buffer) noexcept;
    static RtResult<int32_t> sys_utimes(vm::RtString* path, void* time_value_pair) noexcept;
#endif

    static RtResult<uint32_t> bcrypt_gen_random(intptr_t algo_handle, uint8_t* buffer, int32_t length, int32_t flags) noexcept;
    static RtResult<bool> kernel32_set_thread_error_mode(uint32_t mode, uint32_t& old_mode) noexcept;
    static RtResult<bool> kernel32_get_file_attributes_ex_private(vm::RtString* name, uint32_t file_info_level, void* file_info) noexcept;
    static RtResult<vm::RtObject*> kernel32_find_first_file_ex_private(vm::RtString* lp_file_name, uint32_t f_info_level_id, void* lp_find_file_data,
                                                                       uint32_t f_search_op, intptr_t lp_search_filter, int32_t dw_additional_flags) noexcept;

    // System.Console/WindowsConsole
    static RtResult<int32_t> windows_console_get_console_cp() noexcept;
    static RtResult<int32_t> windows_console_get_console_output_cp() noexcept;
};
} // namespace icalls
} // namespace leanclr

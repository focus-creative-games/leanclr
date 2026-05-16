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

#if LEANCLR_PLATFORM_WIN
    static RtResult<uint32_t> bcrypt_gen_random(intptr_t algo_handle, uint8_t* buffer, int32_t length, int32_t flags) noexcept;

    static RtResult<bool> kernel32_set_thread_error_mode(uint32_t mode, uint32_t& old_mode) noexcept;
    static RtResult<bool> kernel32_get_file_attributes_ex_private(vm::RtString* name, uint32_t file_info_level, void* file_info) noexcept;
    static RtResult<vm::RtObject*> kernel32_find_first_file_ex_private(vm::RtString* lp_file_name, uint32_t f_info_level_id, void* lp_find_file_data,
                                                                       uint32_t f_search_op, intptr_t lp_search_filter, int32_t dw_additional_flags) noexcept;
    static RtResult<uint32_t> kernel32_get_time_zone_information(void* lp_time_zone_information) noexcept;
    static RtResult<uint32_t> kernel32_get_dynamic_time_zone_information(void* lp_dynamic_tz) noexcept;

    static RtResult<bool> kernel32_delete_volume_mount_point_private(vm::RtString* mount_point) noexcept;
    static RtResult<bool> kernel32_free_library(intptr_t h_module) noexcept;
    static RtResult<vm::RtObject*> kernel32_load_library_ex(vm::RtString* lib_filename, intptr_t reserved, int32_t flags) noexcept;
    // static RtResult<bool> kernel32_get_file_mui_path(uint32_t flags, vm::RtString* file_path, vm::RtObject* language, int32_t* language_length_chars,
    //                                                  vm::RtObject* file_mui_path, int32_t* file_mui_path_length_chars, int64_t* enumerator) noexcept;
    static RtResult<bool> kernel32_close_handle(intptr_t handle) noexcept;
    static RtResult<int32_t> kernel32_copy_file2(vm::RtString* existing, vm::RtString* new_file, void* extended_parameters) noexcept;
    static RtResult<bool> kernel32_copy_file_ex_private(vm::RtString* src, vm::RtString* dst, intptr_t progress_routine, intptr_t progress_data,
                                                       int32_t* cancel, int32_t flags) noexcept;
    static RtResult<bool> kernel32_create_directory_private(vm::RtString* path, void* security_attributes) noexcept;
    static RtResult<intptr_t> kernel32_create_file_private(vm::RtString* name, int32_t desired_access, int32_t share_mode, void* security_attributes,
                                                             int32_t creation_disposition, int32_t flags_and_attributes, intptr_t template_file) noexcept;
    static RtResult<bool> kernel32_delete_file_private(vm::RtString* path) noexcept;
    static RtResult<bool> kernel32_find_next_file(vm::RtObject* find_handle, void* find_file_data) noexcept;
    // static RtResult<int32_t> kernel32_format_message(int32_t flags, intptr_t source, uint32_t message_id, int32_t language_id, Utf16Char* buffer,
    //                                                  int32_t buffer_chars, vm::RtArray* arguments) noexcept;
    static RtResult<bool> kernel32_get_file_information_by_handle_ex(intptr_t h_file, int32_t file_information_class, void* file_information,
                                                                     uint32_t buffer_size) noexcept;
    static RtResult<int32_t> kernel32_get_logical_drives() noexcept;
    static RtResult<bool> kernel32_move_file_ex_private(vm::RtString* src, vm::RtString* dst, uint32_t flags) noexcept;
    static RtResult<bool> kernel32_remove_directory_private(vm::RtString* path) noexcept;
    static RtResult<bool> kernel32_replace_file_private(vm::RtString* replaced_file_name, vm::RtString* replacement_file_name,
                                                        vm::RtString* backup_file_name, int32_t replace_flags, intptr_t exclude, intptr_t reserved) noexcept;
    static RtResult<bool> kernel32_set_file_attributes_private(vm::RtString* name, int32_t attributes) noexcept;
    static RtResult<bool> kernel32_set_file_information_by_handle(vm::RtObject* h_file, int32_t file_information_class, void* file_information,
                                                                  uint32_t buffer_size) noexcept;
#endif
};
} // namespace icalls
} // namespace leanclr

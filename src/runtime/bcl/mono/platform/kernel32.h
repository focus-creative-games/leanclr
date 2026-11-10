#pragma once

#include "core/rt_base.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace platform
{
class Kernel32
{
  public:
    /// Wraps GetConsoleCP / GetConsoleOutputCP.
    static int32_t get_console_cp();
    static int32_t get_console_output_cp();
    static intptr_t get_std_handle(int32_t std_handle);
    static bool get_console_screen_buffer_info(intptr_t handle, void* info);
    static bool peek_console_input(intptr_t handle, void* record, int32_t length, int32_t* events_read);
    static bool read_console_input(intptr_t handle, void* record, int32_t length, int32_t* events_read);
    static bool get_console_mode(intptr_t handle, int32_t* mode);
    static bool set_console_mode(intptr_t handle, int32_t mode);

#if LEANCLR_PLATFORM_WIN
    static bool set_thread_error_mode(uint32_t mode, uint32_t& old_mode);
    static bool get_file_attributes_ex_private(vm::RtString* name, uint32_t file_info_level, void* file_info);

    /// Wraps FindFirstFileExW. Returns a Win32 HANDLE as intptr_t (INVALID_HANDLE_VALUE on failure).
    static intptr_t find_first_file_ex_private(vm::RtString* lp_file_name, uint32_t f_info_level_id, void* lp_find_file_data, uint32_t f_search_op,
                                               intptr_t lp_search_filter, int32_t dw_additional_flags);

    /// Wraps GetTimeZoneInformation. @p lp_time_zone_information points at the managed
    /// TIME_ZONE_INFORMATION layout (172 bytes, Unicode names).
    static uint32_t get_time_zone_information(void* lp_time_zone_information);
    static uint32_t get_dynamic_time_zone_information(void* lp_dynamic_tz);

    static bool delete_volume_mount_point_private(vm::RtString* mount_point);
    static bool free_library(intptr_t h_module);
    static intptr_t load_library_ex(vm::RtString* lib_filename, intptr_t reserved, int32_t flags);

    static bool close_handle(intptr_t handle);
    static int32_t copy_file2(vm::RtString* existing, vm::RtString* new_file, void* extended_parameters);
    static bool copy_file_ex_private(vm::RtString* src, vm::RtString* dst, intptr_t progress_routine, intptr_t progress_data, int32_t* cancel, int32_t flags);
    static bool create_directory_private(vm::RtString* path, void* security_attributes);
    static intptr_t create_file_private(vm::RtString* name, int32_t desired_access, int32_t share_mode, void* security_attributes, int32_t creation_disposition,
                                        int32_t flags_and_attributes, intptr_t template_file);
    static bool delete_file_private(vm::RtString* path);
    static bool find_next_file(intptr_t find_handle, void* find_file_data);
    // static int32_t format_message(int32_t flags, intptr_t source, uint32_t message_id, int32_t language_id, Utf16Char* buffer, int32_t buffer_chars,
    //                               intptr_t* arguments, int32_t argument_count);
    static bool get_file_information_by_handle_ex(intptr_t h_file, int32_t file_information_class, void* file_information, uint32_t buffer_size);
    static int32_t get_logical_drives();
    static bool move_file_ex_private(vm::RtString* src, vm::RtString* dst, uint32_t flags);
    static bool remove_directory_private(vm::RtString* path);
    static bool replace_file_private(vm::RtString* replaced_file_name, vm::RtString* replacement_file_name, vm::RtString* backup_file_name,
                                     int32_t replace_flags, intptr_t exclude, intptr_t reserved);
    static bool set_file_attributes_private(vm::RtString* name, int32_t attributes);
    static bool set_file_information_by_handle(intptr_t h_file, int32_t file_information_class, void* file_information, uint32_t buffer_size);
#endif
};
} // namespace platform
} // namespace leanclr
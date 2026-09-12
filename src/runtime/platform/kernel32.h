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
    static uint64_t get_tick_count64();
    static bool get_system_times(int64_t* idle_time, int64_t* kernel_time, int64_t* user_time);
    static intptr_t get_current_thread();
    static int32_t get_current_thread_id();
    static bool get_thread_io_pending_flag(intptr_t thread_handle, int32_t* is_io_pending);
    static intptr_t get_std_handle(int32_t std_handle);
    static bool get_console_screen_buffer_info(intptr_t handle, void* info);
    static bool peek_console_input(intptr_t handle, void* record, int32_t length, int32_t* events_read);
    static bool read_console_input(intptr_t handle, void* record, int32_t length, int32_t* events_read);
    static bool get_console_mode(intptr_t handle, int32_t* mode);
    static bool set_console_mode(intptr_t handle, int32_t mode);
    static int32_t format_message(int32_t flags, intptr_t source, uint32_t message_id, int32_t language_id, void* buffer, int32_t buffer_chars,
                                  intptr_t arguments);
    static uint32_t get_full_path_name(const Utf16Char* path, uint32_t buffer_length, Utf16Char* buffer, intptr_t file_part);
    static intptr_t create_event_ex(intptr_t security_attributes, const Utf16Char* name, uint32_t flags, uint32_t desired_access);
    static intptr_t open_event(uint32_t desired_access, int32_t inherit_handle, const Utf16Char* name);
    static bool set_event(intptr_t handle);
    static bool reset_event(intptr_t handle);
    static int32_t wait_for_single_object_ex(intptr_t handle, int32_t milliseconds, int32_t alertable);
    static int32_t wait_for_multiple_objects_ex(intptr_t* handles, int32_t count, int32_t wait_all, int32_t milliseconds);
    static bool close_handle(intptr_t handle);
    static intptr_t create_file_private(vm::RtString* name, int32_t desired_access, int32_t share_mode, void* security_attributes, int32_t creation_disposition,
                                        int32_t flags_and_attributes, intptr_t template_file);
    static intptr_t create_file_private(const Utf16Char* name, int32_t desired_access, int32_t share_mode, void* security_attributes,
                                        int32_t creation_disposition, int32_t flags_and_attributes, intptr_t template_file);
    static bool delete_file_private(vm::RtString* path);
    static bool delete_file_private(const Utf16Char* path);
    static bool get_file_attributes_ex_private(vm::RtString* name, uint32_t file_info_level, void* file_info);
    static bool get_file_attributes_ex_private(const Utf16Char* name, uint32_t file_info_level, void* file_info);
    static bool get_file_information_by_handle(intptr_t h_file, void* file_information);
    static bool get_file_information_by_handle_ex(intptr_t h_file, int32_t file_information_class, void* file_information, uint32_t buffer_size);
    static uint32_t get_temp_path(uint32_t buffer_length, Utf16Char* buffer);
    static int32_t copy_file2(vm::RtString* existing, vm::RtString* new_file, void* extended_parameters);
    static bool copy_file_ex_private(vm::RtString* src, vm::RtString* dst, intptr_t progress_routine, intptr_t progress_data, int32_t* cancel, int32_t flags);
    static bool create_directory_private(vm::RtString* path, void* security_attributes);
    static int32_t get_logical_drives();
    static bool move_file_ex_private(vm::RtString* src, vm::RtString* dst, uint32_t flags);
    static bool remove_directory_private(vm::RtString* path);
    static bool replace_file_private(vm::RtString* replaced_file_name, vm::RtString* replacement_file_name, vm::RtString* backup_file_name,
                                     int32_t replace_flags, intptr_t exclude, intptr_t reserved);
    static bool set_file_attributes_private(vm::RtString* name, int32_t attributes);
    static bool set_file_information_by_handle(intptr_t h_file, int32_t file_information_class, void* file_information, uint32_t buffer_size);

#if LEANCLR_PLATFORM_WIN
    static bool set_thread_error_mode(uint32_t mode, uint32_t& old_mode);

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
    static intptr_t load_library_ex(const Utf16Char* lib_filename, intptr_t reserved, int32_t flags);
    static intptr_t get_proc_address(intptr_t h_module, const char* proc_name);

    static bool query_performance_frequency(int64_t* frequency);
    static bool query_performance_counter(int64_t* counter);
    static void initialize_critical_section(void* critical_section);
    static void delete_critical_section(void* critical_section);
    static void enter_critical_section(void* critical_section);
    static void leave_critical_section(void* critical_section);
    static void initialize_condition_variable(void* condition_variable);
    static bool sleep_condition_variable_cs(void* condition_variable, void* critical_section, int32_t milliseconds);
    static void wake_condition_variable(void* condition_variable);
    static intptr_t create_io_completion_port(intptr_t file_handle, intptr_t existing_completion_port, uintptr_t completion_key,
                                              int32_t number_of_concurrent_threads);
    static bool post_queued_completion_status(intptr_t completion_port, uint32_t number_of_bytes_transferred, uintptr_t completion_key,
                                              intptr_t overlapped);
    static bool get_queued_completion_status(intptr_t completion_port, uint32_t* number_of_bytes_transferred, uintptr_t* completion_key,
                                             intptr_t* overlapped, int32_t milliseconds);
    static bool get_queued_completion_status_ex(intptr_t completion_port, void* completion_port_entries, int32_t count,
                                                int32_t* number_of_entries_removed, int32_t milliseconds, int32_t alertable);
    static bool find_next_file(intptr_t find_handle, void* find_file_data);
#endif
};
} // namespace platform
} // namespace leanclr

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
    static bool set_thread_error_mode(uint32_t mode, uint32_t& old_mode);
    static bool get_file_attributes_ex_private(vm::RtString* name, uint32_t file_info_level, void* file_info);

    /// Wraps FindFirstFileExW. Returns a Win32 HANDLE as intptr_t (INVALID_HANDLE_VALUE on failure).
    static intptr_t find_first_file_ex_private(vm::RtString* lp_file_name, uint32_t f_info_level_id, void* lp_find_file_data, uint32_t f_search_op,
                                               intptr_t lp_search_filter, int32_t dw_additional_flags);

    /// Wraps GetConsoleCP / GetConsoleOutputCP.
    static int32_t get_console_cp();
    static int32_t get_console_output_cp();

    /// GetLastError / non-Win fallback (see marshal.cpp).
    static int32_t get_last_win32_error();
    static void set_last_win32_error(int32_t error);
};
} // namespace platform
} // namespace leanclr
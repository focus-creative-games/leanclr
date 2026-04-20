#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemIOMonoIO
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // System.IO.MonoIO icalls - property getters
    static RtResult<Utf16Char> get_alt_directory_separator_char();
    static RtResult<intptr_t> get_console_error();
    static RtResult<intptr_t> get_console_input();
    static RtResult<intptr_t> get_console_output();
    static RtResult<Utf16Char> get_directory_separator_char();
    static RtResult<Utf16Char> get_path_separator();
    static RtResult<Utf16Char> get_volume_separator_char();

    // System.IO.MonoIO icalls - file operations
    static RtResult<bool> close(intptr_t handle, int32_t* error);
    static RtResultVoid dump_handles();
    static RtResult<bool> find_close_file(intptr_t handle);
    static RtResult<vm::RtString*> get_current_directory(int32_t* error);
    static RtResult<int32_t> get_file_type(intptr_t handle, int32_t* error);
    static RtResult<int64_t> get_length(intptr_t handle, int32_t* error);
    static RtResult<intptr_t> open(const Utf16Char* filename, int32_t mode, int32_t access, int32_t share, int32_t options, int32_t* error);
    static RtResult<int32_t> read(intptr_t handle, vm::RtArray* dest, int32_t dest_offset, int32_t count, int32_t* error);
    static RtResult<bool> remap_path(vm::RtString* path, vm::RtString** new_path);
    static RtResult<int64_t> seek(intptr_t handle, int64_t offset, int32_t origin, int32_t* error);
    static RtResult<int32_t> write(intptr_t handle, vm::RtArray* src, int32_t src_offset, int32_t count, int32_t* error);
};

} // namespace icalls
} // namespace leanclr

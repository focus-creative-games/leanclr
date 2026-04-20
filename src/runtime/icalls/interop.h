#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{
class Interop
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    static RtResult<int32_t> double_to_string(double value, const char* format, char* buffer, int32_t buffer_size);
    static RtResultVoid sys_lchflags_can_set_hidden_flag();
    static RtResult<uint32_t> bcrypt_gen_random(intptr_t algo_handle, uint8_t* buffer, int32_t length, int32_t flags);
    static RtResult<bool> kernel32_set_thread_error_mode(uint32_t mode, uint32_t& old_mode);
    static RtResult<bool> kernel32_get_file_attributes_ex_private(vm::RtString* name, uint32_t file_info_level, void* file_info);
};
} // namespace icalls
} // namespace leanclr

#pragma once

#include "vm/object.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace vmutils
{
class StringBuilder
{
  public:
    static void get_chunk_chars(vm::RtObject* sb, Utf16Char** out_data, int32_t* out_capacity_chars) noexcept;
    static void set_chunk_length(vm::RtObject* sb, int32_t len) noexcept;
    static void sync_len_from_buffer(vm::RtObject* sb, int32_t cap_chars) noexcept;
};
} // namespace vmutils
} // namespace leanclr
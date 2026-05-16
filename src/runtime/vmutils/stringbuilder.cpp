#include "stringbuilder.h"
#include "vm/class.h"
#include "vm/rt_array.h"

namespace leanclr
{
namespace vmutils
{
  static const metadata::RtFieldInfo* s_chunkCharsField = nullptr;
  static const metadata::RtFieldInfo* s_chunkLengthField = nullptr;

  void StringBuilder::get_chunk_chars(vm::RtObject* sb, Utf16Char** out_data, int32_t* out_capacity_chars) noexcept
  {
    assert(sb != nullptr && out_data != nullptr && out_capacity_chars != nullptr);
    if (s_chunkCharsField == nullptr)
    {
      s_chunkCharsField = vm::Class::get_field_for_name(static_cast<const metadata::RtClass*>(sb->klass), "m_ChunkChars", false);
      assert(s_chunkCharsField != nullptr);
    }
    vm::RtArray* chunk = *(vm::RtArray**)(reinterpret_cast<uint8_t*>(sb + 1) + s_chunkCharsField->offset);
    *out_capacity_chars = vm::Array::get_array_length(chunk);
    *out_data = vm::Array::get_array_data_start_as<Utf16Char>(chunk);
  }
  
  void StringBuilder::set_chunk_length(vm::RtObject* sb, int32_t len) noexcept
  {
    assert (sb != nullptr);
    if (s_chunkLengthField == nullptr)
    {
      s_chunkLengthField = vm::Class::get_field_for_name(static_cast<const metadata::RtClass*>(sb->klass), "m_ChunkLength", false);
      assert(s_chunkLengthField != nullptr);
    }
    *(int32_t*)(reinterpret_cast<uint8_t*>(sb + 1) + s_chunkLengthField->offset) = len;
  }
  
  void StringBuilder::sync_len_from_buffer(vm::RtObject* sb, int32_t cap_chars) noexcept
  {
    assert (sb != nullptr && cap_chars > 0);
    Utf16Char* data = nullptr;
    int32_t cap = 0;
    get_chunk_chars(sb, &data, &cap);
    int32_t n = 0;
    while (n < cap && data[n] != 0)
      ++n;
    set_chunk_length(sb, n);
  }
}
} // namespace leanclr
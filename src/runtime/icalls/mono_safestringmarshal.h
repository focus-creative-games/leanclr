#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class MonoSafeStringMarshal
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // Convert managed string to UTF-8 C-string (caller must free with gfree)
    static RtResult<const char*> string_to_utf8_bytes(vm::RtString* s);

    // Free a string allocated by string_to_utf8_bytes
    static RtResultVoid gfree(void* ptr);
};

} // namespace icalls
} // namespace leanclr

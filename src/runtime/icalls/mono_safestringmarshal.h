#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class MonoSafeStringMarshal
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Convert managed string to UTF-8 C-string (caller must free with gfree)
    static RtResult<intptr_t> string_to_utf8_bytes(vm::RtString** s) noexcept;

    // Free a string allocated by string_to_utf8_bytes
    static RtResultVoid gfree(intptr_t ptr) noexcept;
};

} // namespace icalls
} // namespace leanclr

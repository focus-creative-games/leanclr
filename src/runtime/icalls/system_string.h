#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{
class SystemString
{
  public:
    static utils::Span<vm::NewobjInternalCallEntry> get_newobj_internal_call_entries();
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<vm::RtString*> newobj_char_array(vm::RtArray* charArray) noexcept;
    static RtResult<vm::RtString*> newobj_char_array_range(vm::RtArray* charArray, int32_t startIndex, int32_t length) noexcept;
    static RtResult<vm::RtString*> newobj_utf16chars(const Utf16Char* chars) noexcept;
    static RtResult<vm::RtString*> newobj_utf16chars_range(const Utf16Char* chars, int32_t startIndex, int32_t length) noexcept;
    static RtResult<vm::RtString*> newobj_utf8chars(const int8_t* chars) noexcept;
    static RtResult<vm::RtString*> newobj_utf8chars_range(const int8_t* chars, int32_t startIndex, int32_t length) noexcept;
    static RtResult<vm::RtString*> newobj_char_count(Utf16Char c, int32_t charCount) noexcept;
    static RtResult<vm::RtString*> newobj_readonlyspan(const vm::RtReadOnlySpan<Utf16Char> span) noexcept;

    static RtResult<vm::RtString*> fast_allocate_string(int32_t length) noexcept;
    static RtResult<vm::RtString*> internal_intern(vm::RtString* s) noexcept;
    static RtResult<vm::RtString*> internal_is_interned(vm::RtString* s) noexcept;
};
} // namespace icalls
} // namespace leanclr

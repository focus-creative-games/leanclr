#pragma once

#include <cstdint>

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemString
{
  public:
    // Returns the UTF-16 code unit at the specified index of the string.
    static RtResult<uint16_t> get_chars(vm::RtString* s, int32_t index) noexcept;

    // Returns the length of the string (number of UTF-16 code units).
    static RtResult<int32_t> get_length(vm::RtString* s) noexcept;
    static RtResult<int32_t> get_hash_code(vm::RtString* str) noexcept;

    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
};
} // namespace intrinsics
} // namespace leanclr

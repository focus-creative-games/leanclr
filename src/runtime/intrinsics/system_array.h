#pragma once

#include <cstdint>

#include "vm/intrinsics.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace intrinsics
{
class SystemArray
{
  public:
    // Returns the length of the array (number of elements).
    static RtResult<int32_t> get_length(vm::RtArray* arr) noexcept;

    // Returns the length of the array as a 64-bit integer.
    static RtResult<int64_t> get_long_length(vm::RtArray* arr) noexcept;

    // Gets the value at the specified index and copies it to the destination pointer.
    static RtResultVoid get_generic_value_impl(vm::RtArray* arr, int32_t index, void* value) noexcept;

    // Sets the value at the specified index from the source pointer.
    static RtResultVoid set_generic_value_impl(vm::RtArray* arr, int32_t index, void* value) noexcept;

    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
};
} // namespace intrinsics
} // namespace leanclr

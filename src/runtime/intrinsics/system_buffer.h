#pragma once

#include <cstdint>

#include "vm/intrinsics.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace intrinsics
{
class SystemBuffer
{
  public:
    static RtResult<int32_t> byte_length(vm::RtArray* array) noexcept;
    static RtResultVoid block_copy(vm::RtArray* source, int32_t source_offset, vm::RtArray* destination, int32_t destination_offset,
                                   int32_t count) noexcept;
    static RtResultVoid memory_copy(const void* source, void* destination, uint64_t destination_size_in_bytes,
                                    uint64_t source_bytes_to_copy) noexcept;

    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
};
} // namespace intrinsics
} // namespace leanclr

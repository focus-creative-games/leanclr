#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{

class SystemRuntimeInteropServicesMemoryMarshal
{
  public:
    static RtResult<void*> get_array_data_reference(vm::RtArray* array) noexcept;
    static RtResult<vm::RtReadOnlySpan<uint8_t>> cast_span(const metadata::RtMethodInfo* method, vm::RtReadOnlySpan<uint8_t> span) noexcept;

    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
};

} // namespace intrinsics
} // namespace leanclr

#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemSignature
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<int32_t> get_parameter_offset_internal(void* sig, int32_t csig, int32_t parameter_index) noexcept;
};

} // namespace icalls
} // namespace leanclr

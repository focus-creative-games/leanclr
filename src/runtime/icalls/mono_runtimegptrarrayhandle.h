#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class MonoRuntimeGPtrArrayHandle
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Mono.RuntimeGPtrArrayHandle icalls
    static RtResultVoid gptr_array_free(void* arr) noexcept;
};

} // namespace icalls
} // namespace leanclr

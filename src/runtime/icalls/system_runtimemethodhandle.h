#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeMethodHandle
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<intptr_t> get_function_pointer(intptr_t method) noexcept;
};

} // namespace icalls
} // namespace leanclr

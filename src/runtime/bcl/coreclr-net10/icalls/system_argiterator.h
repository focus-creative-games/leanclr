#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemArgIterator
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Setup the ArgIterator
    static RtResultVoid setup(intptr_t sig, intptr_t first_arg) noexcept;

    // Get next argument
    static RtResultVoid int_get_next_arg(void* value) noexcept;

    // Get next argument with type
    static RtResultVoid int_get_next_arg_with_type(void* value, intptr_t rth_handle) noexcept;

    // Get next argument type
    static RtResult<intptr_t> int_get_next_arg_type() noexcept;
};

} // namespace icalls
} // namespace leanclr

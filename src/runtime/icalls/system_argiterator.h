#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemArgIterator
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // Setup the ArgIterator
    static RtResultVoid setup(intptr_t sig, intptr_t first_arg);

    // Get next argument
    static RtResultVoid int_get_next_arg(void* value);

    // Get next argument with type
    static RtResultVoid int_get_next_arg_with_type(void* value, intptr_t rth_handle);

    // Get next argument type
    static RtResult<intptr_t> int_get_next_arg_type();
};

} // namespace icalls
} // namespace leanclr

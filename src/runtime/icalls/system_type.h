#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{
class SystemType
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    static RtResult<vm::RtReflectionType*> internal_from_handle(intptr_t handle);
};
} // namespace icalls
} // namespace leanclr

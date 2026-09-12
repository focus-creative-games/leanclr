#pragma once

#include "vm/internal_calls.h"

namespace leanclr
{
namespace icalls
{
class SystemRuntimeDependentHandle
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<void*> internal_alloc(vm::RtObject* target, vm::RtObject* dependent) noexcept;
    static RtResult<vm::RtObject*> internal_get_dependent(void* handle) noexcept;
    static RtResult<vm::RtObject*> internal_get_target_and_dependent(void* handle, vm::RtObject** dependent) noexcept;
    static RtResultVoid internal_set_target_to_null(void* handle) noexcept;
    static RtResultVoid internal_set_dependent(void* handle, vm::RtObject* dependent) noexcept;
    static RtResult<bool> internal_free(void* handle) noexcept;
};
} // namespace icalls
} // namespace leanclr

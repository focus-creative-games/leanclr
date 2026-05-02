#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeInteropServicesGCHandle
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<bool> check_current_domain(void* handle) noexcept;
    static RtResult<vm::RtObject*> get_target(void* handle) noexcept;
    static RtResult<void*> get_target_handle(vm::RtObject* obj, void* handle, int32_t handle_type) noexcept;
    static RtResultVoid free_handle(void* handle) noexcept;
    static RtResult<void*> get_addr_of_pinned_object(void* handle) noexcept;
};

} // namespace icalls
} // namespace leanclr

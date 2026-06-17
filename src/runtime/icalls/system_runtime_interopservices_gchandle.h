#pragma once

#include "icall_base.h"
#include "vm/gchandle.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeInteropServicesGCHandle
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<bool> check_current_domain(vm::GCHandleId handle) noexcept;
    static RtResult<vm::RtObject*> get_target(vm::GCHandleId handle) noexcept;
    static RtResult<vm::GCHandleId> get_target_handle(vm::RtObject* obj, vm::GCHandleId handle, int32_t handle_type) noexcept;
    static RtResultVoid free_handle(vm::GCHandleId handle) noexcept;
    static RtResult<void*> get_addr_of_pinned_object(vm::GCHandleId handle) noexcept;
};

} // namespace icalls
} // namespace leanclr

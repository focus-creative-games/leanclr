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
    static RtResult<void*> internal_alloc(vm::RtObject* obj, int32_t handle_type) noexcept;
    static RtResult<bool> internal_free(void* handle) noexcept;
    static RtResultVoid internal_set(void* handle, vm::RtObject* value) noexcept;
    static RtResult<vm::RtObject*> internal_compare_exchange(void* handle, vm::RtObject* value, vm::RtObject* old_value) noexcept;
};

} // namespace icalls
} // namespace leanclr

#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeInteropServicesGCHandle
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    static RtResult<bool> check_current_domain(void* handle);
    static RtResult<vm::RtObject*> get_target(void* handle);
    static RtResult<void*> get_target_handle(vm::RtObject* obj, void* handle, int32_t handle_type);
    static RtResultVoid free_handle(void* handle);
    static RtResult<void*> get_addr_of_pinned_object(void* handle);
};

} // namespace icalls
} // namespace leanclr

#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace vm
{

class GCHandle
{
  public:
#if !LEANCLR_USE_VOID_PTR_GCHANDLE
    static uint32_t get_handle_id(void* handle);
    static void* get_handle_by_id(uint32_t id);
#endif
    static void* new_handle(RtObject* obj, bool pinned);
    static void* new_weakref_handle(RtObject* obj, bool track_resurrection);
    static void free_handle(void* handle);
    static RtObject* get_target(void* handle);
    static void* get_target_handle(RtObject* obj, void* handle, int32_t handle_type);
    static void* get_addr_of_pinned_object(void* handle);
    static bool is_type_pinned(const metadata::RtClass* klass);
    static void foreach_strong_handles(void (*callback)(void*, void*), void* userData);
};
} // namespace vm
} // namespace leanclr

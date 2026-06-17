#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace vm
{
#if LEANCLR_USE_VOID_PTR_GCHANDLE
typedef intptr_t GCHandleId;
#else
typedef int32_t GCHandleId;
#endif

class GCHandle
{
  public:
    static GCHandleId get_handle_id(void* handle);
    static void* get_handle_by_id(GCHandleId id);
    static void* new_handle(RtObject* obj, bool pinned);
    static void* new_weakref_handle(RtObject* obj, bool track_resurrection);
    static void free_handle(void* handle);
    static RtObject* get_target(void* handle);
    static void* get_target_handle(RtObject* obj, void* handle, int32_t handle_type);
    static void* get_addr_of_pinned_object(void* handle);
    static bool is_type_pinned(const metadata::RtClass* klass);
    static void foreach_strong_handles(void (*callback)(vm::RtObject*, void*), void* userData);
    static void sweep_weak_handles(bool (*is_object_marked)(vm::RtObject*, void* ctx), void* ctx);
};
} // namespace vm
} // namespace leanclr

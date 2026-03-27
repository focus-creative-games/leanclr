#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace vm
{

class GCHandle
{
  public:
    static void free_handle(void* handle);
    static RtObject* get_target(void* handle);
    static void* get_target_handle(RtObject* obj, void* handle, int32_t handle_type);
    static void* get_addr_of_pinned_object(void* handle);
    static bool is_type_pinned(metadata::RtClass* klass);
};
} // namespace vm
} // namespace leanclr

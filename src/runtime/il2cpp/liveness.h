#pragma once

#include "vm/rt_managed_types.h"
#include "il2cpp_api_types.h"

namespace leanclr
{
namespace il2cpp
{

class Liveness
{
  public:
    static void* allocate_struct(metadata::RtClass* filter, uint32_t max_object_count, il2cpp_register_object_callback callback, void* userdata,
                                 il2cpp_liveness_reallocate_callback reallocate);
    static void free_struct(void* state);
    static void calculation_from_root(Il2CppObject* root, void* state);
    static void calculation_from_statics(void* state);
    static void finalize(void* state);
};
} // namespace il2cpp
} // namespace leanclr

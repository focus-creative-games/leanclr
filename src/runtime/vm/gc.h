#pragma once

#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{

class GC
{
  public:
    static vm::RtObject* get_ephemeron_tombstone();
    static void register_ephemeron_array(vm::RtObject* arr);
    static int32_t get_collection_count(int32_t generation);
    static int32_t get_max_generation();
    static void internal_collect(int32_t generation);
    static void record_pressure(int64_t bytes);
    static int64_t get_allocated_bytes_for_current_thread();
    static int32_t get_generation(vm::RtObject* obj);
    static void wait_for_pending_finalizers();
    static void suppress_finalize(vm::RtObject* obj);
    static void reregister_for_finalize(vm::RtObject* obj);
    static int64_t get_total_memory(bool force_full_collection);
};

} // namespace vm
} // namespace leanclr

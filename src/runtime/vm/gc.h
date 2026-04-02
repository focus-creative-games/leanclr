#pragma once

#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{

enum class GCMode : int32_t
{
    DISABLED = 0,
    ENABLED = 1,
    MANUAL = 2
};

class GC
{
  public:
    static vm::RtObject* get_ephemeron_tombstone();
    static void register_ephemeron_array(vm::RtObject* arr);
    static int32_t get_collection_count(int32_t generation);
    static int32_t get_max_generation();
    static void collect(int32_t generation);
    static int32_t collect_a_little();
    static void internal_collect(int32_t generation);
    static void record_pressure(int64_t bytes);
    static int64_t get_allocated_bytes_for_current_thread();
    static int32_t get_generation(vm::RtObject* obj);
    static void wait_for_pending_finalizers();
    static void suppress_finalize(vm::RtObject* obj);
    static void reregister_for_finalize(vm::RtObject* obj);
    static int64_t get_total_memory(bool force_full_collection);

    static void start_incremental_collection();
    static void enable();
    static void disable();
    static bool is_disabled();
    static void set_mode(GCMode mode);
    static bool is_incremental();
    static int64_t get_max_time_slice_ns();
    static void set_max_time_slice_ns(int64_t maxTimeSlice);
    static int64_t get_used_size();
    static int64_t get_heap_size();
    static void foreach_heap(void (*func)(void* data, void* context), void* userData);
    static void start_gc_world();
    static void stop_gc_world();
    static void* alloc_fixed(size_t size);
    static void free_fixed(void* address);
    static void write_barrier(RtObject** obj_ref_location, RtObject* new_obj);
    static bool has_strict_wbarriers();
    static void set_external_allocation_tracker(void (*func)(void*, size_t, int));
    static void set_external_wbarrier_tracker(void (*func)(void**));
};

} // namespace vm
} // namespace leanclr

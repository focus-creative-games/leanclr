#pragma once

// GC algorithm contract (no virtual dispatch).
//
// Each heap implementation (MarkSweepHeap, ZeroGcHeap, ...) must provide static methods
// matching the signatures used by garbage_collector.h via detail::GcHeapImpl.

namespace leanclr
{
namespace gc
{

struct GcHeapConcept
{
    // static void initialize();
    // static void collect();
    // static bool maybe_collect();
    // static bool should_collect(bool force);
    // static void* allocate_fixed(size_t size);
    // static void free_fixed(void* address);
    // static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site);
    // static vm::RtObject* allocate_array(...);
    // static void write_barrier(vm::RtObject** loc, vm::RtObject* obj);
    // static int64_t get_used_size();
    // static int64_t get_heap_size();
    // static int32_t get_collection_count();
    // static bool is_object_marked(const vm::RtObject* obj);
    // static void set_pressure_config(const GcPressureConfig& config);
};

} // namespace gc
} // namespace leanclr

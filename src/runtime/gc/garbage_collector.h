#pragma once

#include "core/rt_base.h"
#include "gc/gc_alloc_site.h"
#include "gc/gc_pressure.h"

namespace leanclr
{
namespace metadata
{
struct RtClass;
}

namespace vm
{
struct RtObject;
}

namespace gc
{

class GarbageCollector
{
  public:
    static void initialize();

    static void collect();
    static bool maybe_collect();
    static bool should_collect(bool force = false);

    static void record_pressure(int64_t bytes);
    static uint64_t get_effective_pressure();
    static uint64_t get_bytes_allocated_since_last_gc();
    static void set_pressure_config(const GcPressureConfig& config);

    static void* allocate_fixed(size_t size);
    static void free_fixed(void* address);
    static vm::RtObject** allocate_fixed_reference_array(size_t length);

    static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site);
    static vm::RtObject* allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size, const GcAllocSite& site);
    static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes, const GcAllocSite& site);

    // Legacy entry points (no allocation site).
    static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size);
    static vm::RtObject* allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size);
    static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes);

    static void write_barrier(vm::RtObject** obj_ref_location, vm::RtObject* new_obj);

    static int64_t get_used_size();
    static int64_t get_heap_size();
    static int32_t get_collection_count();
    static bool is_object_marked(const vm::RtObject* obj);
};
} // namespace gc
} // namespace leanclr

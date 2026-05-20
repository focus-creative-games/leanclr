#pragma once

#if LEANCLR_GC_ZERO_GC

#include "gc/gc_alloc_site.h"
#include "gc/gc_pressure.h"
#include "metadata/rt_metadata.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace gc
{

// Bump-only allocator (active when LEANCLR_GC_ZERO_GC is defined).
class ZeroGcHeap
{
  public:
    static void initialize();
    static void collect();
    static bool maybe_collect();
    static bool should_collect(bool force);

    static void* allocate_fixed(size_t size);
    static void free_fixed(void* address);

    static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site);
    // static vm::RtObject* allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size, const GcAllocSite& site);
    static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes, const GcAllocSite& site);

    static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size);
    // static vm::RtObject* allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size);
    static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes);

    static void write_barrier(vm::RtObject** obj_ref_location, vm::RtObject* new_obj);
    static bool has_strict_wbarriers();

    static int64_t get_used_size();
    static int64_t get_heap_size();
    static int32_t get_collection_count();
    static bool is_object_marked(const vm::RtObject* obj);

    static void set_pressure_config(const GcPressureConfig& config);
};

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_ZERO_GC

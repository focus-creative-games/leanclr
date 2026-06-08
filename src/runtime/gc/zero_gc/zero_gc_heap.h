#pragma once

#include "gc/gc_alloc_site.h"
#include "gc/gc_common.h"
#include "gc/gc_pressure.h"
#include "metadata/rt_metadata.h"
#include "vm/rt_managed_types.h"

#if LEANCLR_GC_ZERO_GC

namespace leanclr
{
namespace gc
{

// Bump-only allocator (active when LEANCLR_GC_ZERO_GC is defined).
class ZeroGcHeap
{
  public:
    struct Config
    {
        GCMode mode;
        GcPressureConfig pressure_config;
    };

    static void initialize(const Config& config);
    static void collect();
    static bool maybe_collect();
    static bool should_collect(bool force);

    static int64_t get_used_size();
    static int64_t get_heap_size();
    static int32_t get_collection_count();

    static void set_gc_mode(GCMode mode)
    {
        (void)mode;
    }

    static void write_barrier(vm::RtObject** obj_ref_location, vm::RtObject* new_obj)
    {
        *obj_ref_location = new_obj;
    }

    static bool has_strict_wbarriers()
    {
        return false;
    }

    static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site);
    // static vm::RtObject* allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size, const GcAllocSite& site);
    static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes, const GcAllocSite& site);

    static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size);
    // static vm::RtObject* allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size);
    static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes);
};

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_ZERO_GC

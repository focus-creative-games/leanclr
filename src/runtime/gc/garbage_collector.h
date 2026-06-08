#pragma once

#include "core/rt_base.h"
#include "gc_alloc_site.h"
#include "gc_config.h"
#include "mark_sweep/mark_sweep_heap.h"
#include "zero_gc/zero_gc_heap.h"

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
#if LEANCLR_GC_MARK_SWEEP
    typedef MarkSweepHeap GcHeapImpl;
#elif LEANCLR_GC_ZERO_GC
    typedef ZeroGcHeap GcHeapImpl;
#else
#error "No GC implementation selected"
#endif

    static void initialize(const GcHeapImpl::Config& config)
    {
        GcHeapImpl::initialize(config);
    }

    static void collect()
    {
        GcHeapImpl::collect();
    }

    static bool maybe_collect()
    {
        return GcHeapImpl::maybe_collect();
    }

    static bool should_collect(bool force)
    {
        return GcHeapImpl::should_collect(force);
    }

    static int64_t get_used_size()
    {
        return GcHeapImpl::get_used_size();
    }

    static int64_t get_heap_size()
    {
        return GcHeapImpl::get_heap_size();
    }

    static int32_t get_collection_count()
    {
        return GcHeapImpl::get_collection_count();
    }

    static void set_gc_mode(GCMode mode)
    {
        GcHeapImpl::set_gc_mode(mode);
    }

    static void write_barrier(vm::RtObject** obj_ref_location, vm::RtObject* new_obj)
    {
        GcHeapImpl::write_barrier(obj_ref_location, new_obj);
    }

    static bool has_strict_wbarriers()
    {
        return GcHeapImpl::has_strict_wbarriers();
    }

    static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size)
    {
        return GcHeapImpl::allocate_object(klass, size);
    }

    static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
    {
        return GcHeapImpl::allocate_object(klass, size, site);
    }

    static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes)
    {
        return GcHeapImpl::allocate_array(arrClass, totalBytes);
    }

    static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes, const GcAllocSite& site)
    {
        return GcHeapImpl::allocate_array(arrClass, totalBytes, site);
    }
};
} // namespace gc
} // namespace leanclr

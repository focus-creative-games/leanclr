#pragma once

#include "core/rt_base.h"
#include "gc/gc_alloc_site.h"
#include "gc/gc_pressure.h"

#if defined(LEANCLR_GC_MARK_SWEEP)
#include "gc/mark_sweep/mark_sweep_heap.h"
#elif defined(LEANCLR_GC_ZERO_GC)
#include "gc/zero_gc/zero_gc_heap.h"
#else
#error "Define LEANCLR_GC_MARK_SWEEP or LEANCLR_GC_ZERO_GC for the leanclr target"
#endif

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

namespace detail
{
#if defined(LEANCLR_GC_MARK_SWEEP)
using GcHeapImpl = MarkSweepHeap;
#elif defined(LEANCLR_GC_ZERO_GC)
using GcHeapImpl = ZeroGcHeap;
#else
#error "Define LEANCLR_GC_MARK_SWEEP or LEANCLR_GC_ZERO_GC for the leanclr target"
#endif
} // namespace detail

class GarbageCollector
{
  public:
    static void initialize()
    {
        detail::GcHeapImpl::initialize();
    }

    static void collect()
    {
        detail::GcHeapImpl::collect();
    }

    static bool maybe_collect()
    {
        return detail::GcHeapImpl::maybe_collect();
    }

    static bool should_collect(bool force = false)
    {
        return detail::GcHeapImpl::should_collect(force);
    }

    static void record_pressure(int64_t bytes);
    static uint64_t get_effective_pressure();
    static uint64_t get_bytes_allocated_since_last_gc();

    static void set_pressure_config(const GcPressureConfig& config)
    {
        detail::GcHeapImpl::set_pressure_config(config);
    }

    static void* allocate_fixed(size_t size)
    {
        return detail::GcHeapImpl::allocate_fixed(size);
    }

    static void free_fixed(void* address)
    {
        detail::GcHeapImpl::free_fixed(address);
    }

    // static vm::RtObject** allocate_fixed_reference_array(size_t length);

    static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
    {
        return detail::GcHeapImpl::allocate_object(klass, size, site);
    }

    // static vm::RtObject* allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size, const GcAllocSite& site);
    static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes, const GcAllocSite& site)
    {
        return detail::GcHeapImpl::allocate_array(arrClass, totalBytes, site);
    }

    // Legacy entry points (no allocation site).
    static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size)
    {
        return detail::GcHeapImpl::allocate_object(klass, size);
    }

    // static vm::RtObject* allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size);
    static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes)
    {
        return detail::GcHeapImpl::allocate_array(arrClass, totalBytes);
    }

    static void write_barrier(vm::RtObject** obj_ref_location, vm::RtObject* new_obj)
    {
        detail::GcHeapImpl::write_barrier(obj_ref_location, new_obj);
    }

    static bool has_strict_wbarriers()
    {
        return detail::GcHeapImpl::has_strict_wbarriers();
    }

    static int64_t get_used_size()
    {
        return detail::GcHeapImpl::get_used_size();
    }

    static int64_t get_heap_size()
    {
        return detail::GcHeapImpl::get_heap_size();
    }

    static int32_t get_collection_count()
    {
        return detail::GcHeapImpl::get_collection_count();
    }

    static bool is_object_marked(const vm::RtObject* obj)
    {
        return detail::GcHeapImpl::is_object_marked(obj);
    }
};
} // namespace gc
} // namespace leanclr

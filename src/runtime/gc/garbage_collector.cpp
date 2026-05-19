#include "garbage_collector.h"

#include <cstring>

#include "gc/gc_alloc_site.h"

#if defined(LEANCLR_GC_MARK_SWEEP)
#include "gc/mark_sweep/mark_sweep_heap.h"
using GcHeapImpl = leanclr::gc::MarkSweepHeap;
#elif defined(LEANCLR_GC_ZERO_GC)
#include "gc/zero_gc/zero_gc_heap.h"
using GcHeapImpl = leanclr::gc::ZeroGcHeap;
#else
#error "Define LEANCLR_GC_MARK_SWEEP or LEANCLR_GC_ZERO_GC for the leanclr target"
#endif

namespace leanclr
{
namespace gc
{

void GarbageCollector::initialize()
{
    GcHeapImpl::initialize();
}

void GarbageCollector::collect()
{
    // FIXME
    return;
    GcHeapImpl::collect();
}

bool GarbageCollector::maybe_collect()
{
    // FIXME
    return false;
    return GcHeapImpl::maybe_collect();
}

bool GarbageCollector::should_collect(bool force)
{
    // FIXME
    return false;
    return GcHeapImpl::should_collect(force);
}

void GarbageCollector::record_pressure(int64_t bytes)
{
    GcPressure::record_external(bytes);
}

uint64_t GarbageCollector::get_effective_pressure()
{
    return GcPressure::get_effective_pressure();
}

uint64_t GarbageCollector::get_bytes_allocated_since_last_gc()
{
    return GcPressure::get_bytes_allocated_since_last_gc();
}

void GarbageCollector::set_pressure_config(const GcPressureConfig& config)
{
    GcHeapImpl::set_pressure_config(config);
}

void* GarbageCollector::allocate_fixed(size_t size)
{
    return GcHeapImpl::allocate_fixed(size);
}

void GarbageCollector::free_fixed(void* address)
{
    GcHeapImpl::free_fixed(address);
}

vm::RtObject** GarbageCollector::allocate_fixed_reference_array(size_t length)
{
    vm::RtObject** arr = static_cast<vm::RtObject**>(allocate_fixed(sizeof(vm::RtObject*) * length));
    if (arr != nullptr)
    {
        std::memset(arr, 0, sizeof(vm::RtObject*) * length);
    }
    return arr;
}

vm::RtObject* GarbageCollector::allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
{
    return GcHeapImpl::allocate_object(klass, size, site);
}

vm::RtObject* GarbageCollector::allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
{
    return GcHeapImpl::allocate_object_not_contains_references(klass, size, site);
}

vm::RtObject* GarbageCollector::allocate_object(const metadata::RtClass* klass, size_t size)
{
    return GcHeapImpl::allocate_object(klass, size);
}

vm::RtObject* GarbageCollector::allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size)
{
    return GcHeapImpl::allocate_object_not_contains_references(klass, size);
}

vm::RtObject* GarbageCollector::allocate_array(const metadata::RtClass* arrClass, size_t totalBytes)
{
    return GcHeapImpl::allocate_array(arrClass, totalBytes);
}

void GarbageCollector::write_barrier(vm::RtObject** obj_ref_location, vm::RtObject* new_obj)
{
    GcHeapImpl::write_barrier(obj_ref_location, new_obj);
}

int64_t GarbageCollector::get_used_size()
{
    return GcHeapImpl::get_used_size();
}

int64_t GarbageCollector::get_heap_size()
{
    return GcHeapImpl::get_heap_size();
}

int32_t GarbageCollector::get_collection_count()
{
    return GcHeapImpl::get_collection_count();
}

bool GarbageCollector::is_object_marked(const vm::RtObject* obj)
{
    return GcHeapImpl::is_object_marked(obj);
}

} // namespace gc
} // namespace leanclr

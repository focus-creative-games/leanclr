#include "gc/mark_sweep/mark_sweep_heap.h"

#if LEANCLR_GC_MARK_SWEEP

#include <cstring>

#include "alloc/general_allocation.h"
#include "gc/gc_config.h"
#include "gc/gc_common.h"
#include "gc/gc_scan.h"
#include "gc/handles/gc_handle_table.h"
#include "gc/roots/gc_roots.h"
#include "utils/rt_vector.h"
#include "vm/class.h"

namespace leanclr
{
namespace gc
{

static int64_t s_used_bytes = 0;
static int64_t s_heap_bytes = 0;

void MarkSweepHeap::initialize()
{
    GcPressureConfig cfg = {GC_DEFAULT_BYTE_THRESHOLD, GC_DEFAULT_SOFT_HEAP_LIMIT};
    GcPressure::initialize(cfg);
    s_used_bytes = 0;
    s_heap_bytes = 0;
}

void MarkSweepHeap::collect()
{
    GcPressure::on_collect();
}

bool MarkSweepHeap::should_collect(bool force)
{
    return GcPressure::should_collect(force);
}

bool MarkSweepHeap::maybe_collect()
{
    if (!should_collect(false))
    {
        return false;
    }
    collect();
    return true;
}

int64_t MarkSweepHeap::get_used_size()
{
    return s_used_bytes;
}

int64_t MarkSweepHeap::get_heap_size()
{
    return s_heap_bytes;
}

int32_t MarkSweepHeap::get_collection_count()
{
    return 0;
}

void MarkSweepHeap::set_pressure_config(const GcPressureConfig& config)
{
    GcPressure::set_config(config);
}

void* MarkSweepHeap::allocate_fixed(size_t size)
{
    return alloc::GeneralAllocation::malloc_zeroed(size);
}

void MarkSweepHeap::free_fixed(void* address)
{
    alloc::GeneralAllocation::free(address);
}

vm::RtObject* MarkSweepHeap::allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
{
    return allocate_object(klass, size);
}

vm::RtObject* MarkSweepHeap::allocate_object(const metadata::RtClass* klass, size_t size)
{
    assert(size >= sizeof(vm::RtObject));
    auto obj = (vm::RtObject*)alloc::GeneralAllocation::malloc_zeroed(size);
    obj->klass = const_cast<metadata::RtClass*>(klass);
    return obj;
}

vm::RtObject* MarkSweepHeap::allocate_array(const metadata::RtClass* arrClass, size_t totalBytes, const GcAllocSite& site)
{
    return allocate_object(arrClass, totalBytes);
}

vm::RtObject* MarkSweepHeap::allocate_array(const metadata::RtClass* arrClass, size_t totalBytes)
{
    return allocate_object(arrClass, totalBytes);
}

bool MarkSweepHeap::is_object_marked(const vm::RtObject* /*obj*/)
{
    return true;
}

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_MARK_SWEEP

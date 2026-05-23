#include "gc/zero_gc/zero_gc_heap.h"

#include <cstring>

#include "alloc/general_allocation.h"
#include "gc/gc_common.h"
#include "gc/gc_config.h"
#include "vm/class.h"

#if LEANCLR_GC_ZERO_GC

namespace leanclr
{
namespace gc
{

static int64_t s_used_bytes = 0;
static int64_t s_heap_bytes = 0;

void ZeroGcHeap::initialize()
{
    GcPressureConfig cfg = {GC_DEFAULT_BYTE_THRESHOLD, GC_DEFAULT_SOFT_HEAP_LIMIT};
    GcPressure::initialize(cfg);
    s_used_bytes = 0;
    s_heap_bytes = 0;
}

void ZeroGcHeap::collect()
{
    GcPressure::on_collect();
}

bool ZeroGcHeap::maybe_collect()
{
    return false;
}

bool ZeroGcHeap::should_collect(bool /*force*/)
{
    return false;
}

void* ZeroGcHeap::allocate_fixed(size_t size)
{
    return alloc::GeneralAllocation::malloc_zeroed(size);
}

void ZeroGcHeap::free_fixed(void* address)
{
    alloc::GeneralAllocation::free(address);
}

vm::RtObject* ZeroGcHeap::allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
{
    return allocate_object(klass, size);
}

vm::RtObject* ZeroGcHeap::allocate_object(const metadata::RtClass* klass, size_t size)
{
    assert(size >= sizeof(vm::RtObject));
    auto obj = (vm::RtObject*)alloc::GeneralAllocation::malloc_zeroed(size);
    obj->klass = const_cast<metadata::RtClass*>(klass);
    return obj;
}

vm::RtObject* ZeroGcHeap::allocate_array(const metadata::RtClass* arrClass, size_t totalBytes, const GcAllocSite& site)
{
    return allocate_object(arrClass, totalBytes);
}

vm::RtObject* ZeroGcHeap::allocate_array(const metadata::RtClass* arrClass, size_t totalBytes)
{
    return allocate_object(arrClass, totalBytes);
}

int64_t ZeroGcHeap::get_used_size()
{
    return s_used_bytes;
}

int64_t ZeroGcHeap::get_heap_size()
{
    return s_heap_bytes;
}

int32_t ZeroGcHeap::get_collection_count()
{
    return 0;
}

bool ZeroGcHeap::is_object_marked(const vm::RtObject* /*obj*/)
{
    return true;
}

void ZeroGcHeap::set_pressure_config(const GcPressureConfig& config)
{
    GcPressure::set_config(config);
}

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_ZERO_GC

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

void ZeroGcHeap::initialize(const Config& config)
{
    s_gc_mode = config.mode;
    GcPressure::initialize(config.pressure_config);
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

vm::RtObject* ZeroGcHeap::allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
{
    return allocate_object(klass, size);
}

vm::RtObject* ZeroGcHeap::allocate_object(const metadata::RtClass* klass, size_t size)
{
    assert(size >= sizeof(vm::RtObject));
    auto obj = (vm::RtObject*)alloc::GeneralAllocation::malloc_zeroed(size);
    obj->klass = const_cast<metadata::RtClass*>(klass);
    s_used_bytes += size;
    s_heap_bytes += size;
    GcPressure::on_alloc(size);
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

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_ZERO_GC

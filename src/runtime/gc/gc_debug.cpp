#include "gc/gc_debug.h"

#if LEANCLR_GC_DEBUG

#include <cstring>

#include "alloc/general_allocation.h"
#include "core/rt_base.h"

namespace leanclr
{
namespace gc
{

bool gc_debug_is_quarantined_tombstone(const vm::RtObject* obj)
{
    return obj->klass == nullptr;
}

void gc_debug_quarantine_object(vm::RtObject* obj, size_t size)
{
    void* duplicate = alloc::GeneralAllocation::malloc_zeroed(size);
    if (duplicate == nullptr)
    {
        panic("malloc_zeroed failed during GC debug quarantine");
    }
    std::memcpy(duplicate, obj, size);
    std::memset(obj, 0, size);
    obj->__sync_block = duplicate;
}

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_DEBUG

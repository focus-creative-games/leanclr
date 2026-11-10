#include "general_allocation.h"
#include "host/default_allocation.h"

namespace leanclr
{
namespace alloc
{
bool GeneralAllocation::s_callbacks_locked = false;

bool GeneralAllocation::set_memory_callbacks(MemoryCallbacks callbacks)
{
    if (!callbacks.malloc || !callbacks.calloc || !callbacks.realloc || !callbacks.free || !callbacks.aligned_malloc || !callbacks.aligned_realloc ||
        !callbacks.aligned_free)
        return false;
    const auto& current = memory_callbacks();
    if (s_callbacks_locked)
    {
        return callbacks.malloc == current.malloc && callbacks.calloc == current.calloc && callbacks.realloc == current.realloc &&
               callbacks.free == current.free && callbacks.aligned_malloc == current.aligned_malloc && callbacks.aligned_realloc == current.aligned_realloc &&
               callbacks.aligned_free == current.aligned_free;
    }
    memory_callbacks() = callbacks;
    return true;
}

MemoryCallbacks& GeneralAllocation::memory_callbacks()
{
    static MemoryCallbacks table = host::make_default_memory_callbacks();
    return table;
}
} // namespace alloc
} // namespace leanclr

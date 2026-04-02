#include "garbage_collector.h"
#include "alloc/general_allocation.h"
#include "metadata/rt_metadata.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace gc
{
void GarbageCollector::initialize()
{
    // Initialization logic for the garbage collector goes here
}

void* GarbageCollector::allocate_fixed(size_t size)
{
    // TODO: Implement fixed-size allocation logic
    return alloc::GeneralAllocation::malloc_zeroed(size);
}

void GarbageCollector::free_fixed(void* address)
{
    alloc::GeneralAllocation::free(address);
}

vm::RtObject** GarbageCollector::allocate_fixed_reference_array(size_t length)
{
    return alloc::GeneralAllocation::calloc_any<vm::RtObject*>(length);
}

vm::RtObject* GarbageCollector::allocate_object(const metadata::RtClass* klass, size_t size)
{
    // TODO: Implement object allocation logic
    assert(size >= sizeof(vm::RtObject));
    auto obj = (vm::RtObject*)alloc::GeneralAllocation::malloc_zeroed(size);
    obj->klass = const_cast<metadata::RtClass*>(klass);
    return obj;
}

vm::RtObject* GarbageCollector::allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size)
{
    return allocate_object(klass, size);
}

vm::RtObject* GarbageCollector::allocate_array(const metadata::RtClass* arrClass, size_t totalBytes)
{
    return allocate_object(arrClass, totalBytes);
}
} // namespace gc
} // namespace leanclr

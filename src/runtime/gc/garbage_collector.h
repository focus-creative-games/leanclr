#pragma once

#include "core/rt_base.h"
#include "gc/gc_alloc_site.h"
#include "gc/zero_gc/zero_gc_heap.h"

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
typedef ZeroGcHeap GcHeapImpl;
}

class GarbageCollector
{
  public:
    static void initialize()
    {
        detail::GcHeapImpl::initialize();
    }

    static void* allocate_fixed(size_t size)
    {
        return detail::GcHeapImpl::allocate_fixed(size);
    }

    static void free_fixed(void* address)
    {
        detail::GcHeapImpl::free_fixed(address);
    }

    static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size)
    {
        return detail::GcHeapImpl::allocate_object(klass, size);
    }

    static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
    {
        return detail::GcHeapImpl::allocate_object(klass, size, site);
    }

    static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes)
    {
        return detail::GcHeapImpl::allocate_array(arrClass, totalBytes);
    }

    static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes, const GcAllocSite& site)
    {
        return detail::GcHeapImpl::allocate_array(arrClass, totalBytes, site);
    }

    static void write_barrier(vm::RtObject** obj_ref_location, vm::RtObject* new_obj)
    {
        detail::GcHeapImpl::write_barrier(obj_ref_location, new_obj);
    }
};
} // namespace gc
} // namespace leanclr

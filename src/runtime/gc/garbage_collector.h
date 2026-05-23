#pragma once

#include "core/rt_base.h"
#include "gc/gc_alloc_site.h"

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
            static void initialize();

            static void* allocate_fixed(size_t size);
            static void free_fixed(void* address);
            static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size);
            static vm::RtObject* allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
            {
                return allocate_object(klass, size);
            }
            static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes);
            static vm::RtObject* allocate_array(const metadata::RtClass* arrClass, size_t totalBytes, const GcAllocSite& site)
            {
                return allocate_array(arrClass, totalBytes);
            }
            static void write_barrier(vm::RtObject** obj_ref_location, vm::RtObject* new_obj)
            {
                // TODO: implement write barrier
                *obj_ref_location = new_obj;
            }
        };
    } // namespace gc
} // namespace leanclr

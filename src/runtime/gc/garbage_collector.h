#pragma once

#include "rt_base.h"

namespace leanclr
{
namespace metadata
{
struct RtClass;
}

namespace leanclr
{
namespace vm
{
struct RtObject;
}

namespace leanclr
{
namespace gc
{

class GarbageCollector
{
  public:
    static void initialize();

    static void* allocate_fixed(size_t size);
    static vm::RtObject** allocate_fixed_reference_array(size_t length);
    static vm::RtObject* allocate_object(metadata::RtClass* klass, size_t size);
    static vm::RtObject* allocate_object_not_contains_references(metadata::RtClass* klass, size_t size);
    static vm::RtObject* allocate_array(metadata::RtClass* arrClass, size_t totalBytes);
    static void write_barrier(vm::RtObject** obj_ref_location, vm::RtObject* new_obj)
    {
        // TODO: implement write barrier
        *obj_ref_location = new_obj;
    }
};
} // namespace gc
} // namespace leanclr

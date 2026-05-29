#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace metadata
{
struct RtClass;
}

namespace gc
{

typedef void (*GcRootCallback)(vm::RtObject** slot, void* userdata);
typedef void (*GcVisitObjectRoot)(vm::RtObject* obj, void* userdata);
typedef void (*GcVisitObjectRootsScan)(GcVisitObjectRoot visit, void* userdata);
typedef void (*GcVisitUnknownBlock)(void* address, size_t size, void* userdata);
typedef void (*GcVisitUnknownBlocksScan)(GcVisitUnknownBlock visit, void* userdata);

class GcRoots
{
  public:
    static void register_slot(vm::RtObject** slot);
    static void unregister_slot(vm::RtObject** slot);
    static void register_static_fields(metadata::RtClass* klass);
    static void register_visit_object_roots(GcVisitObjectRootsScan scan);
    static void register_visit_unknown_blocks(GcVisitUnknownBlocksScan scan);
    static void foreach_root(GcRootCallback callback, void* userdata);
    static void foreach_object_root(GcVisitObjectRoot visit, void* userdata);
};

} // namespace gc
} // namespace leanclr

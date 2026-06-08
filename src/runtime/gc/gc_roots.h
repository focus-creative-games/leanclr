#pragma once

#include "vm/rt_managed_types.h"
#include "utils/segmented_address_bitmap.h"

namespace leanclr
{
namespace metadata
{
struct RtClass;
}

namespace gc
{

  typedef utils::SegmentedAddressBitmap<PTR_SIZE * 2, 4 * 1024, 1024> GCAliveObjectBitmap;

typedef void (*GcRootCallback)(vm::RtObject** slot, void* userdata);
typedef void (*GcVisitObjectRoot)(vm::RtObject* obj, void* userdata);
typedef void (*GcVisitObjectRootsScan)(GcVisitObjectRoot visit, void* userdata);

class GcRoots
{
  public:
    static void register_slot(vm::RtObject** slot);
    static void unregister_slot(vm::RtObject** slot);
    static void register_visit_object_roots(GcVisitObjectRootsScan scan);
    static void foreach_root(GCAliveObjectBitmap& alive_object_bitmap);
};

} // namespace gc
} // namespace leanclr

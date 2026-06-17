#pragma once

#include "gc/gc_roots.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace gc
{

class GcFinalizer
{
  public:
    static void on_object_allocated(vm::RtObject* obj);
    static void promote_unreachable(GCAliveObjectBitmap& alive_bitmap);
    static void run_pending_finalizers();
    static void wait_for_pending_finalizers();
    static void suppress_finalize(vm::RtObject* obj);
    static void reregister_for_finalize(vm::RtObject* obj);
    static void on_object_freed(vm::RtObject* obj);
};

} // namespace gc
} // namespace leanclr

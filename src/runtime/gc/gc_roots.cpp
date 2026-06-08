#include "gc/gc_roots.h"

#include "utils/rt_vector.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/gchandle.h"
#include "obj_scan_util.h"

namespace leanclr
{
namespace gc
{

static utils::Vector<vm::RtObject**> s_registered_slots;
static utils::Vector<GcVisitObjectRootsScan> s_visit_object_roots;

void GcRoots::register_slot(vm::RtObject** slot)
{
    s_registered_slots.push_back(slot);
}

void GcRoots::unregister_slot(vm::RtObject** slot)
{
    for (size_t i = 0; i < s_registered_slots.size(); ++i)
    {
        if (s_registered_slots[i] == slot)
        {
            s_registered_slots[i] = s_registered_slots[s_registered_slots.size() - 1];
            s_registered_slots.pop_back();
            return;
        }
    }
}

void GcRoots::register_visit_object_roots(GcVisitObjectRootsScan scan)
{
    s_visit_object_roots.push_back(scan);
}

static void visit_object_roots(GcVisitObjectRoot visit, void* userdata)
{
    for (size_t i = 0; i < s_visit_object_roots.size(); ++i)
    {
        s_visit_object_roots[i](visit, userdata);
    }
}

struct GcVisitContext
{
    GCAliveObjectBitmap& alive_object_bitmap;
};

static bool is_first_visit(vm::RtObject* obj, void* userdata)
{
    GcVisitContext* ctx = reinterpret_cast<GcVisitContext*>(userdata);
    return ctx->alive_object_bitmap.mark(obj);
}

static void on_visit_object(vm::RtObject* obj, void* userdata)
{
    ObjScanUtil::visit_object(obj, is_first_visit, userdata);
}

void GcRoots::foreach_root(GCAliveObjectBitmap& alive_object_bitmap)
{
    GcVisitContext ctx = { alive_object_bitmap };
    for (GcVisitObjectRootsScan visit : s_visit_object_roots)
    {
        visit(on_visit_object, &ctx);
    }
    for (size_t i = 0; i < s_registered_slots.size(); ++i)
    {
        vm::RtObject* obj = *s_registered_slots[i];
        if (obj != nullptr)
        {
            on_visit_object(obj, &ctx);
        }
    }
    vm::GCHandle::foreach_strong_handles(on_visit_object, &ctx);
    ObjScanUtil::visit_all_classes_static_data(is_first_visit, &ctx);
}

} // namespace gc
} // namespace leanclr

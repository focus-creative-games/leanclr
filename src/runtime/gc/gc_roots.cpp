#include "gc/gc_roots.h"

#include <cassert>

#include "utils/rt_vector.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/gchandle.h"
#include "gc/obj_scanner.h"
#include "interp/machine_state.h"
#include "interp/eval_stack_op.h"
#include "utils/rt_span.h"
#include "mark_sweep/mark_sweep_heap.h"

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

struct GcMarkContext
{
    GCAliveObjectBitmap& alive_object_bitmap;
    ObjScanner scanner;

    static bool visit(vm::RtObject* obj, void* userdata)
    {
        auto* ctx = static_cast<GcMarkContext*>(userdata);
        return ctx->alive_object_bitmap.mark(obj);
    }

    explicit GcMarkContext(GCAliveObjectBitmap& alive_object_bitmap)
        : alive_object_bitmap(alive_object_bitmap), scanner(visit, this)
    {
    }
};

static void on_visit_object(vm::RtObject* obj, void* userdata)
{
    auto* ctx = static_cast<GcMarkContext*>(userdata);
    ctx->scanner.visit_object(obj);
}

static void scan_stack(GcMarkContext &ctx)
{
    utils::Span<const interp::InterpFrame> frames = interp::MachineState::get_global_machine_state().get_active_frames();
    for (const interp::InterpFrame &frame : frames) 
    {
        auto offsets = frame.method->interp_data->reference_offsets;
        for (size_t offset : offsets)
        {
            vm::RtObject* obj = interp::EvalStackOp::get_param<vm::RtObject*>(frame.eval_stack_base, offset);
            if(obj == nullptr || !gc::MarkSweepHeap::is_valid_pointer(obj))
                continue;
                
            ctx.scanner.visit_object(obj);
        }
    }
}

void GcRoots::foreach_root(GCAliveObjectBitmap& alive_object_bitmap)
{
    GcMarkContext ctx(alive_object_bitmap);

    scan_stack(ctx);

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
    ctx.scanner.visit_all_classes_static_data();
    ctx.scanner.finish_visit();
}

} // namespace gc
} // namespace leanclr

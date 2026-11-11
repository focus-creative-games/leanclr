#include "host/services.h"
#include "gc/gc_finalizer.h"

#include "gc/obj_scanner.h"
#include "utils/hashmap.h"
#include "utils/rt_vector.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/runtime.h"

namespace leanclr
{
namespace gc
{

enum class FinalizerRegState : uint8_t
{
    Active,
    Suppressed,
    Finalized,
};

static utils::HashMap<vm::RtObject*, FinalizerRegState>& get_s_registry()
{
    static utils::HashMap<vm::RtObject*, FinalizerRegState> value;
    return value;
}

static utils::Vector<vm::RtObject*> s_freachable;
#if LEANCLR_BCL_CORECLR_NET10
static const utils::Vector<vm::RtObject*>* s_running_finalizers = nullptr;

static void visit_finalizer_roots(GcVisitObjectRoot visit, void* userdata)
{
    for (auto* object : s_freachable)
        visit(object, userdata);
    if (s_running_finalizers != nullptr)
        for (auto* object : *s_running_finalizers)
            visit(object, userdata);
}
#endif

struct PromoteScanContext
{
    GCAliveObjectBitmap* alive_bitmap;

    static bool visit(vm::RtObject* obj, void* userdata)
    {
        auto* ctx = static_cast<PromoteScanContext*>(userdata);
        return ctx->alive_bitmap->mark(obj);
    }
};

void GcFinalizer::on_object_allocated(vm::RtObject* obj)
{
    assert(obj != nullptr);
    if (!vm::Class::has_finalizer(obj->klass))
    {
        return;
    }
#if LEANCLR_BCL_CORECLR_NET10
    static bool roots_registered = false;
    if (!roots_registered)
    {
        GcRoots::register_visit_object_roots(visit_finalizer_roots);
        roots_registered = true;
    }
#endif
    get_s_registry()[obj] = FinalizerRegState::Active;
}

void GcFinalizer::promote_unreachable(GCAliveObjectBitmap& alive_bitmap)
{
    assert(s_freachable.empty());
    PromoteScanContext ctx{&alive_bitmap};
    ObjScanner scanner(PromoteScanContext::visit, &ctx);
    for (auto& entry : get_s_registry())
    {
        vm::RtObject* obj = entry.first;
        if (entry.second != FinalizerRegState::Active)
        {
            continue;
        }
        if (alive_bitmap.is_marked(obj))
        {
            continue;
        }

        s_freachable.push_back(obj);
        scanner.visit_object(obj);
    }
    scanner.finish_visit();
}

void GcFinalizer::run_pending_finalizers()
{
#if LEANCLR_BCL_CORECLR_NET10
    if (s_running_finalizers != nullptr)
        return;
#endif
    if (s_freachable.empty())
    {
        return;
    }

    utils::Vector<vm::RtObject*> pending(s_freachable);
    s_freachable.clear();
#if LEANCLR_BCL_CORECLR_NET10
    s_running_finalizers = &pending;
#endif

    for (vm::RtObject* obj : pending)
    {
        auto it = get_s_registry().find(obj);
        if (it == get_s_registry().end() || it->second != FinalizerRegState::Active)
        {
            continue;
        }

        it->second = FinalizerRegState::Finalized;

        assert(vm::Class::has_finalizer(obj->klass));

        const metadata::RtMethodInfo* finalizer = vm::Class::get_finalizer(obj->klass);
        auto ret = vm::Runtime::invoke_object_arguments_with_run_cctor(finalizer, obj, nullptr, 0);
        if (ret.is_err())
        {
            host::Services::logf(host::LogLevel::Error, "Failed to invoke finalizer, klass: %s.%s\n", obj->klass->namespaze, obj->klass->name);
        }
    }
#if LEANCLR_BCL_CORECLR_NET10
    s_running_finalizers = nullptr;
#endif
}

void GcFinalizer::wait_for_pending_finalizers()
{
    run_pending_finalizers();
}

void GcFinalizer::suppress_finalize(vm::RtObject* obj)
{
    if (obj == nullptr)
    {
        return;
    }
    if (!vm::Class::has_finalizer(obj->klass))
    {
        return;
    }
    auto it = get_s_registry().find(obj);
    if (it != get_s_registry().end() && it->second == FinalizerRegState::Active)
    {
        it->second = FinalizerRegState::Suppressed;
    }
}

void GcFinalizer::reregister_for_finalize(vm::RtObject* obj)
{
    if (obj == nullptr)
    {
        return;
    }
    if (!vm::Class::has_finalizer(obj->klass))
    {
        return;
    }
    get_s_registry()[obj] = FinalizerRegState::Active;
}

void GcFinalizer::on_object_freed(vm::RtObject* obj)
{
    if (!vm::Class::has_finalizer(obj->klass))
    {
        return;
    }
    get_s_registry().erase(obj);
}

} // namespace gc
} // namespace leanclr

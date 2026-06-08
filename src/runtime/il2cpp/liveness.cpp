#include "liveness.h"

#include "core/rt_base.h"
#include "utils/mem_op.h"
#include "utils/segmented_address_bitmap.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/object.h"
#include "vm/type.h"
#include "vm/rt_array.h"
#include "gc/obj_scan_util.h"

namespace leanclr
{
namespace il2cpp
{

using VisitedObjectBitmap = utils::SegmentedAddressBitmap<PTR_SIZE * 2, 4 * 1024, 64>;

struct LivenessState
{
    metadata::RtClass* filter;
    il2cpp_register_object_callback callback;
    void* userdata;
    il2cpp_liveness_reallocate_callback reallocate;
    VisitedObjectBitmap visited_objects;

    static constexpr size_t PENDING_CALLBACK_BATCH_SIZE = 256;
    vm::RtObject* pending_callback_batch[PENDING_CALLBACK_BATCH_SIZE];
    size_t pending_callback_batch_size;

    LivenessState(metadata::RtClass* filter, uint32_t max_object_count /* unused */, il2cpp_register_object_callback callback, void* userdata,
                  il2cpp_liveness_reallocate_callback reallocate)
        : filter(filter), callback(callback), userdata(userdata), reallocate(reallocate), pending_callback_batch_size(0)
    {
        (void)max_object_count;
        std::memset(pending_callback_batch, 0, sizeof(pending_callback_batch));
    }

    bool add_visited_object(vm::RtObject* obj)
    {
        return visited_objects.mark(obj);
    }

    void add_pending_callback(vm::RtObject* obj)
    {
        if (filter && !vm::Class::has_class_parent_fast(obj->klass, filter))
        {
            return;
        }
        pending_callback_batch[pending_callback_batch_size++] = obj;
        if (pending_callback_batch_size == PENDING_CALLBACK_BATCH_SIZE)
        {
            callback(pending_callback_batch, static_cast<int>(pending_callback_batch_size), userdata);
            pending_callback_batch_size = 0;
        }
    }

    void call_final_callbacks()
    {
        if (pending_callback_batch_size == 0)
        {
            return;
        }
        callback(pending_callback_batch, static_cast<int>(pending_callback_batch_size), userdata);
        pending_callback_batch_size = 0;
    }

    void finalize()
    {
    }
};

static bool is_first_visit(vm::RtObject* obj, void* userdata)
{
    auto liveness_state = reinterpret_cast<LivenessState*>(userdata);
    if (liveness_state->add_visited_object(obj))
    {
        liveness_state->add_pending_callback(obj);
        return true;
    }
    return false;
}

void* Liveness::allocate_struct(metadata::RtClass* filter, uint32_t max_object_count, il2cpp_register_object_callback callback, void* userdata,
                                il2cpp_liveness_reallocate_callback reallocate)
{
    if (filter)
    {
        auto ret = vm::Class::initialize_all(filter);
        if (ret.is_err())
        {
            assert(false && "Failed to initialize filter class");
            panic("Failed to initialize filter class");
            return nullptr;
        }
    }
    return new LivenessState(filter, max_object_count, callback, userdata, reallocate);
}

void Liveness::free_struct(void* state)
{
    delete reinterpret_cast<LivenessState*>(state);
}

void Liveness::calculation_from_root(vm::RtObject* root, void* state)
{
    auto liveness_state = reinterpret_cast<LivenessState*>(state);
    gc::ObjScanUtil::visit_object(root, is_first_visit, liveness_state);
    liveness_state->call_final_callbacks();
}

void Liveness::calculation_from_statics(void* state)
{
    auto liveness_state = reinterpret_cast<LivenessState*>(state);
    gc::ObjScanUtil::visit_all_classes_static_data(is_first_visit, liveness_state);
    liveness_state->call_final_callbacks();
}

void Liveness::finalize(void* state)
{
    reinterpret_cast<LivenessState*>(state)->finalize();
}
} // namespace il2cpp
} // namespace leanclr
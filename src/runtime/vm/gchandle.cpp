#include "gchandle.h"
#include "rt_string.h"
#include "rt_array.h"
#include "class.h"
#include "metadata/metadata_cache.h"
#include "gc/handles/gc_handle_table.h"

namespace leanclr
{
namespace vm
{

using gc::GcHandleKind;
using gc::GcHandleSlot;

#if !LEANCLR_USE_VOID_PTR_GCHANDLE
uint32_t GCHandle::get_handle_id(void* handle)
{
    return gc::GcHandleTable::get_slot_index(reinterpret_cast<GcHandleSlot*>(handle));
}

void* GCHandle::get_handle_by_id(uint32_t id)
{
    return gc::GcHandleTable::get_slot_by_index(id);
}
#endif

void* GCHandle::new_handle(RtObject* obj, bool pinned)
{
    return get_target_handle(obj, nullptr, (int32_t)(pinned ? GcHandleKind::Pinned : GcHandleKind::Strong));
}

void* GCHandle::new_weakref_handle(RtObject* obj, bool track_resurrection)
{
    return get_target_handle(obj, nullptr, (int32_t)(track_resurrection ? GcHandleKind::WeakTrackResurrection : GcHandleKind::Weak));
}

void GCHandle::free_handle(void* handle)
{
    gc::GcHandleTable::free_slot(reinterpret_cast<GcHandleSlot*>(handle));
}

RtObject* GCHandle::get_target(void* handle)
{
    return gc::GcHandleTable::get_target(reinterpret_cast<GcHandleSlot*>(handle));
}

void* GCHandle::get_target_handle(RtObject* obj, void* handle, int32_t type_)
{
    GcHandleSlot* slot = reinterpret_cast<GcHandleSlot*>(handle);

    if (type_ == -1)
    {
        assert(handle != nullptr);
        gc::GcHandleTable::set_target(slot, obj, slot->kind);
        return handle;
    }

    if (slot == nullptr)
    {
        return gc::GcHandleTable::alloc(static_cast<GcHandleKind>(type_), obj);
    }

    gc::GcHandleTable::set_target(slot, obj, static_cast<GcHandleKind>(type_));
    return slot;
}

void* GCHandle::get_addr_of_pinned_object(void* handle)
{
    GcHandleSlot* slot = reinterpret_cast<GcHandleSlot*>(handle);

    if (slot->kind != GcHandleKind::Pinned)
    {
        return reinterpret_cast<void*>(-2);
    }

    RtObject* obj = slot->target;
    if (obj == nullptr)
    {
        return nullptr;
    }

    const metadata::RtClass* klass = obj->klass;

    if (Class::is_array_or_szarray(klass))
    {
        RtArray* arr = reinterpret_cast<RtArray*>(obj);
        return vm::Array::get_array_data_start_as_ptr_void(arr);
    }

    if (Class::is_string_class(klass))
    {
        RtString* str = reinterpret_cast<RtString*>(obj);
        return const_cast<Utf16Char*>(vm::String::get_chars_ptr(str));
    }

    return obj + 1;
}

bool GCHandle::is_type_pinned(const metadata::RtClass* klass)
{
    if (Class::is_array_or_szarray(klass))
    {
        const metadata::RtClass* ele_klass = klass->element_class;
        if (Class::is_string_class(ele_klass) || Class::is_array_or_szarray(ele_klass))
        {
            return false;
        }
        return is_type_pinned(ele_klass);
    }

    return Class::is_string_class(klass) || Class::is_blittable(klass);
}

void GCHandle::foreach_strong_handles(void (*callback)(void*, void*), void* userData)
{
    struct ForeachCtx
    {
        void (*callback)(void*, void*);
        void* userData;
    };
    ForeachCtx ctx = {callback, userData};
    gc::GcHandleTable::foreach_strong_and_pinned(
        [](vm::RtObject** slot, void* ud)
        {
            ForeachCtx* c = static_cast<ForeachCtx*>(ud);
            if (*slot != nullptr)
            {
                c->callback(*slot, c->userData);
            }
        },
        &ctx);
}

} // namespace vm
} // namespace leanclr

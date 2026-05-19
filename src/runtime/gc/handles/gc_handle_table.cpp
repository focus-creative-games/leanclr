#include "gc/handles/gc_handle_table.h"

#include "gc/garbage_collector.h"
#include "utils/rt_vector.h"

namespace leanclr
{
namespace gc
{

static utils::Vector<GcHandleSlot> s_slots;
static utils::Vector<uint32_t> s_free_indices;

static GcHandleSlot* slot_at(uint32_t index)
{
    return &s_slots[index];
}

GcHandleSlot* GcHandleTable::alloc(GcHandleKind kind, vm::RtObject* obj)
{
    uint32_t index = 0;
    if (!s_free_indices.empty())
    {
        index = s_free_indices.back();
        s_free_indices.pop_back();
    }
    else
    {
        index = static_cast<uint32_t>(s_slots.size());
        s_slots.push_back({});
    }
    GcHandleSlot* slot = slot_at(index);
    slot->target = obj;
    slot->kind = kind;
    slot->used = true;
    return slot;
}

void GcHandleTable::free_slot(GcHandleSlot* slot)
{
    if (slot == nullptr)
    {
        return;
    }
    const ptrdiff_t index = slot - s_slots.data();
    if (index < 0 || static_cast<size_t>(index) >= s_slots.size())
    {
        return;
    }
    slot->target = nullptr;
    slot->kind = GcHandleKind::Strong;
    slot->used = false;
    s_free_indices.push_back(static_cast<uint32_t>(index));
}

void GcHandleTable::set_target(GcHandleSlot* slot, vm::RtObject* obj, GcHandleKind kind)
{
    if (slot == nullptr)
    {
        return;
    }
    slot->target = obj;
    slot->kind = kind;
    slot->used = true;
}

vm::RtObject* GcHandleTable::get_target(GcHandleSlot* slot)
{
    if (slot == nullptr || !slot->used)
    {
        return nullptr;
    }
    return slot->target;
}

void GcHandleTable::foreach_strong_and_pinned(void (*fn)(vm::RtObject**, void*), void* userdata)
{
    for (size_t i = 0; i < s_slots.size(); ++i)
    {
        GcHandleSlot& slot = s_slots[i];
        if (!slot.used || slot.target == nullptr)
        {
            continue;
        }
        if (slot.kind == GcHandleKind::Weak || slot.kind == GcHandleKind::WeakTrackResurrection)
        {
            continue;
        }
        fn(&slot.target, userdata);
    }
}

uint32_t GcHandleTable::get_slot_index(GcHandleSlot* slot)
{
    if (slot == nullptr || s_slots.empty())
    {
        return 0;
    }
    const ptrdiff_t index = slot - s_slots.data();
    if (index < 0 || static_cast<size_t>(index) >= s_slots.size())
    {
        return 0;
    }
    return static_cast<uint32_t>(index) + 1;
}

GcHandleSlot* GcHandleTable::get_slot_by_index(uint32_t index)
{
    if (index == 0 || index > s_slots.size())
    {
        return nullptr;
    }
    GcHandleSlot* slot = slot_at(index - 1);
    return slot->used ? slot : nullptr;
}

void GcHandleTable::sweep_weak_after_mark()
{
    for (size_t i = 0; i < s_slots.size(); ++i)
    {
        GcHandleSlot& slot = s_slots[i];
        if (!slot.used || slot.target == nullptr)
        {
            continue;
        }
        if (slot.kind != GcHandleKind::Weak && slot.kind != GcHandleKind::WeakTrackResurrection)
        {
            continue;
        }
        if (!GarbageCollector::is_object_marked(slot.target))
        {
            slot.target = nullptr;
        }
    }
}

} // namespace gc
} // namespace leanclr

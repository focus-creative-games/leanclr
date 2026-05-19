#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace gc
{

enum class GcHandleKind : uint8_t
{
    Weak = 0,
    WeakTrackResurrection = 1,
    Strong = 2,
    Pinned = 3,
};

struct GcHandleSlot
{
    vm::RtObject* target;
    GcHandleKind kind;
    bool used;
};

class GcHandleTable
{
  public:
    static GcHandleSlot* alloc(GcHandleKind kind, vm::RtObject* obj);
    static void free_slot(GcHandleSlot* slot);
    static void set_target(GcHandleSlot* slot, vm::RtObject* obj, GcHandleKind kind);
    static vm::RtObject* get_target(GcHandleSlot* slot);
    static void foreach_strong_and_pinned(void (*fn)(vm::RtObject**, void*), void* userdata);
    static void sweep_weak_after_mark();

    static uint32_t get_slot_index(GcHandleSlot* slot);
    static GcHandleSlot* get_slot_by_index(uint32_t index);
};

} // namespace gc
} // namespace leanclr

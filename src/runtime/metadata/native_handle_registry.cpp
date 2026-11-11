#include "native_handle_registry.h"
#if defined(LEANCLR_BCL_CORECLR_NET10)
#include "alloc/general_allocation.h"
#include "rt_metadata.h"

namespace leanclr
{
namespace metadata
{
namespace
{
uintptr_t* slots;
size_t capacity, live, used;
constexpr uintptr_t deleted = 1;

size_t hash(uintptr_t value)
{
    value >>= 2;
    value ^= value >> 16;
    value *= uintptr_t{0x85ebca6b};
    value ^= value >> 13;
    return static_cast<size_t>(value);
}

void put(uintptr_t key)
{
    size_t index = hash(key) & (capacity - 1);
    while (slots[index] && slots[index] != deleted)
        index = (index + 1) & (capacity - 1);
    if (!slots[index])
        ++used;
    slots[index] = key;
    ++live;
}

bool prepare()
{
    if (capacity && used + 1 <= capacity / 2)
        return true;
    size_t next = capacity ? (live + 1 <= capacity / 2 ? capacity : capacity * 2) : 64;
    if (next < capacity || next > SIZE_MAX / sizeof(uintptr_t))
        return false;
    auto* fresh = alloc::GeneralAllocation::calloc_any<uintptr_t>(next);
    if (!fresh)
        return false;
    auto* old = slots;
    size_t old_capacity = capacity;
    slots = fresh;
    capacity = next;
    live = used = 0;
    for (size_t i = 0; i < old_capacity; ++i)
        if (old[i] && old[i] != deleted)
            put(old[i]);
    alloc::GeneralAllocation::free(old);
    return true;
}
} // namespace

bool NativeHandleRegistry::contains(const void* pointer, NativeHandleKind kind)
{
    const uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
    if (!address || (address & 3) || !capacity)
        return false;
    const uintptr_t key = address | static_cast<uintptr_t>(kind);
    size_t index = hash(key) & (capacity - 1);
    while (slots[index])
    {
        if (slots[index] == key)
            return true;
        index = (index + 1) & (capacity - 1);
    }
    return false;
}

bool NativeHandleRegistry::insert(const void* pointer, NativeHandleKind kind)
{
    const uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
    if (!address || (address & 3))
        return false;
    if (contains(pointer, kind))
        return true;
    if (!prepare())
        return false;
    put(address | static_cast<uintptr_t>(kind));
    return true;
}

void NativeHandleRegistry::remove_module(const RtModuleDef* module)
{
    for (size_t i = 0; i < capacity; ++i)
    {
        uintptr_t value = slots[i];
        if (!value || value == deleted)
            continue;
        auto* pointer = reinterpret_cast<const void*>(value & ~uintptr_t{3});
        const RtClass* parent = nullptr;
        switch (static_cast<NativeHandleKind>(value & 3))
        {
        case NativeHandleKind::Method:
            parent = static_cast<const RtMethodInfo*>(pointer)->parent;
            break;
        case NativeHandleKind::Field:
            parent = static_cast<const RtFieldInfo*>(pointer)->parent;
            break;
        case NativeHandleKind::Property:
            parent = static_cast<const RtPropertyInfo*>(pointer)->parent;
            break;
        case NativeHandleKind::Event:
            parent = static_cast<const RtEventInfo*>(pointer)->parent;
            break;
        }
        if (parent && parent->image == module)
        {
            slots[i] = deleted;
            --live;
        }
    }
    if (!live)
    {
        alloc::GeneralAllocation::free(slots);
        slots = nullptr;
        capacity = used = 0;
    }
}
} // namespace metadata
} // namespace leanclr
#endif

#include "gchandle.h"
#include "rt_string.h"
#include "rt_array.h"
#include "class.h"
#include "metadata/metadata_cache.h"
#include "alloc/general_allocation.h"
#include "utils/hashmap.h"
#include <cstddef>

namespace leanclr
{
namespace vm
{
enum class GCHandleType : int32_t
{
    Weak = 0,
    WeakTrackResurrection = 1,
    Normal = 2,
    Pinned = 3,
};

// HandleInfo struct for managing GC handles
struct HandleInfo
{
    RtObject* obj;
    HandleInfo* next;
    GCHandleType type_;
    GCHandleId id;
};
static_assert(offsetof(HandleInfo, obj) == 0, "GCHandle target slot must stay at the start of HandleInfo");

struct DependentHandleInfo
{
    RtObject* target;
    RtObject* dependent;
    bool freed;
};
static_assert(offsetof(DependentHandleInfo, target) == 0, "DependentHandle target slot must stay at the start of DependentHandleInfo");

// Head of the freed handle list
static HandleInfo* s_freed_handle_head = nullptr;
static GCHandleId s_last_handle_id = 0;

// TODO: optimize this
static utils::HashMap<GCHandleId, HandleInfo*> s_handle_map;
static utils::Vector<DependentHandleInfo*> s_dependent_handles;

// Allocate a new handle or reuse a freed one
static HandleInfo* alloc_handle()
{
    if (s_freed_handle_head == nullptr)
    {
        // Allocate a new handle
        HandleInfo* h = alloc::GeneralAllocation::malloc_any_zeroed<HandleInfo>();
        assert(s_last_handle_id < INT32_MAX);
        h->id = ++s_last_handle_id;
        s_handle_map[h->id] = h;
        return h;
    }
    else
    {
        // Reuse a freed handle
        HandleInfo* h = s_freed_handle_head;
        s_freed_handle_head = h->next;
        assert(h->id == 0);
        h->id = ++s_last_handle_id;
        s_handle_map[h->id] = h;
        return h;
    }
}

// Free a handle implementation
static void free_handle_impl(HandleInfo* handle)
{
    if (handle == nullptr)
    {
        return;
    }
    // Reset handle and add to freed list
    handle->obj = nullptr;
    handle->type_ = GCHandleType::Normal;
    handle->next = s_freed_handle_head;
    assert(s_handle_map.find(handle->id) != s_handle_map.end());
    s_handle_map.erase(handle->id);
    handle->id = 0;
    s_freed_handle_head = handle;
}

// Public API implementations

#if LEANCLR_USE_VOID_PTR_GCHANDLE
GCHandleId GCHandle::get_handle_id(void* handle)
{
    return reinterpret_cast<GCHandleId>(handle);
}

void* GCHandle::get_handle_by_id(GCHandleId id)
{
    return reinterpret_cast<void*>(id);
}
#else
GCHandleId GCHandle::get_handle_id(void* handle)
{
    HandleInfo* h = reinterpret_cast<HandleInfo*>(handle);
    return h->id;
}

void* GCHandle::get_handle_by_id(GCHandleId id)
{
    auto it = s_handle_map.find(id);
    if (it != s_handle_map.end())
    {
        return it->second;
    }
    return nullptr;
}
#endif

void* GCHandle::new_handle(RtObject* obj, bool pinned)
{
    return get_target_handle(obj, nullptr, (int32_t)(pinned ? GCHandleType::Pinned : GCHandleType::Normal));
}

void* GCHandle::new_weakref_handle(RtObject* obj, bool track_resurrection)
{
    return get_target_handle(obj, nullptr, (int32_t)(track_resurrection ? GCHandleType::WeakTrackResurrection : GCHandleType::Weak));
}

void GCHandle::free_handle(void* handle)
{
    HandleInfo* h = reinterpret_cast<HandleInfo*>(handle);
    free_handle_impl(h);
}

RtObject* GCHandle::get_target(void* handle)
{
    HandleInfo* h = reinterpret_cast<HandleInfo*>(handle);
    if (h == nullptr)
    {
        return nullptr;
    }
    return h->obj;
}

RtObject** GCHandle::get_target_slot(void* handle)
{
    HandleInfo* h = reinterpret_cast<HandleInfo*>(handle);
    if (h == nullptr)
    {
        return nullptr;
    }
    return &h->obj;
}

void* GCHandle::get_handle_by_target_slot(RtObject** slot)
{
    if (slot == nullptr)
    {
        return nullptr;
    }
    return reinterpret_cast<HandleInfo*>(slot);
}

void* GCHandle::get_target_handle(RtObject* obj, void* handle, int32_t type_)
{
    HandleInfo* h = reinterpret_cast<HandleInfo*>(handle);

    if (type_ == -1)
    {
        // Update object
        assert(handle != 0);
        h->obj = obj;
        return handle;
    }

    if (h == nullptr)
    {
        // Allocate new handle
        HandleInfo* new_handle = alloc_handle();
        new_handle->obj = obj;
        new_handle->type_ = static_cast<GCHandleType>(type_);
        new_handle->next = nullptr;
        return new_handle;
    }
    else
    {
        // Update existing handle
        h->obj = obj;
        h->type_ = static_cast<GCHandleType>(type_);
        return handle;
    }
}

void* GCHandle::get_addr_of_pinned_object(void* handle)
{
    HandleInfo* h = reinterpret_cast<HandleInfo*>(handle);

    if (h->type_ != GCHandleType::Pinned)
    {
        // Not a pinned handle
        return reinterpret_cast<void*>(-2);
    }

    RtObject* obj = h->obj;
    if (obj == nullptr)
    {
        return nullptr;
    }

    const metadata::RtClass* klass = obj->klass;

    if (Class::is_array_or_szarray(klass))
    {
        // For arrays, return pointer to array data
        RtArray* arr = reinterpret_cast<RtArray*>(obj);
        return vm::Array::get_array_data_start_as_ptr_void(arr);
    }

    if (Class::is_string_class(klass))
    {
        // For strings, return pointer to character data
        RtString* str = reinterpret_cast<RtString*>(obj);
        return const_cast<Utf16Char*>(vm::String::get_chars_ptr(str));
    }

    // For objects, return pointer to first field (skip object header)
    return obj + 1;
}

void* GCHandle::new_dependent_handle(RtObject* target, RtObject* dependent)
{
    DependentHandleInfo* handle = alloc::GeneralAllocation::malloc_any_zeroed<DependentHandleInfo>();
    handle->target = target;
    handle->dependent = dependent;
    handle->freed = false;
    s_dependent_handles.push_back(handle);
    return handle;
}

RtObject* GCHandle::get_dependent_handle_target(void* handle)
{
    DependentHandleInfo* h = reinterpret_cast<DependentHandleInfo*>(handle);
    if (h == nullptr || h->freed)
    {
        return nullptr;
    }
    return h->target;
}

RtObject* GCHandle::get_dependent_handle_dependent(void* handle)
{
    DependentHandleInfo* h = reinterpret_cast<DependentHandleInfo*>(handle);
    if (h == nullptr || h->freed || h->target == nullptr)
    {
        return nullptr;
    }
    return h->dependent;
}

RtObject* GCHandle::get_dependent_handle_target_and_dependent(void* handle, RtObject** dependent)
{
    DependentHandleInfo* h = reinterpret_cast<DependentHandleInfo*>(handle);
    if (dependent)
    {
        *dependent = nullptr;
    }
    if (h == nullptr || h->freed)
    {
        return nullptr;
    }
    if (dependent && h->target != nullptr)
    {
        *dependent = h->dependent;
    }
    return h->target;
}

void GCHandle::set_dependent_handle_target_to_null(void* handle)
{
    DependentHandleInfo* h = reinterpret_cast<DependentHandleInfo*>(handle);
    if (h == nullptr || h->freed)
    {
        return;
    }
    h->target = nullptr;
}

void GCHandle::set_dependent_handle_dependent(void* handle, RtObject* dependent)
{
    DependentHandleInfo* h = reinterpret_cast<DependentHandleInfo*>(handle);
    if (h == nullptr || h->freed)
    {
        return;
    }
    h->dependent = dependent;
}

bool GCHandle::free_dependent_handle(void* handle)
{
    DependentHandleInfo* h = reinterpret_cast<DependentHandleInfo*>(handle);
    if (h == nullptr || h->freed)
    {
        return true;
    }
    h->target = nullptr;
    h->dependent = nullptr;
    h->freed = true;
    return true;
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

void GCHandle::foreach_strong_handles(void (*callback)(vm::RtObject*, void*), void* userData)
{
    for (auto it = s_handle_map.begin(); it != s_handle_map.end(); ++it)
    {
        HandleInfo* hi = it->second;
        if (hi->type_ == GCHandleType::Weak || hi->type_ == GCHandleType::WeakTrackResurrection)
        {
            continue;
        }
        if (hi->obj != nullptr)
        {
            callback(hi->obj, userData);
        }
    }
    for (size_t i = 0; i < s_dependent_handles.size(); ++i)
    {
        DependentHandleInfo* hi = s_dependent_handles[i];
        if (hi == nullptr || hi->freed)
        {
            continue;
        }
        if (hi->target != nullptr)
        {
            callback(hi->target, userData);
        }
        if (hi->dependent != nullptr)
        {
            callback(hi->dependent, userData);
        }
    }
}

void GCHandle::sweep_weak_handles(bool (*is_object_marked)(vm::RtObject*, void* ctx), void* ctx)
{
    for (auto it = s_handle_map.begin(); it != s_handle_map.end(); ++it)
    {
        HandleInfo* hi = it->second;
        if (hi->type_ != GCHandleType::Weak && hi->type_ != GCHandleType::WeakTrackResurrection)
        {
            continue;
        }
        if (hi->obj != nullptr && !is_object_marked(hi->obj, ctx))
        {
            hi->obj = nullptr;
        }
    }
}

} // namespace vm
} // namespace leanclr

#include "gchandle.h"
#include "rt_string.h"
#include "rt_array.h"
#include "class.h"
#include "metadata/metadata_cache.h"
#include "alloc/general_allocation.h"
#include "utils/hashmap.h"

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
    uint32_t id;
};

// Head of the freed handle list
static HandleInfo* s_freed_handle_head = nullptr;
static uint32_t s_last_handle_id = 0;

// TODO: optimize this
static utils::HashMap<uint32_t, HandleInfo*> s_handle_map;
// Allocate a new handle or reuse a freed one
static HandleInfo* alloc_handle()
{
    if (s_freed_handle_head == nullptr)
    {
        // Allocate a new handle
        HandleInfo* h = alloc::GeneralAllocation::malloc_any_zeroed<HandleInfo>();
        assert(s_last_handle_id < UINT32_MAX);
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
    assert(handle->obj != nullptr);
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

#if !LEANCLR_USE_VOID_PTR_GCHANDLE
uint32_t GCHandle::get_handle_id(void* handle)
{
    HandleInfo* h = reinterpret_cast<HandleInfo*>(handle);
    return h->id;
}

void* GCHandle::get_handle_by_id(uint32_t id)
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
    for (auto it = s_handle_map.begin(); it != s_handle_map.end(); ++it)
    {
        callback(it->second->obj, userData);
    }
}

} // namespace vm
} // namespace leanclr

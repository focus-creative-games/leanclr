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
#if LEANCLR_BCL_CORECLR_NET10
    Dependent = 4,
#endif
};

// HandleInfo struct for managing GC handles
struct HandleInfo
{
    RtObject* obj;
    HandleInfo* next;
    GCHandleType type_;
    GCHandleId id;
#if LEANCLR_BCL_CORECLR_NET10
    RtObject* dependent;
#endif
};

// Head of the freed handle list
static HandleInfo* s_freed_handle_head = nullptr;
static GCHandleId s_last_handle_id = 0;

// Fallible index: no STL allocations on the handle creation path. Empty slots
// are null; erased slots use a stable marker so collision chains stay intact.
template <bool AddressKey = false>
class HandleIndex
{
    HandleInfo** slots_ = nullptr;
    size_t capacity_ = 0, live_ = 0, used_ = 0;
    HandleInfo deleted_{};

    static uintptr_t key(const HandleInfo* handle)
    {
        if constexpr (AddressKey)
            return reinterpret_cast<uintptr_t>(&handle->obj);
        else
            return static_cast<uintptr_t>(handle->id);
    }

    static size_t hash(uintptr_t value)
    {
        if constexpr (AddressKey)
        {
            value >>= 3;
            value ^= value >> 16;
            value *= uintptr_t{0x85ebca6b};
            value ^= value >> 13;
            return static_cast<size_t>(value);
        }
        else
            return static_cast<size_t>(value) * size_t(2654435761u);
    }

  public:
    ~HandleIndex()
    {
        alloc::GeneralAllocation::free(slots_);
    }

    bool prepare_insert()
    {
        if (capacity_ && used_ < capacity_ - capacity_ / 4)
            return true;
        size_t next = capacity_ ? capacity_ : 16;
        if (live_ >= next / 2 && !utils::CheckedSize::multiply(next, 2, next))
            return false;
        auto** replacement = alloc::GeneralAllocation::calloc_any<HandleInfo*>(next);
        if (!replacement)
            return false;
        for (size_t i = 0; i < capacity_; ++i)
        {
            auto* h = slots_[i];
            if (!h || h == &deleted_)
                continue;
            size_t at = hash(key(h)) & (next - 1);
            while (replacement[at])
                at = (at + 1) & (next - 1);
            replacement[at] = h;
        }
        alloc::GeneralAllocation::free(slots_);
        slots_ = replacement;
        capacity_ = next;
        used_ = live_;
        return true;
    }

    void insert(HandleInfo* h)
    {
        size_t at = hash(key(h)) & (capacity_ - 1);
        while (slots_[at] && slots_[at] != &deleted_)
            at = (at + 1) & (capacity_ - 1);
        if (!slots_[at])
            ++used_;
        slots_[at] = h;
        ++live_;
    }

    HandleInfo* find(uintptr_t id) const
    {
        if (!id || !capacity_)
            return nullptr;
        size_t at = hash(id) & (capacity_ - 1);
        while (slots_[at])
        {
            if (slots_[at] != &deleted_ && key(slots_[at]) == id)
                return slots_[at];
            at = (at + 1) & (capacity_ - 1);
        }
        return nullptr;
    }

    void erase(uintptr_t id)
    {
        size_t at = hash(id) & (capacity_ - 1);
        while (slots_[at])
        {
            if (slots_[at] != &deleted_ && key(slots_[at]) == id)
            {
                slots_[at] = &deleted_;
                --live_;
                return;
            }
            at = (at + 1) & (capacity_ - 1);
        }
    }

    size_t capacity() const
    {
        return capacity_;
    }

    HandleInfo* at(size_t i) const
    {
        return slots_[i] == &deleted_ ? nullptr : slots_[i];
    }
};

static HandleIndex<> s_handle_map;
#if LEANCLR_BCL_CORECLR_NET10
// Exact slot addresses are validated without dereferencing caller-provided
// pointers. Both indices are prepared before publishing an allocation.
static HandleIndex<true> s_handle_address_map;
#endif

// Allocate a new handle or reuse a freed one
static HandleInfo* alloc_handle()
{
    if (s_last_handle_id >= INT32_MAX || !s_handle_map.prepare_insert())
        return nullptr;
#if LEANCLR_BCL_CORECLR_NET10
    if (!s_handle_address_map.prepare_insert())
        return nullptr;
#endif
    if (s_freed_handle_head == nullptr)
    {
        // Allocate a new handle
        HandleInfo* h = alloc::GeneralAllocation::malloc_any_zeroed<HandleInfo>();
        if (!h)
            return nullptr;
        h->id = ++s_last_handle_id;
        s_handle_map.insert(h);
#if LEANCLR_BCL_CORECLR_NET10
        s_handle_address_map.insert(h);
#endif
        return h;
    }
    else
    {
        // Reuse a freed handle
        HandleInfo* h = s_freed_handle_head;
        s_freed_handle_head = h->next;
        assert(h->id == 0);
        h->id = ++s_last_handle_id;
        s_handle_map.insert(h);
#if LEANCLR_BCL_CORECLR_NET10
        s_handle_address_map.insert(h);
#endif
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
#if LEANCLR_BCL_CORECLR_NET10
    handle->dependent = nullptr;
#endif
    handle->type_ = GCHandleType::Normal;
    handle->next = s_freed_handle_head;
    assert(s_handle_map.find(handle->id) == handle);
    s_handle_map.erase(handle->id);
#if LEANCLR_BCL_CORECLR_NET10
    s_handle_address_map.erase(reinterpret_cast<uintptr_t>(&handle->obj));
#endif
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
    return h ? h->id : 0;
}

void* GCHandle::get_handle_by_id(GCHandleId id)
{
    return s_handle_map.find(id);
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
        if (!new_handle)
            return nullptr;
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

void GCHandle::foreach_strong_handles(void (*callback)(vm::RtObject*, void*), void* userData)
{
    for (size_t i = 0; i < s_handle_map.capacity(); ++i)
    {
        HandleInfo* hi = s_handle_map.at(i);
        if (!hi)
            continue;
        if (hi->type_ == GCHandleType::Weak || hi->type_ == GCHandleType::WeakTrackResurrection)
        {
            continue;
        }
#if LEANCLR_BCL_CORECLR_NET10
        if (hi->type_ == GCHandleType::Dependent)
            continue;
#endif
        if (hi->obj != nullptr)
        {
            callback(hi->obj, userData);
        }
    }
}

void GCHandle::sweep_weak_handles(bool (*is_object_marked)(vm::RtObject*, void* ctx), void* ctx, bool track_resurrection)
{
    for (size_t i = 0; i < s_handle_map.capacity(); ++i)
    {
        HandleInfo* hi = s_handle_map.at(i);
        if (!hi)
            continue;
#if LEANCLR_BCL_CORECLR_NET10
        if (hi->type_ == GCHandleType::Dependent && !track_resurrection)
        {
            if (hi->obj && !is_object_marked(hi->obj, ctx))
            {
                hi->obj = nullptr;
                hi->dependent = nullptr;
            }
            continue;
        }
#endif
        if (hi->type_ != (track_resurrection ? GCHandleType::WeakTrackResurrection : GCHandleType::Weak))
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

#if LEANCLR_BCL_CORECLR_NET10
namespace leanclr
{
namespace vm
{
RtObject** GCHandle::get_target_slot(void* handle)
{
    auto* info = static_cast<HandleInfo*>(handle);
    return info ? &info->obj : nullptr;
}

void* GCHandle::get_handle_by_target_slot(RtObject** slot)
{
    return s_handle_address_map.find(reinterpret_cast<uintptr_t>(slot));
}

void* GCHandle::new_dependent_handle(RtObject* target, RtObject* dependent)
{
    auto* info = alloc_handle();
    if (!info)
        return nullptr;
    info->obj = target;
    info->dependent = target ? dependent : nullptr;
    info->type_ = GCHandleType::Dependent;
    info->next = nullptr;
    return info;
}

RtObject* GCHandle::get_dependent_handle_target(void* handle)
{
    auto* info = static_cast<HandleInfo*>(handle);
    return info && info->type_ == GCHandleType::Dependent ? info->obj : nullptr;
}

RtObject* GCHandle::get_dependent_handle_dependent(void* handle)
{
    auto* info = static_cast<HandleInfo*>(handle);
    return info && info->type_ == GCHandleType::Dependent && info->obj ? info->dependent : nullptr;
}

RtObject* GCHandle::get_dependent_handle_target_and_dependent(void* handle, RtObject** dependent)
{
    auto* target = get_dependent_handle_target(handle);
    if (dependent)
        *dependent = target ? get_dependent_handle_dependent(handle) : nullptr;
    return target;
}

void GCHandle::set_dependent_handle_target_to_null(void* handle)
{
    auto* info = static_cast<HandleInfo*>(handle);
    if (info && info->type_ == GCHandleType::Dependent)
    {
        info->obj = nullptr;
        info->dependent = nullptr;
    }
}

void GCHandle::set_dependent_handle_dependent(void* handle, RtObject* dependent)
{
    auto* info = static_cast<HandleInfo*>(handle);
    if (info && info->type_ == GCHandleType::Dependent)
        info->dependent = info->obj ? dependent : nullptr;
}

bool GCHandle::free_dependent_handle(void* handle)
{
    auto* info = static_cast<HandleInfo*>(handle);
    if (!info || info->type_ != GCHandleType::Dependent)
        return false;
    free_handle_impl(info);
    return true;
}

bool GCHandle::visit_dependent_handles(bool (*is_marked)(RtObject*, void*), void (*visit)(RtObject*, void*), void* context)
{
    bool changed = false;
    for (size_t i = 0; i < s_handle_map.capacity(); ++i)
    {
        auto* info = s_handle_map.at(i);
        if (info && info->type_ == GCHandleType::Dependent && info->obj && info->dependent && is_marked(info->obj, context) &&
            !is_marked(info->dependent, context))
        {
            visit(info->dependent, context);
            changed = true;
        }
    }
    return changed;
}
} // namespace vm
} // namespace leanclr
#endif

#pragma once

#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <new>
#include <utility>

#include "core/rt_base.h"
#include "utils/checked_size.h"

namespace leanclr
{
namespace alloc
{
struct MemoryCallbacks
{
    void* (*malloc)(size_t size);
    void* (*aligned_malloc)(size_t size, size_t alignment);
    void (*free)(void* ptr);
    void (*aligned_free)(void* ptr);
    void* (*calloc)(size_t nmemb, size_t size);
    void* (*realloc)(void* ptr, size_t size);
    void* (*aligned_realloc)(void* ptr, size_t size, size_t alignment);
};

class GeneralAllocation
{
  private:
    static MemoryCallbacks& memory_callbacks();
    static bool s_callbacks_locked;

  public:
    // Configure on the single host/runtime thread before the first allocation
    // attempt. Invalid tables and later changes are rejected without mutation.
    // Reinstalling the exact same function pointers is an idempotent success.
    static bool set_memory_callbacks(MemoryCallbacks callbacks);

    template <typename T>
    static T* require_allocation(T* ptr)
    {
        if (!ptr)
            leanclr::panic("Infallible runtime allocation failed");
        return ptr;
    }

    static MemoryCallbacks get_memory_callbacks()
    {
        return memory_callbacks();
    }

    static void* malloc(size_t size)
    {
        s_callbacks_locked = true;
        return memory_callbacks().malloc ? memory_callbacks().malloc(size) : nullptr;
    }

    template <typename T>
    static T* malloc_any()
    {
        void* ptr = alignof(T) > alignof(std::max_align_t) ? aligned_malloc(sizeof(T), alignof(T)) : malloc(sizeof(T));
        return static_cast<T*>(ptr);
    }

    static void* malloc_zeroed(size_t size)
    {
        return calloc(1, size);
    }

    template <typename T>
    static T* malloc_any_zeroed()
    {
        return calloc_any<T>(1);
    }

    static void* calloc(size_t count, size_t size)
    {
        size_t total;
        if (!utils::CheckedSize::multiply(count, size, total))
            return nullptr;
        s_callbacks_locked = true;
        return memory_callbacks().calloc ? memory_callbacks().calloc(count, size) : nullptr;
    }

    template <typename T>
    static T* calloc_any(size_t count)
    {
        if (alignof(T) <= alignof(std::max_align_t))
            return static_cast<T*>(calloc(count, sizeof(T)));
        size_t bytes;
        if (!utils::CheckedSize::multiply(count, sizeof(T), bytes))
            return nullptr;
        auto* ptr = static_cast<T*>(aligned_malloc(bytes, alignof(T)));
        if (ptr && bytes)
            std::memset(ptr, 0, bytes);
        return ptr;
    }

    static void* realloc(void* ptr, size_t size)
    {
        s_callbacks_locked = true;
        return memory_callbacks().realloc ? memory_callbacks().realloc(ptr, size) : nullptr;
    }

    static void* aligned_malloc(size_t size, size_t alignment)
    {
        if (alignment == 0 || (alignment & (alignment - 1)) != 0)
            return nullptr;
        s_callbacks_locked = true;
        return memory_callbacks().aligned_malloc ? memory_callbacks().aligned_malloc(size, alignment) : nullptr;
    }

    static void* aligned_realloc(void* ptr, size_t size, size_t alignment)
    {
        if (alignment == 0 || (alignment & (alignment - 1)) != 0)
            return nullptr;
        s_callbacks_locked = true;
        return memory_callbacks().aligned_realloc ? memory_callbacks().aligned_realloc(ptr, size, alignment) : nullptr;
    }

    template <typename T, typename... Args>
    static T* new_any(Args&&... args)
    {
        void* ptr = malloc_any_zeroed<T>();
        if (ptr)
        {
            return new (ptr) T(std::forward<Args>(args)...);
        }
        return nullptr;
    }

    template <typename T>
    static void delete_any(T* ptr)
    {
        if (ptr)
        {
            ptr->~T();
            free_any(ptr);
        }
    }

    // Pair typed allocations with typed release to preserve the allocator domain.
    template <typename T>
    static void free_any(T* ptr)
    {
        if (alignof(T) > alignof(std::max_align_t))
            aligned_free(ptr);
        else
            free(ptr);
    }

    static void free(void* ptr)
    {
        if (ptr)
        {
            memory_callbacks().free(ptr);
        }
    }

    static void aligned_free(void* ptr)
    {
        if (ptr)
        {
            memory_callbacks().aligned_free(ptr);
        }
    }

    static void free_and_nullify(void*& ptr_location)
    {
        if (ptr_location)
        {
            memory_callbacks().free(ptr_location);
            ptr_location = nullptr;
        }
    }
};

class ScopeFreeGuard
{
  private:
    void* ptr;

  public:
    ScopeFreeGuard(void* ptr) : ptr(ptr)
    {
    }

    ScopeFreeGuard(const ScopeFreeGuard&) = delete;
    ScopeFreeGuard& operator=(const ScopeFreeGuard&) = delete;
    ScopeFreeGuard(ScopeFreeGuard&&) = delete;
    ScopeFreeGuard& operator=(ScopeFreeGuard&&) = delete;

    ~ScopeFreeGuard()
    {
        GeneralAllocation::free(ptr);
    }
};
} // namespace alloc
} // namespace leanclr

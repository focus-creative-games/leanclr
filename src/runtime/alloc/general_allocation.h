#pragma once

#include <cstdlib>
#include <cstring>

#include "core/rt_base.h"

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
    static MemoryCallbacks s_memory_callbacks;

  public:
    static void set_memory_callbacks(MemoryCallbacks callbacks)
    {
        s_memory_callbacks = callbacks;
    }

    static void* malloc(size_t size)
    {
        return s_memory_callbacks.malloc(size);
    }

    template <typename T>
    static T* malloc_any()
    {
        void* ptr = malloc(sizeof(T));
        return static_cast<T*>(ptr);
    }

    static void* malloc_zeroed(size_t size)
    {
        return s_memory_callbacks.calloc(1, size);
    }

    template <typename T>
    static T* malloc_any_zeroed()
    {
        void* ptr = malloc_zeroed(sizeof(T));
        return static_cast<T*>(ptr);
    }

    static void* calloc(size_t count, size_t size)
    {
        return s_memory_callbacks.calloc(count, size);
    }

    template <typename T>
    static T* calloc_any(size_t count)
    {
        void* ptr = calloc(count, sizeof(T));
        return static_cast<T*>(ptr);
    }

    static void* realloc(void* ptr, size_t size)
    {
        return s_memory_callbacks.realloc(ptr, size);
    }

    static void* aligned_malloc(size_t size, size_t alignment)
    {
        return s_memory_callbacks.aligned_malloc(size, alignment);
    }

    static void* aligned_realloc(void* ptr, size_t size, size_t alignment)
    {
        return s_memory_callbacks.aligned_realloc(ptr, size, alignment);
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
            free(ptr);
        }
    }

    static void free(void* ptr)
    {
        if (ptr)
        {
            s_memory_callbacks.free(ptr);
        }
    }

    static void aligned_free(void* ptr)
    {
        if (ptr)
        {
            s_memory_callbacks.aligned_free(ptr);
        }
    }

    static void free_and_nullify(void*& ptr_location)
    {
        if (ptr_location)
        {
            s_memory_callbacks.free(ptr_location);
            ptr_location = nullptr;
        }
    }
};
} // namespace alloc
} // namespace leanclr

#pragma once

#include <cstdlib>
#include <cstring>

#include "rt_base.h"

namespace leanclr
{
namespace alloc
{
class GeneralAllocation
{
  public:
    static void* malloc(size_t size)
    {
        return std::malloc(size);
    }

    template <typename T>
    static T* malloc_any()
    {
        void* ptr = malloc(sizeof(T));
        return static_cast<T*>(ptr);
    }

    static void* malloc_zeroed(size_t size)
    {
        return std::calloc(1, size);
    }

    template <typename T>
    static T* malloc_any_zeroed()
    {
        void* ptr = malloc_zeroed(sizeof(T));
        return static_cast<T*>(ptr);
    }

    static void* calloc(size_t count, size_t size)
    {
        return std::calloc(count, size);
    }

    template <typename T>
    static T* calloc_any(size_t count)
    {
        void* ptr = calloc(count, sizeof(T));
        return static_cast<T*>(ptr);
    }

    static void* realloc(void* ptr, size_t size)
    {
        return std::realloc(ptr, size);
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
            std::free(ptr);
        }
    }

    static void free_and_nullify(void*& ptr_location)
    {
        if (ptr_location)
        {
            std::free(ptr_location);
            ptr_location = nullptr;
        }
    }
};
} // namespace alloc
} // namespace leanclr

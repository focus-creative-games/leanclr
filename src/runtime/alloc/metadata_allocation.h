#pragma once

#include "general_allocation.h"
#include "mem_pool.h"

namespace leanclr
{
namespace alloc
{
class MetadataAllocation
{
  public:
    static void init()
    {
        s_memPool = new MemPool();
    }

    static void* malloc(size_t size)
    {
        return s_memPool->malloc_zeroed(size);
    }

    static void* malloc_zeroed(size_t size)
    {
        return s_memPool->malloc_zeroed(size);
    }

    template <typename T>
    static T* malloc_any()
    {
        return s_memPool->malloc_any_zeroed<T>();
    }

    template <typename T>
    static T* malloc_any_zeroed()
    {
        return s_memPool->malloc_any_zeroed<T>();
    }

    static void* calloc(size_t count, size_t size)
    {
        return s_memPool->calloc(count, size);
    }

    template <typename T>
    static T* calloc_any(size_t count)
    {
        return s_memPool->calloc_any<T>(count);
    }

  private:
    static MemPool* s_memPool;
};
} // namespace alloc
} // namespace leanclr

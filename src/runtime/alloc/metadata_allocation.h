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
        if (!s_memPool)
            s_memPool = GeneralAllocation::require_allocation(GeneralAllocation::new_any<MemPool>());
    }

    static void* malloc(size_t size)
    {
        return GeneralAllocation::require_allocation(s_memPool->malloc_zeroed(size));
    }

    static void* malloc_zeroed(size_t size)
    {
        return GeneralAllocation::require_allocation(s_memPool->malloc_zeroed(size));
    }

    template <typename T>
    static T* malloc_any()
    {
        return GeneralAllocation::require_allocation(s_memPool->malloc_any_zeroed<T>());
    }

    template <typename T>
    static T* malloc_any_zeroed()
    {
        return GeneralAllocation::require_allocation(s_memPool->malloc_any_zeroed<T>());
    }

    static void* calloc(size_t count, size_t size)
    {
        return GeneralAllocation::require_allocation(s_memPool->calloc(count, size));
    }

    template <typename T>
    static T* calloc_any(size_t count)
    {
        return GeneralAllocation::require_allocation(s_memPool->calloc_any<T>(count));
    }

  private:
    static MemPool* s_memPool;
};
} // namespace alloc
} // namespace leanclr

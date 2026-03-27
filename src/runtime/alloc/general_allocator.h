#pragma once

#include <cstddef>
#include <new>

#include "general_allocation.h"

namespace leanclr
{
namespace alloc
{

template <typename T>
class GeneralAllocator
{
  public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    template <typename U>
    struct rebind
    {
        using other = GeneralAllocator<U>;
    };

    GeneralAllocator() noexcept = default;
    GeneralAllocator(const GeneralAllocator&) noexcept = default;

    template <typename U>
    GeneralAllocator(const GeneralAllocator<U>&) noexcept
    {
    }

    ~GeneralAllocator() = default;

    pointer allocate(size_type n)
    {
        if (n == 0)
        {
            return nullptr;
        }

        void* ptr = GeneralAllocation::malloc(n * sizeof(T));
        return static_cast<pointer>(ptr);
    }

    void deallocate(pointer p, size_type n) noexcept
    {
        (void)n; // unused
        GeneralAllocation::free(p);
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args)
    {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p)
    {
        p->~U();
    }

    bool operator==(const GeneralAllocator&) const noexcept
    {
        return true;
    }

    bool operator!=(const GeneralAllocator&) const noexcept
    {
        return false;
    }
};

} // namespace alloc
} // namespace leanclr

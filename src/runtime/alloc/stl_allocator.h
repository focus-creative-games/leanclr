#pragma once
#include "general_allocator.h"

namespace leanclr
{
namespace alloc
{
// STL requires allocate to succeed or leave through its failure policy. With
// exceptions disabled, use the explicit fatal domain, never a nullable allocator.
template <typename T>
class StlAllocator : public GeneralAllocator<T>
{
  public:
    using value_type = T;

    template <typename U>
    struct rebind
    {
        using other = StlAllocator<U>;
    };

    StlAllocator() noexcept = default;

    template <typename U>
    StlAllocator(const StlAllocator<U>&) noexcept
    {
    }

    T* allocate(size_t count)
    {
        return GeneralAllocation::require_allocation(GeneralAllocator<T>::allocate(count ? count : 1));
    }

    template <typename U>
    bool operator==(const StlAllocator<U>&) const noexcept
    {
        return true;
    }

    template <typename U>
    bool operator!=(const StlAllocator<U>&) const noexcept
    {
        return false;
    }
};
} // namespace alloc
} // namespace leanclr

#pragma once
#include "general_allocation.h"
#include <cstddef>
#include <cstring>
#include <type_traits>

namespace leanclr
{
namespace alloc
{
// POD scratch storage. Small buffers stay inline; larger buffers belong to the
// configured runtime allocator. Failure preserves the previous buffer.
template <typename T, size_t InlineCount>
class ScopedBuffer
{
    static_assert(std::is_trivial<T>::value && std::is_standard_layout<T>::value, "POD scratch buffers only");
    static_assert(alignof(T) <= alignof(std::max_align_t), "Overaligned scratch buffers need an aligned allocator");
    T inline_[InlineCount == 0 ? 1 : InlineCount];
    T* heap_ = nullptr;
    size_t size_ = 0;

  public:
    ScopedBuffer() = default;
    ScopedBuffer(const ScopedBuffer&) = delete;
    ScopedBuffer& operator=(const ScopedBuffer&) = delete;
    ScopedBuffer(ScopedBuffer&&) = delete;
    ScopedBuffer& operator=(ScopedBuffer&&) = delete;

    ~ScopedBuffer()
    {
        GeneralAllocation::free(heap_);
    }

    [[nodiscard]] bool try_allocate_zeroed(size_t count)
    {
        size_t bytes;
        if (!utils::CheckedSize::multiply(count, sizeof(T), bytes))
            return false;
        T* replacement = nullptr;
        if (count > InlineCount)
        {
            replacement = GeneralAllocation::calloc_any<T>(count);
            if (!replacement)
                return false;
        }
        else if (bytes != 0)
        {
            std::memset(inline_, 0, bytes);
        }
        GeneralAllocation::free(heap_);
        heap_ = replacement;
        size_ = count;
        return true;
    }

    T* data()
    {
        return heap_ ? heap_ : inline_;
    }

    const T* data() const
    {
        return heap_ ? heap_ : inline_;
    }

    size_t size() const
    {
        return size_;
    }
};
} // namespace alloc
} // namespace leanclr

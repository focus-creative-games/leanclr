#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "general_allocation.h"
#include "utils/mem_op.h"

namespace leanclr
{
namespace alloc
{
class MemPool
{
  private:
    struct Region
    {
        std::uint8_t* data{nullptr};
        std::size_t size{0};
        std::size_t cur{0};
        Region* next{nullptr};
    };

    static constexpr std::size_t DEFAULT_PAGE_SIZE = 4 * 1024;
    static constexpr std::size_t DEFAULT_REGION_SIZE = 64 * 1024;
    static constexpr std::size_t ALIGNMENT = 8;

    Region* region_{nullptr};
    std::size_t page_size_{DEFAULT_PAGE_SIZE};
    std::size_t region_size_{DEFAULT_REGION_SIZE};

    static std::size_t align_up(std::size_t value, std::size_t alignment)
    {
        return utils::MemOp::align_up(value, alignment);
    }

    Region* create_region(std::size_t capacity)
    {
        const std::size_t aligned_capacity = std::max(align_up(capacity, page_size_), region_size_);

        auto* data = static_cast<std::uint8_t*>(alloc::GeneralAllocation::malloc_zeroed(aligned_capacity));
        if (!data)
        {
            return nullptr;
        }

        auto* reg = static_cast<Region*>(alloc::GeneralAllocation::malloc_zeroed(sizeof(Region)));
        if (!reg)
        {
            alloc::GeneralAllocation::free(data);
            return nullptr;
        }

        reg->data = data;
        reg->size = aligned_capacity;
        reg->cur = 0;
        reg->next = nullptr;
        return reg;
    }

    bool add_region(std::size_t capacity)
    {
        Region* new_region = create_region(capacity);
        if (!new_region)
        {
            return false;
        }
        new_region->next = region_;
        region_ = new_region;
        return true;
    }

  public:
    MemPool() : region_(nullptr), page_size_(DEFAULT_PAGE_SIZE), region_size_(DEFAULT_REGION_SIZE)
    {
        add_region(region_size_);
    }

    explicit MemPool(std::size_t capacity) : region_(nullptr), page_size_(DEFAULT_PAGE_SIZE), region_size_(DEFAULT_REGION_SIZE)
    {
        add_region(capacity);
    }

    MemPool(std::size_t capacity, std::size_t page_size, std::size_t region_size) : region_(nullptr), page_size_(page_size), region_size_(region_size)
    {
        add_region(capacity);
    }

    MemPool(const MemPool&) = delete;
    MemPool& operator=(const MemPool&) = delete;

    MemPool(MemPool&&) = delete;
    MemPool& operator=(MemPool&&) = delete;

    ~MemPool()
    {
        Region* reg = region_;
        while (reg)
        {
            Region* next = reg->next;
#ifndef NDEBUG
            std::memset(reg->data, 0xDD, reg->size);
#endif
            alloc::GeneralAllocation::free(reg->data);
#ifndef NDEBUG
            std::memset(reg, 0xDD, sizeof(Region));
#endif
            alloc::GeneralAllocation::free(reg);
            reg = next;
        }
    }

    std::uint8_t* malloc_zeroed(std::size_t size, std::size_t alignment = ALIGNMENT)
    {
        assert(alignment && (alignment & (alignment - 1)) == 0 && "Alignment must be a power of two");
        assert(size % alignment == 0 && "Size must be multiple of alignment");

        assert(region_ && "No region available in MemPool");

        size_t start_pos = align_up(region_->cur, alignment);

        if (start_pos + size > region_->size)
        {
            if (!add_region(size))
            {
                return nullptr;
            }
        }

        Region* reg = region_;
        start_pos = align_up(region_->cur, alignment);
        std::uint8_t* ptr = reg->data + start_pos;
        reg->cur = start_pos + size;
        return ptr;
    }

    std::uint8_t* calloc(std::size_t count, std::size_t size, size_t alignment = ALIGNMENT)
    {
        assert(alignment && (alignment & (alignment - 1)) == 0 && "Alignment must be a power of two");
        assert(size % alignment == 0 && "Size must be multiple of alignment");
        assert(count == 0 || (count <= SIZE_MAX / size) && "Size overflow in calloc");
        const std::size_t total = count * size;
        return malloc_zeroed(total, alignment);
    }

    template <typename T>
    T* malloc_any_zeroed()
    {
        return reinterpret_cast<T*>(malloc_zeroed(sizeof(T), alignof(T)));
    }

    template <typename T>
    T* calloc_any(std::size_t count)
    {
        return reinterpret_cast<T*>(calloc(count, sizeof(T), alignof(T)));
    }

    template <typename T, typename... Args>
    T* new_any(Args&&... args)
    {
        void* ptr = malloc_any_zeroed<T>();
        if (ptr)
        {
            return new (ptr) T(std::forward<Args>(args)...);
        }
        return nullptr;
    }

    template <typename T>
    void delete_any(T* ptr)
    {
        if (ptr)
        {
            ptr->~T();
        }
    }
};
} // namespace alloc
} // namespace leanclr

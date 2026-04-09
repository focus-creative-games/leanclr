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
        uint8_t* data{nullptr};
        size_t size{0};
        size_t cur{0};
        Region* next{nullptr};
    };

    static constexpr size_t DEFAULT_PAGE_SIZE = 4 * 1024;
    static constexpr size_t DEFAULT_REGION_SIZE = 64 * 1024;
    static constexpr size_t ALIGNMENT = 8;

    static size_t s_default_region_size;

    Region* region_;
    size_t page_size_;
    size_t region_size_;

    static size_t align_up(size_t value, size_t alignment)
    {
        return utils::MemOp::align_up(value, alignment);
    }

    Region* create_region(size_t capacity);
    bool add_region(size_t capacity);

  public:
    static void set_default_region_size(size_t size);
    static size_t get_default_region_size();

    MemPool() : region_(nullptr), page_size_(DEFAULT_PAGE_SIZE), region_size_(s_default_region_size)
    {
        add_region(region_size_);
    }

    explicit MemPool(size_t capacity) : region_(nullptr), page_size_(DEFAULT_PAGE_SIZE), region_size_(s_default_region_size)
    {
        add_region(capacity);
    }

    MemPool(size_t capacity, size_t page_size, size_t region_size) : region_(nullptr), page_size_(page_size), region_size_(region_size)
    {
        add_region(capacity);
    }

    MemPool(const MemPool&) = delete;
    MemPool& operator=(const MemPool&) = delete;

    MemPool(MemPool&&) = delete;
    MemPool& operator=(MemPool&&) = delete;

    ~MemPool();

    uint8_t* malloc_zeroed(size_t size, size_t alignment = ALIGNMENT);
    uint8_t* calloc(size_t count, size_t size, size_t alignment = ALIGNMENT);

    template <typename T>
    T* malloc_any_zeroed()
    {
        return reinterpret_cast<T*>(malloc_zeroed(sizeof(T), alignof(T)));
    }

    template <typename T>
    T* calloc_any(size_t count)
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

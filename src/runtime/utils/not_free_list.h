#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "alloc/mem_pool.h"

namespace leanclr
{
namespace utils
{

constexpr std::size_t NOT_FREE_LIST_DEFAULT_CAPACITY = 4;

template <typename V>
class NotFreeList
{
  public:
    NotFreeList(alloc::MemPool* pool) : data_(nullptr), size_(0), capacity_(0), pool_(pool)
    {
    }

    NotFreeList(alloc::MemPool* pool, std::size_t capacity) : size_(0), pool_(pool)
    {
        data_ = pool->calloc_any<V>(capacity);
        capacity_ = capacity;
    }

    static NotFreeList with_capacity(alloc::MemPool* pool, std::size_t capacity)
    {
        NotFreeList list(pool);
        if (capacity > 0)
        {
            list.data_ = pool ? pool->calloc_any<V>(capacity) : nullptr;
            list.capacity_ = capacity;
        }
        return list;
    }

    void resize_uninitialized(std::size_t new_size)
    {
        if (new_size > capacity_)
        {
            std::size_t new_capacity = std::max(capacity_ * 2, new_size);
            reserve(std::max(new_capacity, NOT_FREE_LIST_DEFAULT_CAPACITY));
        }
        size_ = new_size;
    }

    V get(std::size_t index) const
    {
        assert(index < size_);
        return *(data_ + index);
    }

    V* get_mut_ref(std::size_t index)
    {
        assert(index < size_);
        return data_ + index;
    }

    void reserve(std::size_t new_capacity)
    {
        if (capacity_ >= new_capacity)
        {
            return;
        }
        assert(pool_ != nullptr);
        V* new_data = pool_ ? pool_->calloc_any<V>(new_capacity) : nullptr;
        if (data_ && new_data)
        {
            std::memcpy(new_data, data_, size_ * sizeof(V));
        }
        data_ = new_data;
        capacity_ = new_capacity;
    }

    void push_back(const V& value)
    {
        if (size_ >= capacity_)
        {
            std::size_t new_capacity = capacity_ * 2;
            reserve(std::max(new_capacity, NOT_FREE_LIST_DEFAULT_CAPACITY));
        }
        assert(data_ != nullptr);
        *(data_ + size_) = value;
        size_ += 1;
    }

    void push_range(const V* values, std::size_t count)
    {
        if (count == 0)
        {
            return;
        }
        if (size_ + count > capacity_)
        {
            std::size_t new_capacity = std::max(capacity_ * 2, size_ + count);
            reserve(std::max(new_capacity, NOT_FREE_LIST_DEFAULT_CAPACITY));
        }
        assert(data_ != nullptr);
        std::memcpy(data_ + size_, values, count * sizeof(V));
        size_ += count;
    }

    template <typename Iter>
    void push_range(Iter begin, Iter end)
    {
        for (auto it = begin; it != end; ++it)
        {
            push_back(*it);
        }
    }

    void pop_unchecked()
    {
        assert(size_ > 0);
        size_ -= 1;
    }

    void clear()
    {
        if (data_)
        {
            size_ = 0;
        }
    }

    std::size_t size() const
    {
        return size_;
    }

    std::size_t capacity() const
    {
        return capacity_;
    }

    bool empty() const
    {
        return size_ == 0;
    }

    V* data() const
    {
        return data_;
    }

    V* begin() const
    {
        return data_;
    }

    V* end() const
    {
        return data_ + size_;
    }

    const V& operator[](std::size_t index) const
    {
        assert(index < size_);
        return *(data_ + index);
    }

    V& operator[](std::size_t index)
    {
        assert(index < size_);
        return *(data_ + index);
    }

  private:
    V* data_;
    std::size_t size_;
    std::size_t capacity_;
    alloc::MemPool* pool_;
};

} // namespace utils
} // namespace leanclr

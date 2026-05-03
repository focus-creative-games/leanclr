#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <utility>
#include <type_traits>

#include "alloc/general_allocator.h"

namespace leanclr
{
namespace utils
{

template <typename T, typename Allocator = alloc::GeneralAllocator<T>>
class Vector
{
    static_assert(std::is_standard_layout<T>::value && std::is_trivial<T>::value, "T must be a POD type");

  public:
    // Standard container type aliases (required for std::back_inserter)
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;

    Vector() : allocator_(Allocator()), data_(nullptr), size_(0), capacity_(0)
    {
    }

    // Vector(const T* begin, const T* end, const Allocator& alloc = Allocator())
    //     : allocator_(alloc), data_(nullptr), size_(0), capacity_(0)
    // {
    //     size_t count = static_cast<size_t>(end - begin);
    //     reserve(count);
    //     size_ = count;
    //     for (size_t i = 0; i < count; ++i)
    //     {
    //         new (&data_[i]) T(begin[i]);
    //     }
    // }

    explicit Vector(const Allocator& alloc) : allocator_(alloc), data_(nullptr), size_(0), capacity_(0)
    {
    }

    explicit Vector(size_t size, const Allocator& alloc = Allocator()) : allocator_(alloc), data_(nullptr), size_(0), capacity_(0)
    {
        resize(size);
    }

    explicit Vector(const_iterator begin, const_iterator end, const Allocator& alloc = Allocator()) : allocator_(alloc), data_(nullptr), size_(0), capacity_(0)
    {
        assert(begin <= end);
        push_range(begin, static_cast<size_t>(end - begin));
    }

    Vector(const Vector& other) : allocator_(other.allocator_), data_(nullptr), size_(0), capacity_(0)
    {
        reserve(other.size_);
        for (size_t i = 0; i < other.size_; ++i)
        {
            push_back(other.data_[i]);
        }
    }

    Vector& operator=(const Vector& other)
    {
        if (this != &other)
        {
            clear();
            reserve(other.size_);
            for (size_t i = 0; i < other.size_; ++i)
            {
                push_back(other.data_[i]);
            }
        }
        return *this;
    }

    Vector(Vector&& other) noexcept : allocator_(std::move(other.allocator_)), data_(other.data_), size_(other.size_), capacity_(other.capacity_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    Vector& operator=(Vector&& other) noexcept
    {
        if (this != &other)
        {
            clear();
            if (data_)
            {
                allocator_.deallocate(data_, capacity_);
            }
            allocator_ = std::move(other.allocator_);
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    ~Vector()
    {
        clear();
        if (data_)
        {
            allocator_.deallocate(data_, capacity_);
        }
    }

    void push_back(const T& value)
    {
        if (size_ >= capacity_)
        {
            grow();
        }
        data_[size_] = value;
        ++size_;
    }

    void push_range(const T* begin, size_t count)
    {
        reserve(size_ + count);
        std::memcpy(&data_[size_], begin, count * sizeof(T));
        size_ += count;
    }

    template <typename... Args>
    void emplace_back(Args&&... args)
    {
        if (size_ >= capacity_)
        {
            grow();
        }
        new (&data_[size_]) T(std::forward<Args>(args)...);
        ++size_;
    }

    void pop_back()
    {
        if (size_ > 0)
        {
            --size_;
            data_[size_].~T();
        }
    }

    void clear()
    {
        for (size_t i = 0; i < size_; ++i)
        {
            data_[i].~T();
        }
        size_ = 0;
    }

    void resize(size_t new_size, const T& value = T{})
    {
        if (new_size > capacity_)
        {
            reserve(new_size);
        }
        if (new_size > size_)
        {
            for (size_t i = size_; i < new_size; ++i)
            {
                new (&data_[i]) T(value);
            }
        }
        else if (new_size < size_)
        {
            for (size_t i = new_size; i < size_; ++i)
            {
                data_[i].~T();
            }
        }
        size_ = new_size;
    }

    void reserve(size_t new_capacity)
    {
        if (new_capacity <= capacity_)
        {
            return;
        }

        T* new_data = allocator_.allocate(new_capacity);
        for (size_t i = 0; i < size_; ++i)
        {
            new (&new_data[i]) T(std::move(data_[i]));
            data_[i].~T();
        }

        if (data_)
        {
            allocator_.deallocate(data_, capacity_);
        }

        data_ = new_data;
        capacity_ = new_capacity;
    }

    T& operator[](size_t index)
    {
        assert(index < size_ && "Vector index out of range");
        return data_[index];
    }

    const T& operator[](size_t index) const
    {
        assert(index < size_ && "Vector index out of range");
        return data_[index];
    }

    T& front()
    {
        return data_[0];
    }

    const T& front() const
    {
        return data_[0];
    }

    T& back()
    {
        return data_[size_ - 1];
    }

    const T& back() const
    {
        return data_[size_ - 1];
    }

    T* data()
    {
        return data_;
    }

    const T* data() const
    {
        return data_;
    }

    size_t size() const
    {
        return size_;
    }

    size_t capacity() const
    {
        return capacity_;
    }

    bool empty() const
    {
        return size_ == 0;
    }

    // Iterator support
    T* begin()
    {
        return data_;
    }

    const T* begin() const
    {
        return data_;
    }

    T* end()
    {
        return data_ + size_;
    }

    const T* end() const
    {
        return data_ + size_;
    }

  private:
    void grow()
    {
        size_t new_capacity = capacity_ == 0 ? 8 : capacity_ * 2;
        reserve(new_capacity);
    }

    Allocator allocator_;
    T* data_;
    size_t size_;
    size_t capacity_;
};

} // namespace utils
} // namespace leanclr

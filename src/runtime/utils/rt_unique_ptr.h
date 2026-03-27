#pragma once

#include "alloc/general_allocation.h"

namespace leanclr
{
namespace utils
{
template <typename T>
class UniquePtr
{
  private:
    T* ptr_;

  public:
    // Constructor
    explicit UniquePtr(T* ptr = nullptr) : ptr_(ptr)
    {
    }

    // Destructor - calls GeneralAllocation::free
    ~UniquePtr()
    {
        if (ptr_)
        {
            alloc::GeneralAllocation::free(ptr_);
        }
    }

    // Disable copy
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // Enable move
    UniquePtr(UniquePtr&& other) noexcept : ptr_(other.ptr_)
    {
        other.ptr_ = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept
    {
        if (this != &other)
        {
            if (ptr_)
            {
                alloc::GeneralAllocation::free(ptr_);
            }
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    // Access operators
    T* operator->() const
    {
        return ptr_;
    }
    T& operator*() const
    {
        return *ptr_;
    }

    // Get raw pointer
    T* get() const
    {
        return ptr_;
    }

    // Release ownership
    T* release()
    {
        T* temp = ptr_;
        ptr_ = nullptr;
        return temp;
    }

    // Reset pointer
    void reset(T* ptr = nullptr)
    {
        if (ptr_)
        {
            alloc::GeneralAllocation::free(ptr_);
        }
        ptr_ = ptr;
    }

    // Boolean conversion
    explicit operator bool() const
    {
        return ptr_ != nullptr;
    }
};

// Helper function to create UniquePtr
template <typename T, typename... Args>
UniquePtr<T> make_unique(Args&&... args)
{
    void* memory = alloc::GeneralAllocation::malloc(sizeof(T));
    if (!memory)
    {
        return UniquePtr<T>(nullptr);
    }
    T* obj = new (memory) T(std::forward<Args>(args)...);
    return UniquePtr<T>(obj);
}
} // namespace utils
} // namespace leanclr

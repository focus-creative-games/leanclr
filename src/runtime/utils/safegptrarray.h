#pragma once

#include "build_config.h"
#include "alloc/general_allocation.h"

namespace leanclr
{
namespace utils
{

template <typename T>
class SafeGPtrArray
{
  private:
    const T** _data;
    int32_t _length;

    SafeGPtrArray()
    {
        _data = nullptr;
        _length = 0;
    }

    SafeGPtrArray(const T** data, int32_t length)
    {
        if (length > 0)
        {
            const T** new_data = alloc::GeneralAllocation::calloc_any<const T*>(static_cast<size_t>(length));
            std::memcpy(new_data, data, static_cast<size_t>(length) * sizeof(const T*));
            _data = new_data;
            _length = length;
        }
        else
        {
            _data = nullptr;
            _length = 0;
        }
    }

  public:
    static SafeGPtrArray<T>* create_empty()
    {
        return new (alloc::GeneralAllocation::malloc_any<SafeGPtrArray<T>>()) SafeGPtrArray<T>();
    }

    static SafeGPtrArray<T>* create_from_data(const T** data, int32_t length)
    {
        return new (alloc::GeneralAllocation::malloc_any<SafeGPtrArray<T>>()) SafeGPtrArray<T>(data, length);
    }

    static void destroy(SafeGPtrArray<T>* array)
    {
        if (array)
        {
            array->~SafeGPtrArray<T>();
            alloc::GeneralAllocation::free(array);
        }
    }

    const T** data() const
    {
        return _data;
    }

    int32_t length() const
    {
        return _length;
    }

    ~SafeGPtrArray()
    {
        alloc::GeneralAllocation::free_and_nullify(reinterpret_cast<void*&>(_data));
        _length = 0;
    }
};
} // namespace utils
} // namespace leanclr

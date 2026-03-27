#pragma once

#include <cstddef>
#include <cstdint>

namespace leanclr
{
namespace utils
{
template <typename T>
class Span
{
  public:
    Span() : _data(nullptr), _size(0)
    {
    }

    Span(T* ptr, size_t count) : _data(ptr), _size(count)
    {
    }

    T* data() const
    {
        return _data;
    }

    size_t size() const
    {
        return _size;
    }

    bool empty() const
    {
        return _size == 0;
    }

    T& operator[](size_t idx) const
    {
        return _data[idx];
    }

    T* begin() const
    {
        return _data;
    }

    T* end() const
    {
        return _data + _size;
    }

  private:
    T* _data;
    size_t _size;
};

using ByteSpan = Span<uint8_t>;
using ConstByteSpan = Span<const uint8_t>;

} // namespace utils
} // namespace leanclr

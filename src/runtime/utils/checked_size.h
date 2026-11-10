#pragma once
#include <cstddef>
#include <cstdint>

namespace leanclr
{
namespace utils
{
// Size calculations leave the output unchanged on failure. Callers translate
// failure to their own boundary error (BadImageFormat, Overflow or OOM).
class CheckedSize
{
  public:
    static bool add(size_t a, size_t b, size_t& result)
    {
        if (b > SIZE_MAX - a)
            return false;
        result = a + b;
        return true;
    }

    static bool multiply(size_t a, size_t b, size_t& result)
    {
        if (a != 0 && b > SIZE_MAX / a)
            return false;
        result = a * b;
        return true;
    }

    static bool align_up(size_t value, size_t alignment, size_t& result)
    {
        size_t rounded;
        if (alignment == 0 || (alignment & (alignment - 1)) != 0 || !add(value, alignment - 1, rounded))
            return false;
        result = rounded & ~(alignment - 1);
        return true;
    }

    static bool contains(size_t length, size_t offset, size_t count)
    {
        return offset <= length && count <= length - offset;
    }
};
} // namespace utils
} // namespace leanclr

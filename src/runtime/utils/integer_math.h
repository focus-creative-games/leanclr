#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>

namespace leanclr
{
namespace utils
{
// CLI evaluation-stack integers wrap at their width. Perform arithmetic in
// unsigned types, then recover the signed value without an out-of-range cast.
class IntegerMath
{
  public:
    template <typename T> using Unsigned = typename std::make_unsigned<T>::type;

    template <typename T> static T from_unsigned(Unsigned<T> bits)
    {
        static_assert(sizeof(T) >= sizeof(int32_t), "evaluation-stack integer required");
        if (!std::is_signed<T>::value || bits <= static_cast<Unsigned<T>>(std::numeric_limits<T>::max()))
            return static_cast<T>(bits);
        return T(-1) - static_cast<T>(static_cast<Unsigned<T>>(~bits));
    }
    template <typename T> static T add(T a, T b)
    {
        return from_unsigned<T>(static_cast<Unsigned<T>>(a) + static_cast<Unsigned<T>>(b));
    }
    template <typename T> static T sub(T a, T b)
    {
        return from_unsigned<T>(static_cast<Unsigned<T>>(a) - static_cast<Unsigned<T>>(b));
    }
    template <typename T> static T mul(T a, T b)
    {
        return from_unsigned<T>(static_cast<Unsigned<T>>(a) * static_cast<Unsigned<T>>(b));
    }
    template <typename T> static T neg(T value)
    {
        return from_unsigned<T>(Unsigned<T>(0) - static_cast<Unsigned<T>>(value));
    }
    template <typename T> static T shl(T value, uint32_t amount)
    {
        return from_unsigned<T>(static_cast<Unsigned<T>>(value) << (amount & (sizeof(T) * 8 - 1)));
    }
    template <typename T> static T shr_un(T value, uint32_t amount)
    {
        return from_unsigned<T>(static_cast<Unsigned<T>>(value) >> (amount & (sizeof(T) * 8 - 1)));
    }
    template <typename T> static T shr(T value, uint32_t amount)
    {
        const uint32_t width = sizeof(T) * 8;
        const uint32_t shift = amount & (width - 1);
        if (shift == 0) return value;
        Unsigned<T> bits = static_cast<Unsigned<T>>(value) >> shift;
        if (std::is_signed<T>::value && value < 0)
            bits |= static_cast<Unsigned<T>>(~Unsigned<T>(0)) << (width - shift);
        return from_unsigned<T>(bits);
    }
};
} // namespace utils
} // namespace leanclr

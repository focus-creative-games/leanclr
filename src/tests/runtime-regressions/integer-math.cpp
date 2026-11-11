#include "utils/integer_math.h"
#include <cstdio>
#include <cstdlib>
#include <limits>
using leanclr::utils::IntegerMath;

static void check(bool value)
{
    if (!value)
    {
        std::fprintf(stderr, "integer arithmetic mismatch\n");
        std::exit(1);
    }
}

template <typename T>
static void verify()
{
    using U = typename std::make_unsigned<T>::type;
    const T minimum = std::numeric_limits<T>::min(), maximum = std::numeric_limits<T>::max();
    const uint32_t width = sizeof(T) * 8;
    check(IntegerMath::add<T>(maximum, 1) == minimum);
    check(IntegerMath::sub<T>(minimum, 1) == maximum);
    check(IntegerMath::mul<T>(maximum, 2) == -2);
    check(IntegerMath::mul<T>(minimum, -1) == minimum);
    check(IntegerMath::neg<T>(minimum) == minimum);
    check(IntegerMath::shl<T>(-1, width - 1) == minimum);
    check(IntegerMath::shr<T>(minimum, width - 1) == -1);
    check(IntegerMath::shr_un<T>(minimum, width - 1) == 1);
    check(IntegerMath::shl<T>(-1, width) == -1);
    check(IntegerMath::shr<T>(-1, UINT32_MAX) == -1);
    U state = 123456789;
    for (int i = 0; i < 10000; ++i)
    {
        state = state * U(1664525) + U(1013904223);
        const U a = state;
        state = state * U(1664525) + U(1013904223);
        const U b = state;
        const T x = IntegerMath::from_unsigned<T>(a), y = IntegerMath::from_unsigned<T>(b);
        check(static_cast<U>(x) == a && static_cast<U>(y) == b);
        check(static_cast<U>(IntegerMath::add<T>(x, y)) == U(a + b));
        check(static_cast<U>(IntegerMath::sub<T>(x, y)) == U(a - b));
        check(static_cast<U>(IntegerMath::mul<T>(x, y)) == U(a * b));
    }
}

template <typename T>
static void verify_unsigned()
{
    const T maximum = std::numeric_limits<T>::max();
    const uint32_t width = sizeof(T) * 8;
    check(IntegerMath::from_unsigned<T>(maximum) == maximum);
    check(IntegerMath::add<T>(maximum, 1) == 0);
    check(IntegerMath::sub<T>(0, 1) == maximum);
    check(IntegerMath::mul<T>(maximum, 2) == maximum - 1);
    check(IntegerMath::neg<T>(1) == maximum);
    check(IntegerMath::shr<T>(maximum, 0) == maximum);
    check(IntegerMath::shr<T>(maximum, width) == maximum);
    check(IntegerMath::shr<T>(maximum, width - 1) == 1);
    check(IntegerMath::shr<T>(maximum, UINT32_MAX) == 1);
    T state = 123456789;
    for (unsigned i = 0; i < 10000; ++i)
    {
        state = state * T(1664525) + T(1013904223);
        const T a = state;
        state = state * T(1664525) + T(1013904223);
        const T b = state;
        const uint32_t shift = static_cast<uint32_t>(b) & (width - 1);
        check(IntegerMath::add<T>(a, b) == T(a + b));
        check(IntegerMath::sub<T>(a, b) == T(a - b));
        check(IntegerMath::mul<T>(a, b) == T(a * b));
        check(IntegerMath::shr<T>(a, static_cast<uint32_t>(b)) == T(a >> shift));
    }
}

int main()
{
    verify<int32_t>();
    verify<int64_t>();
    verify<intptr_t>();
    verify_unsigned<uint32_t>();
    verify_unsigned<uint64_t>();
    verify_unsigned<uintptr_t>();
    std::puts("integer arithmetic passed");
}

#include "utils/integer_math.h"
#include <cstdio>
#include <cstdlib>
#include <limits>
using leanclr::utils::IntegerMath;
static void check(bool value) { if (!value) { std::fprintf(stderr, "integer arithmetic mismatch\n"); std::exit(1); } }
template <typename T> static void verify()
{
    using U = typename std::make_unsigned<T>::type;
    const T minimum = std::numeric_limits<T>::min(), maximum = std::numeric_limits<T>::max();
    const uint32_t width = sizeof(T) * 8;
    check(IntegerMath::add<T>(maximum,1) == minimum);
    check(IntegerMath::sub<T>(minimum,1) == maximum);
    check(IntegerMath::mul<T>(maximum,2) == -2);
    check(IntegerMath::mul<T>(minimum,-1) == minimum);
    check(IntegerMath::neg<T>(minimum) == minimum);
    check(IntegerMath::shl<T>(-1,width-1) == minimum);
    check(IntegerMath::shr<T>(minimum,width-1) == -1);
    check(IntegerMath::shr_un<T>(minimum,width-1) == 1);
    check(IntegerMath::shl<T>(-1,width) == -1);
    check(IntegerMath::shr<T>(-1,UINT32_MAX) == -1);
    U state = 123456789;
    for (int i = 0; i < 10000; ++i)
    {
        state = state * U(1664525) + U(1013904223);
        const U a = state;
        state = state * U(1664525) + U(1013904223);
        const U b = state;
        const T x = IntegerMath::from_unsigned<T>(a), y = IntegerMath::from_unsigned<T>(b);
        check(static_cast<U>(x) == a && static_cast<U>(y) == b);
        check(static_cast<U>(IntegerMath::add<T>(x,y)) == U(a+b));
        check(static_cast<U>(IntegerMath::sub<T>(x,y)) == U(a-b));
        check(static_cast<U>(IntegerMath::mul<T>(x,y)) == U(a*b));
    }
}
int main()
{
    verify<int32_t>(); verify<int64_t>(); verify<intptr_t>();
    std::puts("integer arithmetic passed");
}

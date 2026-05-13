#include <cstdint>
#include <cstring>

#ifndef __EMSCRIPTEN__

extern "C" int32_t leanclr_pinvoke_add_i32(int32_t a, int32_t b)
{
    return a + b;
}

extern "C" int32_t leanclr_pinvoke_mul_i32(int32_t a, int32_t b)
{
    return a * b;
}

extern "C" int32_t leanclr_pinvoke_neg_i32(int32_t x)
{
    return -x;
}

extern "C" bool leanclr_pinvoke_is_nonzero_i32(int32_t x)
{
    return x != 0;
}

extern "C" int32_t leanclr_pinvoke_utf8_byte_len(const char* s)
{
    return static_cast<int32_t>(strlen(s));
}

extern "C" const char* leanclr_pinvoke_return_null_utf8(const char* s)
{
    (void)s;
    return nullptr;
}

extern "C" int32_t leanclr_pinvoke_sum_int_range(int32_t* arr, int32_t count)
{
    int32_t sum = 0;
    for (int32_t i = 0; i < count; i++)
    {
        sum += arr[i];
    }
    return sum;
}

#endif

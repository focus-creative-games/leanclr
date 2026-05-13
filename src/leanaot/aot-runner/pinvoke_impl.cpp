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

// Layout: int32 __field_0 then __field_1 (LeanAOT instance field naming, declaration order).
struct LeanClrPinvokeTestPair_abi
{
    int32_t __field_0;
    int32_t __field_1;
};

extern "C" int32_t leanclr_pinvoke_struct_pair_mul_add(LeanClrPinvokeTestPair_abi p)
{
    return p.__field_0 * p.__field_1 + p.__field_1;
}

#if defined(_MSC_VER)
#define LEANCLR_TEST_PINVOKE_CC __cdecl
#else
#define LEANCLR_TEST_PINVOKE_CC
#endif

using LeanClrPinvokeBinaryOp_fn = int32_t(LEANCLR_TEST_PINVOKE_CC*)(int32_t, int32_t);

extern "C" int32_t leanclr_pinvoke_invoke_binary_op(void* op, int32_t a, int32_t b)
{
    return reinterpret_cast<LeanClrPinvokeBinaryOp_fn>(op)(a, b);
}

extern "C" int32_t leanclr_pinvoke_safe_handle_add_ten(void* raw_handle)
{
    return static_cast<int32_t>(reinterpret_cast<intptr_t>(raw_handle)) + 10;
}

#endif

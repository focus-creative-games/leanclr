#pragma once
#include "core/rt_base.h"

namespace leanclr
{
namespace interp
{
template <typename Src, typename Dst>
inline int32_t cast_float_to_small_int(Src value)
{
    return (int32_t)(Dst)(int32_t)(value);
}

template <typename Src, typename Dst>
inline int32_t cast_float_to_i32(Src value)
{
    if (value >= 0.0)
    {
        return (int32_t)(Dst)(value);
    }
    else
    {
        return (int32_t)(Dst)(int64_t)value;
    }
}

template <typename Src, typename Dst>
inline int64_t cast_float_to_i64(Src value)
{
    if (value >= 0.0)
    {
        return (int64_t)(uint64_t)value;
    }
    else
    {
        return (int64_t)(value);
    }
}

template <typename Src, typename Dst>
inline intptr_t cast_float_to_intptr(Src value)
{
#if LEANCLR_ARCH_64BIT
    return (intptr_t)cast_float_to_i64<Src, Dst>(value);
#else
    return (intptr_t)cast_float_to_i32<Src, Dst>(value);
#endif
}

// Compiler-agnostic overflow checking wrappers
// Use built-in functions when available (GCC/Clang), fall back to manual checks for MSVC
#if defined(__GNUC__) || defined(__clang__)
// GCC and Clang support __builtin_*_overflow functions
#define CHECK_ADD_OVERFLOW_I32(a, b, result) __builtin_add_overflow(a, b, result)
#define CHECK_ADD_OVERFLOW_I64(a, b, result) __builtin_add_overflow(a, b, result)
#define CHECK_ADD_OVERFLOW_U32(a, b, result) __builtin_add_overflow(a, b, result)
#define CHECK_ADD_OVERFLOW_U64(a, b, result) __builtin_add_overflow(a, b, result)
#define CHECK_ADD_OVERFLOW_INTPTR(a, b, result) __builtin_add_overflow(a, b, result)
#define CHECK_ADD_OVERFLOW_UINTPTR(a, b, result) __builtin_add_overflow(a, b, result)
#define CHECK_SUB_OVERFLOW_I32(a, b, result) __builtin_sub_overflow(a, b, result)
#define CHECK_SUB_OVERFLOW_I64(a, b, result) __builtin_sub_overflow(a, b, result)
#define CHECK_SUB_OVERFLOW_U32(a, b, result) __builtin_sub_overflow(a, b, result)
#define CHECK_SUB_OVERFLOW_U64(a, b, result) __builtin_sub_overflow(a, b, result)
#define CHECK_SUB_OVERFLOW_INTPTR(a, b, result) __builtin_sub_overflow(a, b, result)
#define CHECK_SUB_OVERFLOW_UINTPTR(a, b, result) __builtin_sub_overflow(a, b, result)
#define CHECK_MUL_OVERFLOW_I32(a, b, result) __builtin_mul_overflow(a, b, result)
#define CHECK_MUL_OVERFLOW_I64(a, b, result) __builtin_mul_overflow(a, b, result)
#define CHECK_MUL_OVERFLOW_U32(a, b, result) __builtin_mul_overflow(a, b, result)
#define CHECK_MUL_OVERFLOW_U64(a, b, result) __builtin_mul_overflow(a, b, result)
#define CHECK_MUL_OVERFLOW_INTPTR(a, b, result) __builtin_mul_overflow(a, b, result)
#define CHECK_MUL_OVERFLOW_UINTPTR(a, b, result) __builtin_mul_overflow(a, b, result)
#else
// Fallback for MSVC and other compilers: manual overflow checking
inline bool check_add_overflow_i32(int32_t a, int32_t b, int32_t* result)
{
    int64_t res = static_cast<int64_t>(a) + static_cast<int64_t>(b);
    if (res > INT32_MAX || res < INT32_MIN)
        return true;
    *result = static_cast<int32_t>(res);
    return false;
}
inline bool check_add_overflow_i64(int64_t a, int64_t b, int64_t* result)
{
    if ((b > 0 && a > INT64_MAX - b) || (b < 0 && a < INT64_MIN - b))
        return true;
    *result = a + b;
    return false;
}
inline bool check_add_overflow_u32(uint32_t a, uint32_t b, uint32_t* result)
{
    if (a > UINT32_MAX - b)
        return true;
    *result = a + b;
    return false;
}
inline bool check_add_overflow_u64(uint64_t a, uint64_t b, uint64_t* result)
{
    if (a > UINT64_MAX - b)
        return true;
    *result = a + b;
    return false;
}

inline bool check_add_overflow_intptr(intptr_t a, intptr_t b, intptr_t* result)
{
#if LEANCLR_ARCH_64BIT
    return check_add_overflow_i64(a, b, (int64_t*)result);
#else
    return check_add_overflow_i32(a, b, (int32_t*)result);
#endif
}

inline bool check_add_overflow_uintptr(uintptr_t a, uintptr_t b, uintptr_t* result)
{
#if LEANCLR_ARCH_64BIT
    return check_add_overflow_u64(a, b, (uint64_t*)result);
#else
    return check_add_overflow_u32(a, b, (uint32_t*)result);
#endif
}

inline bool check_sub_overflow_i32(int32_t a, int32_t b, int32_t* result)
{
    int64_t res = static_cast<int64_t>(a) - static_cast<int64_t>(b);
    if (res > INT32_MAX || res < INT32_MIN)
        return true;
    *result = static_cast<int32_t>(res);
    return false;
}
inline bool check_sub_overflow_i64(int64_t a, int64_t b, int64_t* result)
{
    if ((b > 0 && a < INT64_MIN + b) || (b < 0 && a > INT64_MAX + b))
        return true;
    *result = a - b;
    return false;
}
inline bool check_sub_overflow_u32(uint32_t a, uint32_t b, uint32_t* result)
{
    if (a < b)
        return true;
    *result = a - b;
    return false;
}
inline bool check_sub_overflow_u64(uint64_t a, uint64_t b, uint64_t* result)
{
    if (a < b)
        return true;
    *result = a - b;
    return false;
}

inline bool check_sub_overflow_intptr(intptr_t a, intptr_t b, intptr_t* result)
{
#if LEANCLR_ARCH_64BIT
    return check_sub_overflow_i64(a, b, (int64_t*)result);
#else
    return check_sub_overflow_i32(a, b, (int32_t*)result);
#endif
}

inline bool check_sub_overflow_uintptr(uintptr_t a, uintptr_t b, uintptr_t* result)
{
#if LEANCLR_ARCH_64BIT
    return check_sub_overflow_u64(a, b, (uint64_t*)result);
#else
    return check_sub_overflow_u32(a, b, (uint32_t*)result);
#endif
}

inline bool check_mul_overflow_i32(int32_t a, int32_t b, int32_t* result)
{
    if (b != 0 && (a > INT32_MAX / b || a < INT32_MIN / b))
        return true;
    if (a == INT32_MIN && b == -1) // Special case: overflow
        return true;
    *result = a * b;
    return false;
}
inline bool check_mul_overflow_i64(int64_t a, int64_t b, int64_t* result)
{
    if (b != 0 && (a > INT64_MAX / b || a < INT64_MIN / b))
        return true;
    if (a == INT64_MIN && b == -1) // Special case: overflow
        return true;
    *result = a * b;
    return false;
}
inline bool check_mul_overflow_u32(uint32_t a, uint32_t b, uint32_t* result)
{
    if (b != 0 && a > UINT32_MAX / b)
        return true;
    *result = a * b;
    return false;
}
inline bool check_mul_overflow_u64(uint64_t a, uint64_t b, uint64_t* result)
{
    if (b != 0 && a > UINT64_MAX / b)
        return true;
    *result = a * b;
    return false;
}

inline bool check_mul_overflow_intptr(intptr_t a, intptr_t b, intptr_t* result)
{
#if LEANCLR_ARCH_64BIT
    return check_mul_overflow_i64(a, b, (int64_t*)result);
#else
    return check_mul_overflow_i32(a, b, (int32_t*)result);
#endif
}

inline bool check_mul_overflow_uintptr(uintptr_t a, uintptr_t b, uintptr_t* result)
{
#if LEANCLR_ARCH_64BIT
    return check_mul_overflow_u64(a, b, (uint64_t*)result);
#else
    return check_mul_overflow_u32(a, b, (uint32_t*)result);
#endif
}

#define CHECK_ADD_OVERFLOW_I32(a, b, result) leanclr::interp::check_add_overflow_i32(a, b, result)
#define CHECK_ADD_OVERFLOW_I64(a, b, result) leanclr::interp::check_add_overflow_i64(a, b, result)
#define CHECK_ADD_OVERFLOW_U32(a, b, result) leanclr::interp::check_add_overflow_u32(a, b, result)
#define CHECK_ADD_OVERFLOW_U64(a, b, result) leanclr::interp::check_add_overflow_u64(a, b, result)
#define CHECK_ADD_OVERFLOW_INTPTR(a, b, result) leanclr::interp::check_add_overflow_intptr(a, b, result)
#define CHECK_ADD_OVERFLOW_UINTPTR(a, b, result) leanclr::interp::check_add_overflow_uintptr(a, b, result)
#define CHECK_SUB_OVERFLOW_I32(a, b, result) leanclr::interp::check_sub_overflow_i32(a, b, result)
#define CHECK_SUB_OVERFLOW_I64(a, b, result) leanclr::interp::check_sub_overflow_i64(a, b, result)
#define CHECK_SUB_OVERFLOW_U32(a, b, result) leanclr::interp::check_sub_overflow_u32(a, b, result)
#define CHECK_SUB_OVERFLOW_U64(a, b, result) leanclr::interp::check_sub_overflow_u64(a, b, result)
#define CHECK_SUB_OVERFLOW_INTPTR(a, b, result) leanclr::interp::check_sub_overflow_intptr(a, b, result)
#define CHECK_SUB_OVERFLOW_UINTPTR(a, b, result) leanclr::interp::check_sub_overflow_uintptr(a, b, result)
#define CHECK_MUL_OVERFLOW_I32(a, b, result) leanclr::interp::check_mul_overflow_i32(a, b, result)
#define CHECK_MUL_OVERFLOW_I64(a, b, result) leanclr::interp::check_mul_overflow_i64(a, b, result)
#define CHECK_MUL_OVERFLOW_U32(a, b, result) leanclr::interp::check_mul_overflow_u32(a, b, result)
#define CHECK_MUL_OVERFLOW_U64(a, b, result) leanclr::interp::check_mul_overflow_u64(a, b, result)
#define CHECK_MUL_OVERFLOW_INTPTR(a, b, result) leanclr::interp::check_mul_overflow_intptr(a, b, result)
#define CHECK_MUL_OVERFLOW_UINTPTR(a, b, result) leanclr::interp::check_mul_overflow_uintptr(a, b, result)
#endif

} // namespace interp
} // namespace leanclr
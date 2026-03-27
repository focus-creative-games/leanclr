#pragma once

#include <stdint.h>
#include <cassert>
#include <cstring>

#include "alloc/general_allocation.h"

namespace leanclr
{
namespace utils
{
class MemOp
{
  public:
    static size_t align_up(size_t value, size_t alignment)
    {
        assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of two");
        return (value + alignment - 1) & ~(alignment - 1);
    }

    template <typename T>
    static void copy_obj_nonoverlapping(T* dst, const T* src)
    {
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
        *dst = *src;
#else
        std::memcpy(dst, src, sizeof(T));
#endif
    }

    template <typename T>
    static void copy_obj_mayoverlapping(T* dst, const T* src)
    {
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
        *dst = *src;
#else
        std::memmove(dst, src, sizeof(T));
#endif
    }

    template <typename T>
    static void copy_obj_array_nonoverlapping(T* dst, const T* src, size_t count)
    {
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
        for (size_t i = 0; i < count; i++)
        {
            dst[i] = src[i];
        }
#else
        std::memcpy(dst, src, sizeof(T) * count);
#endif
    }

    template <typename T>
    static void copy_obj_array_mayoverlapping(T* dst, const T* src, size_t count)
    {
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
        for (size_t i = 0; i < count; i++)
        {
            dst[i] = src[i];
        }
#else
        std::memmove(dst, src, sizeof(T) * count);
#endif
    }

    static int16_t read_i16_may_unaligned(const void* ptr)
    {
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
        return *reinterpret_cast<const int16_t*>(ptr);
#else
        int16_t val;
        std::memcpy(&val, ptr, sizeof(int16_t));
        return val;
#endif
    }

    static uint16_t read_u16_may_unaligned(const void* ptr)
    {
        return (uint16_t)read_i16_may_unaligned(ptr);
    }

    static int32_t read_i32_may_unaligned(const void* ptr)
    {
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
        return *reinterpret_cast<const int32_t*>(ptr);
#else
        int32_t val;
        std::memcpy(&val, ptr, sizeof(int32_t));
        return val;
#endif
    }

    static uint32_t read_u32_may_unaligned(const void* ptr)
    {
        return (uint32_t)read_i32_may_unaligned(ptr);
    }

    static int64_t read_i64_may_unaligned(const void* ptr)
    {
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
        return *reinterpret_cast<const int64_t*>(ptr);
#else
        int64_t val;
        std::memcpy(&val, ptr, sizeof(int64_t));
        return val;
#endif
    }

    static uint64_t read_u64_may_unaligned(const void* ptr)
    {
        return (uint64_t)read_i64_may_unaligned(ptr);
    }

    static float read_f32_may_unaligned(const void* ptr)
    {
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
        return *reinterpret_cast<const float*>(ptr);
#else
        float val;
        std::memcpy(&val, ptr, sizeof(float));
        return val;
#endif
    }

    static double read_f64_may_unaligned(const void* ptr)
    {
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
        return *reinterpret_cast<const double*>(ptr);
#else
        double val;
        std::memcpy(&val, ptr, sizeof(double));
        return val;
#endif
    }

    static void write_i16_may_unaligned(void* ptr, int16_t value)
    {
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
        *reinterpret_cast<int16_t*>(ptr) = value;
#else
        std::memcpy(ptr, &value, sizeof(int16_t));
#endif
    }

    static void write_u16_may_unaligned(void* ptr, uint16_t value)
    {
        write_i16_may_unaligned(ptr, static_cast<int16_t>(value));
    }

    static void write_i32_may_unaligned(void* ptr, int32_t value)
    {
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
        *reinterpret_cast<int32_t*>(ptr) = value;
#else
        std::memcpy(ptr, &value, sizeof(int32_t));
#endif
    }

    static void write_u32_may_unaligned(void* ptr, uint32_t value)
    {
        write_i32_may_unaligned(ptr, static_cast<int32_t>(value));
    }

    static void write_i64_may_unaligned(void* ptr, int64_t value)
    {
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
        *reinterpret_cast<int64_t*>(ptr) = value;
#else
        std::memcpy(ptr, &value, sizeof(int64_t));
#endif
    }

    static void write_u64_may_unaligned(void* ptr, uint64_t value)
    {
        write_i64_may_unaligned(ptr, static_cast<int64_t>(value));
    }

    static uint32_t get_not_zero_bit_count(uint64_t bits)
    {
        uint32_t count = 0;
        while (bits)
        {
            if (bits & 1)
                count++;
            bits >>= 1;
        }
        return count;
    }

    static void* dup_mem(const void* src, size_t size)
    {
        void* dst = alloc::GeneralAllocation::malloc(size);
        if (dst)
        {
            std::memcpy(dst, src, size);
        }
        return dst;
    }
};
} // namespace utils
} // namespace leanclr

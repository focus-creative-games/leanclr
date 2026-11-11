#pragma once

#include <stdint.h>
#include <cassert>
#include <cstring>
#include <type_traits>

#include "build_config.h"
#include "alloc/general_allocation.h"

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace leanclr
{
namespace utils
{
class MemOp
{
  public:
    // Managed fields and IL operands need not have native alignment. Never
    // dereference a cast pointer, even on hardware that accepts unaligned loads.
    template <typename T>
    static T load_native(const T* ptr)
    {
        T value;
        std::memcpy(&value, static_cast<const void*>(ptr), sizeof(value));
        return value;
    }

    template <typename T>
    static void store_native(T* ptr, typename std::remove_cv<T>::type value)
    {
        std::memcpy(static_cast<void*>(ptr), &value, sizeof(value));
    }

    static size_t align_up(size_t value, size_t alignment)
    {
        assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of two");
        return (value + alignment - 1) & ~(alignment - 1);
    }

    static size_t align_down(size_t value, size_t alignment)
    {
        assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of two");
        return value & ~(alignment - 1);
    }

    template <typename T>
    static void copy_obj_nonoverlapping(T* dst, const T* src)
    {
        std::memcpy(static_cast<void*>(dst), static_cast<const void*>(src), sizeof(T));
    }

    template <typename T>
    static void copy_obj_mayoverlapping(T* dst, const T* src)
    {
        std::memmove(static_cast<void*>(dst), static_cast<const void*>(src), sizeof(T));
    }

    template <typename T>
    static void copy_obj_array_nonoverlapping(T* dst, const T* src, size_t count)
    {
        std::memcpy(static_cast<void*>(dst), static_cast<const void*>(src), sizeof(T) * count);
    }

    template <typename T>
    static void copy_obj_array_mayoverlapping(T* dst, const T* src, size_t count)
    {
        std::memmove(static_cast<void*>(dst), static_cast<const void*>(src), sizeof(T) * count);
    }

    static int16_t read_i16_may_unaligned(const void* ptr)
    {
        int16_t val;
        std::memcpy(&val, ptr, sizeof(int16_t));
        return val;
    }

    static uint16_t read_u16_may_unaligned(const void* ptr)
    {
        return (uint16_t)read_i16_may_unaligned(ptr);
    }

    static int32_t read_i32_may_unaligned(const void* ptr)
    {
        int32_t val;
        std::memcpy(&val, ptr, sizeof(int32_t));
        return val;
    }

    static uint32_t read_u32_may_unaligned(const void* ptr)
    {
        return (uint32_t)read_i32_may_unaligned(ptr);
    }

    static int64_t read_i64_may_unaligned(const void* ptr)
    {
        int64_t val;
        std::memcpy(&val, ptr, sizeof(int64_t));
        return val;
    }

    static uint64_t read_u64_may_unaligned(const void* ptr)
    {
        return (uint64_t)read_i64_may_unaligned(ptr);
    }

    static float read_f32_may_unaligned(const void* ptr)
    {
        float val;
        std::memcpy(&val, ptr, sizeof(float));
        return val;
    }

    static double read_f64_may_unaligned(const void* ptr)
    {
        double val;
        std::memcpy(&val, ptr, sizeof(double));
        return val;
    }

    static void write_i16_may_unaligned(void* ptr, int16_t value)
    {
        std::memcpy(ptr, &value, sizeof(int16_t));
    }

    static void write_u16_may_unaligned(void* ptr, uint16_t value)
    {
        write_i16_may_unaligned(ptr, static_cast<int16_t>(value));
    }

    static void write_i32_may_unaligned(void* ptr, int32_t value)
    {
        std::memcpy(ptr, &value, sizeof(int32_t));
    }

    static void write_u32_may_unaligned(void* ptr, uint32_t value)
    {
        write_i32_may_unaligned(ptr, static_cast<int32_t>(value));
    }

    static void write_i64_may_unaligned(void* ptr, int64_t value)
    {
        std::memcpy(ptr, &value, sizeof(int64_t));
    }

    static void write_u64_may_unaligned(void* ptr, uint64_t value)
    {
        write_i64_may_unaligned(ptr, static_cast<int64_t>(value));
    }

    static void* read_ptr_may_unaligned(void* ptr)
    {
        void* val;
        std::memcpy(&val, ptr, sizeof(void*));
        return val;
    }

    static void write_ptr_may_unaligned(void* ptr, void* value)
    {
        std::memcpy(ptr, &value, sizeof(void*));
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

    // word must be non-zero
    static size_t count_trailing_zeros_nonzero(size_t word)
    {
        assert(word != 0);
#if defined(__GNUC__) || defined(__clang__)
#if LEANCLR_ARCH_64BIT
        return static_cast<size_t>(__builtin_ctzll(static_cast<unsigned long long>(word)));
#else
        return static_cast<size_t>(__builtin_ctzl(word));
#endif
#elif defined(_MSC_VER)
        unsigned long index;
#if LEANCLR_ARCH_64BIT
        _BitScanForward64(&index, word);
#else
        _BitScanForward(&index, static_cast<unsigned long>(word));
#endif
        return static_cast<size_t>(index);
#else
        size_t n = 0;
        while ((word & static_cast<size_t>(1)) == 0)
        {
            word >>= 1;
            ++n;
        }
        return n;
#endif
    }

    // word must be non-zero
    static size_t count_trailing_zeros_nonzero64(uint64_t word)
    {
        assert(word != 0);
#if defined(__GNUC__) || defined(__clang__)
        return static_cast<size_t>(__builtin_ctzll(word));
#elif defined(_MSC_VER)
        unsigned long index;
#if LEANCLR_ARCH_64BIT
        _BitScanForward64(&index, word);
        return static_cast<size_t>(index);
#else
        const unsigned long low = static_cast<unsigned long>(word);
        if (low != 0)
        {
            _BitScanForward(&index, low);
            return static_cast<size_t>(index);
        }
        _BitScanForward(&index, static_cast<unsigned long>(word >> 32));
        return static_cast<size_t>(index) + 32;
#endif
#else
        size_t n = 0;
        while ((word & static_cast<uint64_t>(1)) == 0)
        {
            word >>= 1;
            ++n;
        }
        return n;
#endif
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

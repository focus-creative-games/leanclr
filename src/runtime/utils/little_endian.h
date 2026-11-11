#pragma once

#include <cstdint>
#include <cstring>

namespace leanclr
{
namespace utils
{
// Serialized PE/CLI/IL bytes have little-endian order, independently of native
// object representation. The input may start at any byte address.
class LittleEndian
{
  public:
    static uint16_t read_u16(const void* ptr)
    {
        const auto* p = static_cast<const uint8_t*>(ptr);
        return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
    }

    static uint32_t read_u32(const void* ptr)
    {
        const auto* p = static_cast<const uint8_t*>(ptr);
        return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
    }

    static uint64_t read_u64(const void* ptr)
    {
        const auto* p = static_cast<const uint8_t*>(ptr);
        return uint64_t(read_u32(p)) | (uint64_t(read_u32(p + 4)) << 32);
    }

    static int16_t read_i16(const void* ptr)
    {
        const uint16_t bits = read_u16(ptr);
        int16_t value;
        static_assert(sizeof(value) == sizeof(bits), "Unexpected scalar size");
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }

    static int32_t read_i32(const void* ptr)
    {
        const uint32_t bits = read_u32(ptr);
        int32_t value;
        static_assert(sizeof(value) == sizeof(bits), "Unexpected scalar size");
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }

    static int64_t read_i64(const void* ptr)
    {
        const uint64_t bits = read_u64(ptr);
        int64_t value;
        static_assert(sizeof(value) == sizeof(bits), "Unexpected scalar size");
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }

    static float read_f32(const void* ptr)
    {
        const uint32_t bits = read_u32(ptr);
        float value;
        static_assert(sizeof(value) == sizeof(bits), "Unexpected scalar size");
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }

    static double read_f64(const void* ptr)
    {
        const uint64_t bits = read_u64(ptr);
        double value;
        static_assert(sizeof(value) == sizeof(bits), "Unexpected scalar size");
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }
};
} // namespace utils
} // namespace leanclr

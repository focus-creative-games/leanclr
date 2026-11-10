#pragma once

#include "mem_op.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace leanclr
{
namespace utils
{
class BinaryReader
{
  private:
    const uint8_t* _data;
    size_t _index;
    size_t _length;

  public:
    BinaryReader(const void* data, size_t length) : _data(static_cast<const uint8_t*>(data)), _index(0), _length(length)
    {
    }

    const uint8_t* data() const
    {
        return _data;
    }

    size_t length() const
    {
        return _length;
    }

    size_t get_position() const
    {
        return _index;
    }

    bool try_set_position(size_t new_index)
    {
        if (new_index <= _length)
        {
            _index = new_index;
            return true;
        }
        return false;
    }

    bool try_advance(size_t offset)
    {
        if (offset > _length - _index)
        {
            return false;
        }

        _index = _index + offset;
        return true;
    }

    bool try_offset(ptrdiff_t offset)
    {
        if (offset < 0)
        {
            const size_t distance = static_cast<size_t>(-(offset + 1)) + 1;
            if (distance > _index) return false;
            _index -= distance;
            return true;
        }
        return try_advance(static_cast<size_t>(offset));
    }

    bool try_align_up_position(size_t alignment)
    {
        if (alignment == 0 || (alignment & (alignment - 1)) != 0 || _index > SIZE_MAX - (alignment - 1))
            return false;
        size_t aligned_index = MemOp::align_up(_index, alignment);
        if (aligned_index > _length)
        {
            return false;
        }
        _index = aligned_index;
        return true;
    }

    size_t remaining() const { return _length - _index; }

    bool try_peek_cstring(size_t max_length, const char*& out_value, size_t& out_length) const
    {
        const size_t limit = std::min(remaining(), max_length == SIZE_MAX ? max_length : max_length + 1);
        if (limit == 0) return false;
        const void* end = std::memchr(get_current_ptr(), 0, limit);
        if (!end) return false;
        out_value = reinterpret_cast<const char*>(get_current_ptr());
        out_length = static_cast<const uint8_t*>(end) - get_current_ptr();
        return true;
    }

    bool not_empty() const
    {
        return _index < _length;
    }

    bool is_remaining(size_t count) const
    {
        return count <= _length - _index;
    }

    bool is_aligned(size_t alignment) const
    {
        return _index % alignment == 0;
    }

    const uint8_t* get_current_ptr() const
    {
        return _data + _index;
    }

    template <typename T>
    bool try_peek_any_ptr(const T*& out_ptr) const
    {
        if (sizeof(T) <= _length - _index)
        {
            out_ptr = reinterpret_cast<const T*>(_data + _index);
            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename T>
    bool try_peek_any_ptr_range(size_t count, const T*& out_ptr) const
    {
        if (count <= (_length - _index) / sizeof(T))
        {
            out_ptr = reinterpret_cast<const T*>(_data + _index);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool try_peek_byte(uint8_t& out_byte) const
    {
        if (_index < _length)
        {
            out_byte = _data[_index];
            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename T>
    bool try_read_any(T& out_value)
    {
        if (sizeof(T) <= _length - _index)
        {
            MemOp::copy_obj_nonoverlapping(&out_value, reinterpret_cast<const T*>(_data + _index));
            _index += sizeof(T);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool try_read_byte(uint8_t& out_byte)
    {
        if (_index < _length)
        {
            out_byte = _data[_index];
            _index++;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool try_read_u16(uint16_t& out_value)
    {
        if (2 <= _length - _index)
        {
            MemOp::copy_obj_nonoverlapping(&out_value, reinterpret_cast<const uint16_t*>(_data + _index));
            _index += 2;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool try_read_u32(uint32_t& out_value)
    {
        if (4 <= _length - _index)
        {
            MemOp::copy_obj_nonoverlapping(&out_value, reinterpret_cast<const uint32_t*>(_data + _index));
            _index += 4;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool try_read_u64(uint64_t& out_value)
    {
        if (8 <= _length - _index)
        {
            MemOp::copy_obj_nonoverlapping(&out_value, reinterpret_cast<const uint64_t*>(_data + _index));
            _index += 8;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool try_read_f32(float& out_value)
    {
        if (4 <= _length - _index)
        {
            MemOp::copy_obj_nonoverlapping(&out_value, reinterpret_cast<const float*>(_data + _index));
            _index += 4;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool try_read_f64(double& out_value)
    {
        if (8 <= _length - _index)
        {
            MemOp::copy_obj_nonoverlapping(&out_value, reinterpret_cast<const double*>(_data + _index));
            _index += 8;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool try_read_bytes(void* out_buffer, size_t count)
    {
        if (count <= _length - _index)
        {
            MemOp::copy_obj_array_nonoverlapping(reinterpret_cast<uint8_t*>(out_buffer), _data + _index, count);
            _index += count;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool try_read_compressed_uint32(uint32_t& out_value)
    {
        size_t remaining = _length - _index;
        size_t length_size = 0;
        if (!try_decode_compressed_uint32(_data + _index, remaining, out_value, length_size))
        {
            return false;
        }
        _index += length_size;
        return true;
    }

    static bool try_decode_compressed_uint32(const uint8_t* data, size_t remaining, uint32_t& out_value, size_t& out_used)
    {
        if (remaining == 0)
            return false;
        uint8_t b0 = data[0];
        if ((b0 & 0x80) == 0)
        {
            out_value = b0;
            out_used = 1;
            return true;
        }
        if ((b0 & 0xC0) == 0x80)
        {
            if (remaining < 2)
                return false;
            out_value = (static_cast<uint32_t>(b0 & 0x3F) << 8) | data[1];
            out_used = 2;
            return true;
        }
        if ((b0 & 0xE0) == 0xC0)
        {
            if (remaining < 4)
                return false;
            out_value = (static_cast<uint32_t>(b0 & 0x1F) << 24) | (static_cast<uint32_t>(data[1]) << 16) | (static_cast<uint32_t>(data[2]) << 8) |
                        static_cast<uint32_t>(data[3]);
            out_used = 4;
            return true;
        }
        return false;
    }

    bool try_read_compressed_int32(int32_t& out_value)
    {
        const size_t begin = _index;
        uint32_t bits;
        if (!try_read_compressed_uint32(bits)) return false;
        const size_t width = _index - begin;
        const int32_t value = static_cast<int32_t>(bits >> 1);
        // The sign bit was rotated into bit zero. Its original position is
        // determined by the encoding width, not by the decoded magnitude.
        const int32_t sign_range = width == 1 ? 0x40 : (width == 2 ? 0x2000 : 0x10000000);
        out_value = (bits & 1) != 0 ? value - sign_range : value;
        return true;
    }
};
} // namespace utils
} // namespace leanclr

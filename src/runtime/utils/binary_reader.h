#pragma once

#include "mem_op.h"

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
        if (new_index < _length)
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
        if (offset < 0 && static_cast<size_t>(-offset) > _index)
        {
            return false;
        }
        if (offset > 0 && static_cast<size_t>(offset) > _length - _index)
        {
            return false;
        }

        _index = static_cast<size_t>(static_cast<ptrdiff_t>(_index) + offset);
        return true;
    }

    bool try_align_up_position(size_t alignment)
    {
        size_t aligned_index = MemOp::align_up(_index, alignment);
        if (aligned_index > _length)
        {
            return false;
        }
        _index = aligned_index;
        return true;
    }

    bool not_empty() const
    {
        return _index < _length;
    }

    bool is_remaining(size_t count) const
    {
        return _index + count <= _length;
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
        if (_index + sizeof(T) <= _length)
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
        if (_index + sizeof(T) * count <= _length)
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
        if (_index + sizeof(T) <= _length)
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
        if (_index + 2 <= _length)
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
        if (_index + 4 <= _length)
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
        if (_index + 8 <= _length)
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
        if (_index + 4 <= _length)
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
        if (_index + 8 <= _length)
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
        if (_index + count <= _length)
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
        uint32_t unsignedValue;
        if (!try_read_compressed_uint32(unsignedValue))
        {
            return false;
        }
        uint32_t value = unsignedValue >> 1;
        if (!(unsignedValue & 0x1))
        {
            out_value = static_cast<int32_t>(value);
            return true;
        }
        if (value < 0x40)
        {
            out_value = static_cast<int32_t>(value - 0x40);
            return true;
        }
        if (value < 0x2000)
        {
            out_value = static_cast<int32_t>(value - 0x2000);
            return true;
        }
        if (value < 0x10000000)
        {
            out_value = static_cast<int32_t>(value - 0x10000000);
            return true;
        }
        assert(value < 0x20000000);
        out_value = static_cast<int32_t>(value - 0x20000000);
        return true;
    }
};
} // namespace utils
} // namespace leanclr

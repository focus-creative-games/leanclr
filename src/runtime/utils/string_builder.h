#pragma once

#include <cstring>

#include "core/rt_base.h"
#include "alloc/general_allocation.h"
#include "string_util.h"

namespace leanclr
{
namespace utils
{

class StringBuilder
{
  private:
    char* _buf;
    size_t _length;
    size_t _capacity;

    constexpr static size_t INITIAL_BUFFER_SIZE = 32;
    char _initial_buffer[INITIAL_BUFFER_SIZE];

  public:
    // Default constructor with initial capacity of 32
    StringBuilder() : _buf(_initial_buffer), _length(0), _capacity(INITIAL_BUFFER_SIZE)
    {
    }

    // Constructor with specific capacity
    explicit StringBuilder(size_t capacity_) : StringBuilder()
    {
        with_capacity_internal(capacity_);
    }

    // Destructor
    ~StringBuilder()
    {
        if (_buf && _buf != _initial_buffer)
        {
            alloc::GeneralAllocation::free(_buf);
            _buf = nullptr;
        }
    }

    // Delete copy constructor and copy assignment
    StringBuilder(const StringBuilder&) = delete;
    StringBuilder& operator=(const StringBuilder&) = delete;

    // Move constructor
    StringBuilder(StringBuilder&& other) noexcept : _buf(other._buf), _length(other._length), _capacity(other._capacity)
    {
        other._buf = nullptr;
        other._length = 0;
        other._capacity = 0;
    }

    // Reserve additional space
    void reserve(size_t additional)
    {
        if (_length + additional > _capacity)
        {
            size_t new_capacity = std::max(_capacity * 2, _length + additional);
            char* new_buf = static_cast<char*>(alloc::GeneralAllocation::malloc(new_capacity));

            if (_buf && _length > 0)
            {
                std::memcpy(new_buf, _buf, _length);
            }
            if (_buf && _buf != _initial_buffer)
            {
                alloc::GeneralAllocation::free(_buf);
            }
            _buf = new_buf;
            _capacity = new_capacity;
        }
    }

    // Append single character
    StringBuilder& append_char(uint8_t c)
    {
        reserve(1);
        _buf[_length] = static_cast<char>(c);
        _length++;
        return *this;
    }

    StringBuilder& append_chars(char c, size_t count)
    {
        reserve(count);
        for (size_t i = 0; i < count; i++)
        {
            _buf[_length + i] = c;
        }
        _length += count;
        return *this;
    }

    // Append C-string (null-terminated)
    StringBuilder& append_cstr(const char* s)
    {
        size_t str_len = std::strlen(s);
        reserve(str_len);
        if (_buf)
        {
            std::memcpy(_buf + _length, s, str_len);
            _length += str_len;
        }
        return *this;
    }

    // Append byte slice
    StringBuilder& append_cstr(const uint8_t* data, size_t len)
    {
        if (len > 0)
        {
            reserve(len);
            std::memcpy(_buf + _length, data, len);
            _length += len;
        }
        return *this;
    }

    StringBuilder& append_utf16_str(const Utf16Char* utf16_str, size_t utf16_len)
    {
        // Convert UTF-16 to UTF-8 and append
        StringUtil::utf16_to_utf8(utf16_str, utf16_len, *this);
        return *this;
    }

    // Append uint16 as decimal string
    StringBuilder& append_u16(uint16_t value)
    {
        return append_u32(static_cast<uint32_t>(value));
    }

    // Append uint32 as decimal string
    StringBuilder& append_u32(uint32_t value)
    {
        // Count digits
        size_t digit_count = 0;
        uint32_t tmp_value = value;
        do
        {
            digit_count++;
            tmp_value /= 10;
        } while (tmp_value > 0);

        reserve(digit_count);
        if (_buf)
        {
            size_t write_pos = _length + digit_count;
            tmp_value = value;
            do
            {
                write_pos--;
                _buf[write_pos] = static_cast<char>('0' + static_cast<int>(tmp_value % 10));
                tmp_value /= 10;
            } while (tmp_value > 0);
            _length += digit_count;
        }
        return *this;
    }

    // Append uint8 as hexadecimal string (uppercase)
    StringBuilder& append_hex(uint8_t value)
    {
        reserve(2);
        if (_buf)
        {
            uint8_t high = (value >> 4) & 0x0F;
            uint8_t low = value & 0x0F;
            _buf[_length] = hex_to_uppercase_char(high);
            _buf[_length + 1] = hex_to_uppercase_char(low);
            _length += 2;
        }
        return *this;
    }

    const char* get_data() const
    {
        return _buf;
    }

    char* get_mut_data() const
    {
        return _buf;
    }

    char* get_current_write_ptr() const
    {
        return _buf + _length;
    }

    // Get current buffer as const pointer
    const char* as_cstr() const
    {
        return _buf;
    }

    // Ensure null terminator without appending to length
    void sure_null_terminator_but_not_append()
    {
        reserve(1);
        _buf[_length] = 0;
    }

    // Duplicate to zero-ended C string
    char* dup_to_zero_end_cstr() const
    {
        // Allocate space for content + null terminator
        char* result = static_cast<char*>(alloc::GeneralAllocation::malloc(_length + 1));

        if (_length > 0)
        {
            std::memcpy(result, _buf, _length);
        }
        result[_length] = 0;

        return result;
    }

    // Get current length
    size_t length() const
    {
        return _length;
    }

    // Get current capacity
    size_t get_capacity() const
    {
        return _capacity;
    }

    // Clear the buffer
    void clear()
    {
        _length = 0;
    }

    void resize(size_t new_length)
    {
        if (new_length > _capacity)
        {
            reserve(new_length - _length);
        }
        _length = new_length;
    }

  private:
    // Helper: Initialize with specific capacity
    void with_capacity_internal(size_t cap)
    {
        if (cap > 0)
        {
            _buf = static_cast<char*>(alloc::GeneralAllocation::malloc(cap));

            _capacity = cap;
            _length = 0;
        }
    }

    // Helper: Convert hex digit (0-15) to uppercase character
    static char hex_to_uppercase_char(uint8_t digit)
    {
        if (digit < 10)
        {
            return static_cast<char>('0' + digit);
        }
        else
        {
            return static_cast<char>('A' + (digit - 10));
        }
    }
};

} // namespace utils
} // namespace leanclr

#pragma once

#include <algorithm>
#include <cstring>

#include "core/rt_base.h"
#include "alloc/general_allocation.h"

namespace leanclr
{
namespace utils
{

class ByteStringBuilder
{
  protected:
    char* _buf;
    size_t _length;
    size_t _capacity;

  private:
    constexpr static size_t INITIAL_BUFFER_SIZE = 32;
    char _initial_buffer[INITIAL_BUFFER_SIZE];

  public:
    ByteStringBuilder() : _buf(_initial_buffer), _length(0), _capacity(INITIAL_BUFFER_SIZE)
    {
    }

    explicit ByteStringBuilder(size_t capacity_) : ByteStringBuilder()
    {
        with_capacity_internal(capacity_);
    }

    ~ByteStringBuilder()
    {
        if (_buf && _buf != _initial_buffer)
        {
            alloc::GeneralAllocation::free(_buf);
            _buf = nullptr;
        }
    }

    ByteStringBuilder(const ByteStringBuilder&) = delete;
    ByteStringBuilder& operator=(const ByteStringBuilder&) = delete;

    ByteStringBuilder(ByteStringBuilder&& other) noexcept : _buf(other._buf), _length(other._length), _capacity(other._capacity)
    {
        other._buf = nullptr;
        other._length = 0;
        other._capacity = 0;
    }

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

    ByteStringBuilder& append_char(uint8_t c)
    {
        reserve(1);
        _buf[_length] = static_cast<char>(c);
        _length++;
        return *this;
    }

    ByteStringBuilder& append_chars(char c, size_t count)
    {
        reserve(count);
        for (size_t i = 0; i < count; i++)
        {
            _buf[_length + i] = c;
        }
        _length += count;
        return *this;
    }

    ByteStringBuilder& append_cstr(const char* s)
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

    ByteStringBuilder& append_cstr(const uint8_t* data, size_t len)
    {
        if (len > 0)
        {
            reserve(len);
            std::memcpy(_buf + _length, data, len);
            _length += len;
        }
        return *this;
    }

    ByteStringBuilder& append_u16(uint16_t value)
    {
        return append_u32(static_cast<uint32_t>(value));
    }

    ByteStringBuilder& append_u32(uint32_t value)
    {
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

    ByteStringBuilder& append_hex(uint8_t value)
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

    const char* as_cstr() const
    {
        return _buf;
    }

    void sure_null_terminator_but_not_append()
    {
        reserve(1);
        _buf[_length] = 0;
    }

    char* dup_to_zero_end_cstr() const
    {
        char* result = static_cast<char*>(alloc::GeneralAllocation::malloc(_length + 1));
        if (_length > 0)
        {
            std::memcpy(result, _buf, _length);
        }
        result[_length] = 0;
        return result;
    }

    size_t length() const
    {
        return _length;
    }

    size_t get_capacity() const
    {
        return _capacity;
    }

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
    void with_capacity_internal(size_t cap)
    {
        if (cap > 0)
        {
            _buf = static_cast<char*>(alloc::GeneralAllocation::malloc(cap));
            _capacity = cap;
            _length = 0;
        }
    }

    static char hex_to_uppercase_char(uint8_t digit)
    {
        return digit < 10 ? static_cast<char>('0' + digit) : static_cast<char>('A' + (digit - 10));
    }
};

class Utf8StringBuilder : public ByteStringBuilder
{
  public:
    using ByteStringBuilder::ByteStringBuilder;

    Utf8StringBuilder& append_char(uint8_t c)
    {
        ByteStringBuilder::append_char(c);
        return *this;
    }

    Utf8StringBuilder& append_chars(char c, size_t count)
    {
        ByteStringBuilder::append_chars(c, count);
        return *this;
    }

    Utf8StringBuilder& append_cstr(const char* s)
    {
        ByteStringBuilder::append_cstr(s);
        return *this;
    }

    Utf8StringBuilder& append_cstr(const uint8_t* data, size_t len)
    {
        ByteStringBuilder::append_cstr(data, len);
        return *this;
    }

    Utf8StringBuilder& append_u16(uint16_t value)
    {
        ByteStringBuilder::append_u16(value);
        return *this;
    }

    Utf8StringBuilder& append_u32(uint32_t value)
    {
        ByteStringBuilder::append_u32(value);
        return *this;
    }

    Utf8StringBuilder& append_hex(uint8_t value)
    {
        ByteStringBuilder::append_hex(value);
        return *this;
    }

    Utf8StringBuilder& append_utf16_str(const Utf16Char* utf16_str, size_t utf16_len);

    void sure_utf8_null_terminator_but_not_append()
    {
        sure_null_terminator_but_not_append();
    }

    const char* as_utf8_cstr() const
    {
        return as_cstr();
    }
};

class AnsiStringBuilder : public ByteStringBuilder
{
  public:
    using ByteStringBuilder::ByteStringBuilder;

    AnsiStringBuilder& append_utf16_str(const Utf16Char* utf16_str, size_t utf16_len);

    AnsiChar* as_ansi_chars() const
    {
        return reinterpret_cast<AnsiChar*>(_buf);
    }

    size_t get_ansi_chars_length() const
    {
        return _length;
    }

    void sure_ansi_null_terminator_but_not_append()
    {
        sure_null_terminator_but_not_append();
    }

    AnsiChar* dup_to_zero_end_ansi_chars() const;
};

class Utf16StringBuilder : public ByteStringBuilder
{
  public:
    using ByteStringBuilder::ByteStringBuilder;

    Utf16StringBuilder& append_utf8_str(const char* utf8_str, size_t utf8_len);
    Utf16StringBuilder& append_utf8_str(const char* utf8_str);
    Utf16StringBuilder& append_ansi_str(const AnsiChar* ansi_str, size_t ansi_len);
    Utf16StringBuilder& append_ansi_str(const AnsiChar* ansi_str);

    Utf16Char* as_utf16chars() const
    {
        return reinterpret_cast<Utf16Char*>(_buf);
    }

    size_t get_utf16chars_length() const
    {
        return _length / sizeof(Utf16Char);
    }

    void sure_utf16_null_terminator_but_not_append()
    {
        reserve(sizeof(Utf16Char));
        assert(_length % sizeof(Utf16Char) == 0);
        _buf[_length] = 0;
        _buf[_length + 1] = 0;
    }

    Utf16Char* dup_to_zero_end_utf16chars() const;
};

} // namespace utils
} // namespace leanclr

#pragma once

#include <algorithm>
#include <cstring>

#include "core/rt_base.h"
#include "alloc/general_allocation.h"
#include "string_util.h"

namespace leanclr
{
namespace utils
{

template <typename T>
class StringBuilderAny
{
  protected:
    T* _buf;
    size_t _length;
    size_t _capacity;

  private:
    constexpr static size_t INITIAL_BUFFER_SIZE = 32;
    T _initial_buffer[INITIAL_BUFFER_SIZE];

  public:
    StringBuilderAny() : _buf(_initial_buffer), _length(0), _capacity(INITIAL_BUFFER_SIZE)
    {
    }

    explicit StringBuilderAny(size_t capacity_) : StringBuilderAny()
    {
        with_capacity_internal(capacity_);
    }

    ~StringBuilderAny()
    {
        if (_buf && _buf != _initial_buffer)
        {
            alloc::GeneralAllocation::free(_buf);
            _buf = nullptr;
        }
    }

    StringBuilderAny(const StringBuilderAny&) = delete;
    StringBuilderAny& operator=(const StringBuilderAny&) = delete;

    StringBuilderAny(StringBuilderAny&& other) noexcept : _length(other._length), _capacity(other._capacity)
    {
        if (other._buf != _initial_buffer)
        {
            _buf = other._buf;
            other._buf = other._initial_buffer;
        }
        else
        {
            std::memcpy(_initial_buffer, other._initial_buffer, other._length * sizeof(T));
            _buf = _initial_buffer;
            assert(other._capacity == INITIAL_BUFFER_SIZE);
        }
        other._length = 0;
        other._capacity = 0;
    }

    void reserve(size_t additional)
    {
        if (_length + additional > _capacity)
        {
            size_t new_capacity = std::max(_capacity * 2, _length + additional);
            T* new_buf = static_cast<T*>(alloc::GeneralAllocation::malloc(new_capacity * sizeof(T)));
            if (_length > 0)
            {
                std::memcpy(new_buf, _buf, _length * sizeof(T));
            }
            if (_buf != _initial_buffer)
            {
                alloc::GeneralAllocation::free(_buf);
            }
            _buf = new_buf;
            _capacity = new_capacity;
        }
    }

    const T* get_const_chars() const
    {
        return _buf;
    }

    T* get_mut_chars() const
    {
        return _buf;
    }

    T* get_current_write_ptr() const
    {
        return _buf + _length;
    }

    void sure_null_terminator_but_not_append()
    {
        reserve(1);
        _buf[_length] = {};
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

    T* dup_zero_terminated_chars() const
    {
        T* result = static_cast<T*>(alloc::GeneralAllocation::malloc((_length + 1) * sizeof(T)));
        if (_length > 0)
        {
            std::memcpy(result, _buf, _length * sizeof(T));
        }
        result[_length] = {};
        return result;
    }

  private:
    void with_capacity_internal(size_t cap)
    {
        if (cap > 0)
        {
            _buf = static_cast<T*>(alloc::GeneralAllocation::malloc(cap * sizeof(T)));
            _capacity = cap;
            _length = 0;
        }
    }
};

class Utf8StringBuilder : public StringBuilderAny<Utf8Char>
{
  public:
    Utf8StringBuilder() : StringBuilderAny<Utf8Char>()
    {
    }

    Utf8StringBuilder(size_t capacity) : StringBuilderAny<Utf8Char>(capacity)
    {
    }

    Utf8StringBuilder(const Utf16Char* utf16_str, size_t utf16_len) : StringBuilderAny<Utf8Char>()
    {
        append_utf16_str(utf16_str, utf16_len);
    }

    Utf8StringBuilder(const Utf16Char* utf16_str) : StringBuilderAny<Utf8Char>()
    {
        append_utf16_str(utf16_str, static_cast<size_t>(utils::StringUtil::get_utf16chars_length(utf16_str)));
    }

    Utf8StringBuilder& append_char(uint8_t c)
    {
        reserve(1);
        _buf[_length] = static_cast<char>(c);
        _length++;
        return *this;
    }

    Utf8StringBuilder& append_char(char c)
    {
        return append_char(static_cast<uint8_t>(c));
    }

    Utf8StringBuilder& append_chars(char c, size_t count)
    {
        reserve(count);
        for (size_t i = 0; i < count; i++)
        {
            _buf[_length + i] = c;
        }
        _length += count;
        return *this;
    }

    Utf8StringBuilder& append_cstr(const char* s)
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

    Utf8StringBuilder& append_cstr(const char* data, size_t len)
    {
        if (len > 0)
        {
            reserve(len);
            std::memcpy(_buf + _length, data, len);
            _length += len;
        }
        return *this;
    }

    Utf8StringBuilder& append_u16(uint16_t value)
    {
        return append_u32(static_cast<uint32_t>(value));
    }

    Utf8StringBuilder& append_u32(uint32_t value)
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

    Utf8StringBuilder& append_hex(uint8_t value)
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

    Utf8StringBuilder& append_utf16_str(const Utf16Char* utf16_str, size_t utf16_len);

  private:
    static char hex_to_uppercase_char(uint8_t digit)
    {
        return digit < 10 ? static_cast<char>('0' + digit) : static_cast<char>('A' + (digit - 10));
    }
};

class AnsiStringBuilder : public StringBuilderAny<AnsiChar>
{
  public:
    AnsiStringBuilder() : StringBuilderAny<AnsiChar>()
    {
    }

    AnsiStringBuilder(const Utf16Char* utf16_str, size_t utf16_len) : StringBuilderAny()
    {
        append_utf16_str(utf16_str, utf16_len);
    }

    AnsiStringBuilder(const Utf16Char* utf16_str) : StringBuilderAny()
    {
        append_utf16_str(utf16_str, static_cast<size_t>(utils::StringUtil::get_utf16chars_length(utf16_str)));
    }

    AnsiStringBuilder& append_utf16_str(const Utf16Char* utf16_str, size_t utf16_len);
};

class Utf16StringBuilder : public StringBuilderAny<Utf16Char>
{
  public:
    Utf16StringBuilder() : StringBuilderAny<Utf16Char>()
    {
    }

    Utf16StringBuilder& append_utf8_str(const char* utf8_str, size_t utf8_len);
    Utf16StringBuilder& append_utf8_str(const char* utf8_str);
    Utf16StringBuilder& append_ansi_str(const AnsiChar* ansi_str, size_t ansi_len);
    Utf16StringBuilder& append_ansi_str(const AnsiChar* ansi_str);
};

} // namespace utils
} // namespace leanclr

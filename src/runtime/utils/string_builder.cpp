#include "string_builder.h"

#include "encode_conv.h"

#include <cstring>

namespace leanclr
{
namespace utils
{

Utf8StringBuilder& Utf8StringBuilder::append_utf16_str(const Utf16Char* utf16_str, size_t utf16_len)
{
    if (utf16_str == nullptr || utf16_len == 0)
    {
        sure_utf8_null_terminator_but_not_append();
        return *this;
    }

    size_t utf8_len = 0;
    reserve(EncodeConv::get_preserved_utf16_to_utf8_length(utf16_str, utf16_len));
    EncodeConv::utf16_to_utf8(utf16_str, utf16_len, get_current_write_ptr(), utf8_len);
    _length += utf8_len;
    assert(_length <= _capacity);
    sure_utf8_null_terminator_but_not_append();
    return *this;
}

AnsiStringBuilder& AnsiStringBuilder::append_utf16_str(const Utf16Char* utf16_str, size_t utf16_len)
{
    if (utf16_str == nullptr || utf16_len == 0)
    {
        sure_ansi_null_terminator_but_not_append();
        return *this;
    }

    size_t ansi_len = 0;
    reserve(EncodeConv::get_preserved_utf16_to_ansi_length(utf16_str, utf16_len));
    EncodeConv::utf16_to_ansi(utf16_str, utf16_len, as_ansi_chars(), ansi_len);
    _length += ansi_len;
    assert(_length <= _capacity);
    sure_ansi_null_terminator_but_not_append();
    return *this;
}

AnsiChar* AnsiStringBuilder::dup_to_zero_end_ansi_chars() const
{
    char* bytes = static_cast<char*>(alloc::GeneralAllocation::malloc(_length + 1));
    if (_length > 0)
    {
        std::memcpy(bytes, _buf, _length);
    }
    bytes[_length] = 0;
    return reinterpret_cast<AnsiChar*>(bytes);
}

Utf16StringBuilder& Utf16StringBuilder::append_utf8_str(const char* utf8_str, size_t utf8_len)
{
    if (utf8_str == nullptr || utf8_len == 0)
    {
        sure_utf16_null_terminator_but_not_append();
        return *this;
    }

    size_t utf16_len = 0;
    reserve(EncodeConv::get_preserved_utf8_to_utf16_length(utf8_str, utf8_len));
    EncodeConv::utf8_to_utf16(utf8_str, utf8_len, reinterpret_cast<Utf16Char*>(get_current_write_ptr()), utf16_len);
    _length += utf16_len * sizeof(Utf16Char);
    assert(_length <= _capacity);
    sure_utf16_null_terminator_but_not_append();
    return *this;
}

Utf16StringBuilder& Utf16StringBuilder::append_utf8_str(const char* utf8_str)
{
    return append_utf8_str(utf8_str, std::strlen(utf8_str));
}

Utf16StringBuilder& Utf16StringBuilder::append_ansi_str(const AnsiChar* ansi_str, size_t ansi_len)
{
    if (ansi_str == nullptr || ansi_len == 0)
    {
        sure_utf16_null_terminator_but_not_append();
        return *this;
    }

    size_t utf16_len = 0;
    reserve(EncodeConv::get_preserved_ansi_to_utf16_length(ansi_str, ansi_len));
    EncodeConv::ansi_to_utf16(ansi_str, ansi_len, reinterpret_cast<Utf16Char*>(get_current_write_ptr()), utf16_len);
    _length += utf16_len * sizeof(Utf16Char);
    assert(_length <= _capacity);
    sure_utf16_null_terminator_but_not_append();
    return *this;
}

Utf16StringBuilder& Utf16StringBuilder::append_ansi_str(const AnsiChar* ansi_str)
{
    size_t ansi_len = std::strlen(ansi_str);
    return append_ansi_str(ansi_str, ansi_len);
}

Utf16Char* Utf16StringBuilder::dup_to_zero_end_utf16chars() const
{
    assert(_length % sizeof(Utf16Char) == 0);
    Utf16Char* result = static_cast<Utf16Char*>(alloc::GeneralAllocation::calloc(_length / sizeof(Utf16Char) + 1, sizeof(Utf16Char)));
    if (_length > 0)
    {
        std::memcpy(result, _buf, _length);
    }
    return result;
}

} // namespace utils
} // namespace leanclr

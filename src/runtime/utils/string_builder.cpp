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
        sure_null_terminator_but_not_append();
        return *this;
    }

    size_t utf8_len = 0;
    reserve(EncodeConv::get_preserved_utf16_to_utf8_length(utf16_str, utf16_len));
    EncodeConv::utf16_to_utf8(utf16_str, utf16_len, get_current_write_ptr(), utf8_len);
    _length += utf8_len;
    assert(_length <= _capacity);
    sure_null_terminator_but_not_append();
    return *this;
}

AnsiStringBuilder& AnsiStringBuilder::append_utf16_str(const Utf16Char* utf16_str, size_t utf16_len)
{
    if (utf16_str == nullptr || utf16_len == 0)
    {
        sure_null_terminator_but_not_append();
        return *this;
    }

    size_t ansi_len = EncodeConv::get_preserved_utf16_to_ansi_length(utf16_str, utf16_len);
    reserve(ansi_len);
    EncodeConv::utf16_to_ansi(utf16_str, utf16_len, get_current_write_ptr(), ansi_len);
    _length += ansi_len;
    assert(_length <= _capacity);
    return *this;
}

Utf16StringBuilder& Utf16StringBuilder::append_utf8_str(const char* utf8_str, size_t utf8_len)
{
    if (utf8_str == nullptr || utf8_len == 0)
    {
        sure_null_terminator_but_not_append();
        return *this;
    }

    size_t utf16_len = EncodeConv::get_preserved_utf8_to_utf16_length(utf8_str, utf8_len);
    reserve(utf16_len);
    EncodeConv::utf8_to_utf16(utf8_str, utf8_len, get_current_write_ptr(), utf16_len);
    _length += utf16_len;
    assert(_length <= _capacity);
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
        sure_null_terminator_but_not_append();
        return *this;
    }

    size_t utf16_len = EncodeConv::get_preserved_ansi_to_utf16_length(ansi_str, ansi_len);
    reserve(utf16_len);
    EncodeConv::ansi_to_utf16(ansi_str, ansi_len, get_current_write_ptr(), utf16_len);
    _length += utf16_len;
    assert(_length <= _capacity);
    return *this;
}

Utf16StringBuilder& Utf16StringBuilder::append_ansi_str(const AnsiChar* ansi_str)
{
    size_t ansi_len = std::strlen(ansi_str);
    return append_ansi_str(ansi_str, ansi_len);
}

} // namespace utils
} // namespace leanclr

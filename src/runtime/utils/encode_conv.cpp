#include "encode_conv.h"

#include "3rd/utf8/utf8.h"
#include "string_builder.h"

#include <climits>

namespace leanclr
{
namespace utils
{

void EncodeConv::utf16_to_utf8(const Utf16Char* utf16_str, size_t utf16_len, Utf8StringBuilder& out_utf8_str)
{
    out_utf8_str.append_utf16_str(utf16_str, utf16_len);
}

void EncodeConv::utf16_to_utf8(const Utf16Char* utf16_str, size_t utf16_len, Utf8Char* out_utf8_str, size_t& out_utf8_len)
{
    if (!utf16_str || utf16_len == 0)
    {
        out_utf8_len = 0;
        out_utf8_str[0] = '\0';
        return;
    }

    char* end = utf8::unchecked::utf16to8(utf16_str, utf16_str + utf16_len, out_utf8_str);
    *end = '\0';
    out_utf8_len = static_cast<size_t>(end - out_utf8_str);
}

void EncodeConv::utf8_to_utf16(const Utf8Char* utf8_str, size_t utf8_len, Utf16Char* out_utf16_str, size_t& out_utf16_len)
{
    if (!utf8_str || utf8_len == 0)
    {
        out_utf16_len = 0;
        out_utf16_str[0] = 0;
        return;
    }

    const auto* u8 = reinterpret_cast<const unsigned char*>(utf8_str);
    const auto* u8end = u8 + utf8_len;
    Utf16Char* end16 = utf8::unchecked::utf8to16(u8, u8end, out_utf16_str);
    *end16 = 0;
    out_utf16_len = static_cast<size_t>(end16 - out_utf16_str);
}

} // namespace utils
} // namespace leanclr

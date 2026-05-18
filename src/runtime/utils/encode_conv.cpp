#include "encode_conv.h"

#include "3rd/utf8/utf8.h"
#include "string_builder.h"

#if LEANCLR_PLATFORM_WIN
#include <windows.h>
#endif

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

void EncodeConv::utf16_to_ansi(const Utf16Char* utf16_str, size_t utf16_len, AnsiChar* out_ansi_str, size_t& out_ansi_len)
{
    if (!utf16_str || utf16_len == 0)
    {
        out_ansi_len = 0;
        out_ansi_str[0] = '\0';
        return;
    }

#if LEANCLR_PLATFORM_WIN
    const int cch = static_cast<int>(utf16_len);
    const LPCWCH wch = reinterpret_cast<LPCWCH>(utf16_str);
    char* const out_bytes = out_ansi_str;
    const int cb = WideCharToMultiByte(CP_ACP, 0, wch, cch, out_bytes, INT_MAX, nullptr, nullptr);
    if (cb == 0)
    {
        out_ansi_len = 0;
        out_bytes[0] = '\0';
        return;
    }
    out_bytes[cb] = '\0';
    out_ansi_len = static_cast<size_t>(cb);
#else
    utf16_to_utf8(utf16_str, utf16_len, reinterpret_cast<Utf8Char*>(out_ansi_str), out_ansi_len);
#endif
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

void EncodeConv::ansi_to_utf16(const AnsiChar* ansi_str, size_t ansi_len, Utf16Char* out_utf16_str, size_t& out_utf16_len)
{
    if (!ansi_str || ansi_len == 0)
    {
        out_utf16_len = 0;
        out_utf16_str[0] = 0;
        return;
    }

#if LEANCLR_PLATFORM_WIN
    const char* const ansi_bytes = ansi_str;
    const int cb = static_cast<int>(ansi_len);
    const int cch = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, ansi_bytes, cb, reinterpret_cast<wchar_t*>(out_utf16_str), INT_MAX);
    if (cch == 0)
    {
        out_utf16_len = 0;
        out_utf16_str[0] = 0;
        return;
    }
    out_utf16_str[cch] = 0;
    out_utf16_len = static_cast<size_t>(cch);
#else
    utf8_to_utf16(reinterpret_cast<const char*>(ansi_str), ansi_len, out_utf16_str, out_utf16_len);
#endif
}

} // namespace utils
} // namespace leanclr

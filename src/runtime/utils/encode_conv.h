#pragma once

#include <cstddef>

#include "core/rt_base.h"

namespace leanclr
{
namespace utils
{

class Utf8StringBuilder;

class EncodeConv
{
  public:
    static void utf16_to_utf8(const Utf16Char* utf16_str, size_t utf16_len, Utf8StringBuilder& out_utf8_str);

    static size_t get_preserved_utf16_to_utf8_length(const Utf16Char* utf16_str, size_t utf16_len)
    {
        return utf16_len * 4 + 1;
    }

    static size_t get_preserved_utf16_to_ansi_length(const Utf16Char* utf16_str, size_t utf16_len)
    {
#if LEANCLR_PLATFORM_WIN
        return utf16_len * 2 + 1;
#else
        return utf16_len * 4 + 1;
#endif
    }

    static size_t get_preserved_utf8_to_utf16_length(const char* utf8_str, size_t utf8_len)
    {
        return (utf8_len + 1) * sizeof(Utf16Char);
    }

    static size_t get_preserved_ansi_to_utf16_length(const AnsiChar* ansi_str, size_t ansi_len)
    {
        return (ansi_len + 1) * sizeof(Utf16Char);
    }

    static void utf16_to_utf8(const Utf16Char* utf16_str, size_t utf16_len, char* out_utf8_str, size_t& out_utf8_len);
    static void utf16_to_ansi(const Utf16Char* utf16_str, size_t utf16_len, AnsiChar* out_ansi_str, size_t& out_ansi_len);
    static void utf8_to_utf16(const char* utf8_str, size_t utf8_len, Utf16Char* out_utf16_str, size_t& out_utf16_len);
    static void ansi_to_utf16(const AnsiChar* ansi_str, size_t ansi_len, Utf16Char* out_utf16_str, size_t& out_utf16_len);
};

} // namespace utils
} // namespace leanclr

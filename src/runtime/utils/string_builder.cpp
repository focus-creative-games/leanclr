#include "string_builder.h"

#include "3rd/utf8/utf8.h"
#include "string_util.h"

#if LEANCLR_PLATFORM_WIN
#include <windows.h>
#endif

#include <climits>
#include <cstring>

namespace leanclr
{
namespace utils
{

namespace
{
int clamp_size_t_to_int(size_t n)
{
    return n > static_cast<size_t>(INT_MAX) ? INT_MAX : static_cast<int>(n);
}
} // namespace

StringBuilder& StringBuilder::append_ansi_to_utf16_str(const NativeChar* ansi_str, size_t ansi_len)
{
    if (ansi_str == nullptr || ansi_len == 0)
    {
        sure_utf16_null_terminator_but_not_append();
        return *this;
    }
#if LEANCLR_PLATFORM_WIN
    // NativeChar* is passed from interop; ANSI (CP_ACP) payload is interpreted as char octets.
    const char* const src = reinterpret_cast<const char*>(ansi_str);
    const int cb = clamp_size_t_to_int(ansi_len);
    const DWORD flags = MB_PRECOMPOSED;
    const int cch = MultiByteToWideChar(CP_ACP, flags, src, cb, nullptr, 0);
    if (cch == 0)
    {
        sure_utf16_null_terminator_but_not_append();
        return *this;
    }
    const size_t old_len = length();
    reserve(static_cast<size_t>(cch) * sizeof(Utf16Char));
    Utf16Char* const dst = reinterpret_cast<Utf16Char*>(get_current_write_ptr());
    if (MultiByteToWideChar(CP_ACP, flags, src, cb, reinterpret_cast<wchar_t*>(dst), cch) == 0)
    {
        sure_utf16_null_terminator_but_not_append();
        return *this;
    }
    resize(old_len + static_cast<size_t>(cch) * sizeof(Utf16Char));
#else
    const size_t old_len = length();
    const auto* u8 = reinterpret_cast<const unsigned char*>(ansi_str);
    const auto* u8end = u8 + ansi_len;
    reserve(ansi_len * 2 + sizeof(Utf16Char));
    Utf16Char* const dst = reinterpret_cast<Utf16Char*>(get_current_write_ptr());
    Utf16Char* const end16 = utf8::unchecked::utf8to16(u8, u8end, dst);
    const size_t utf16_units = static_cast<size_t>(end16 - dst);
    resize(old_len + utf16_units * sizeof(Utf16Char));
#endif
    sure_utf16_null_terminator_but_not_append();
    return *this;
}

StringBuilder& StringBuilder::append_utf16_to_ansi_str(const Utf16Char* utf16_str, size_t utf16_len)
{
    if (utf16_str == nullptr || utf16_len == 0)
    {
        sure_ansi_null_terminator_but_not_append();
        return *this;
    }
#if LEANCLR_PLATFORM_WIN
    const int cch = clamp_size_t_to_int(utf16_len);
    const LPCWCH wch = reinterpret_cast<LPCWCH>(utf16_str);
    const int cb = WideCharToMultiByte(CP_ACP, 0, wch, cch, nullptr, 0, nullptr, nullptr);
    if (cb == 0)
    {
        sure_ansi_null_terminator_but_not_append();
        return *this;
    }
    const size_t old_len = length();
    reserve(static_cast<size_t>(cb));
    char* const dst = get_current_write_ptr();
    if (WideCharToMultiByte(CP_ACP, 0, wch, cch, dst, cb, nullptr, nullptr) == 0)
    {
        sure_ansi_null_terminator_but_not_append();
        return *this;
    }
    resize(old_len + static_cast<size_t>(cb));
#else
    StringUtil::utf16_to_utf8(utf16_str, utf16_len, *this);
#endif
    sure_ansi_null_terminator_but_not_append();
    return *this;
}

} // namespace utils
} // namespace leanclr

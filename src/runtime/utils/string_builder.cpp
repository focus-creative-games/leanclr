#include "string_builder.h"
#include "string_util.h"
#if LEANCLR_PLATFORM_WIN
#include <windows.h>
#endif

namespace leanclr
{
namespace utils
{

StringBuilder& StringBuilder::append_ansi_to_utf16_str(const NativeChar* ansi_str, size_t ansi_len)
{
    if (ansi_str == nullptr || ansi_len == 0)
    {
        return *this;
    }
    assert(false && "not implemented");
    fatal_on_not_implemented_error();
    return *this;
}

StringBuilder& StringBuilder::append_utf16_to_ansi_str(const Utf16Char* utf16_str, size_t utf16_len)
{
    if (utf16_str == nullptr || utf16_len == 0)
    {
        return *this;
    }
    assert(false && "not implemented");
    fatal_on_not_implemented_error();
    return *this;
}

} // namespace utils
} // namespace leanclr

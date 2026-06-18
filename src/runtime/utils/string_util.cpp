#include "string_util.h"

#include "alloc/general_allocation.h"
#include "build_config.h"

#if LEANCLR_PLATFORM_POSIX
#include <strings.h>
#endif

namespace leanclr
{
namespace utils
{

namespace
{

bool ascii_equals_ignorecase(char a, char b)
{
    if (a >= 'A' && a <= 'Z')
    {
        a = static_cast<char>(a + ('a' - 'A'));
    }
    if (b >= 'A' && b <= 'Z')
    {
        b = static_cast<char>(b + ('a' - 'A'));
    }
    return a == b;
}

bool portable_equals_ignorecase(const char* s1, const char* s2)
{
    while (*s1 && *s2)
    {
        if (!ascii_equals_ignorecase(*s1, *s2))
        {
            return false;
        }
        ++s1;
        ++s2;
    }
    return *s1 == *s2;
}

bool portable_equals_ignorecase_n(const char* s1, const char* s2, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        if (!ascii_equals_ignorecase(s1[i], s2[i]))
        {
            return false;
        }
    }
    return true;
}

} // namespace

const char* StringUtil::strdup(const char* str)
{
    if (!str)
    {
        return nullptr;
    }
    size_t len = std::strlen(str);
    char* copy = (char*)alloc::GeneralAllocation::malloc(len + 1);
    std::memcpy(copy, str, len + 1);
    return copy;
}

const Utf16Char* StringUtil::strdup_utf16_with_null_terminator(const Utf16Char* str, size_t length)
{
    if (!str)
    {
        return nullptr;
    }
    Utf16Char* copy = (Utf16Char*)alloc::GeneralAllocation::malloc((length + 1) * sizeof(Utf16Char));
    std::memcpy(copy, str, length * sizeof(Utf16Char));
    copy[length] = 0;
    return copy;
}

const Utf16Char* StringUtil::strdup_utf16_without_null_terminator(const Utf16Char* str, size_t length)
{
    if (!str)
    {
        return nullptr;
    }
    Utf16Char* copy = (Utf16Char*)alloc::GeneralAllocation::malloc(length * sizeof(Utf16Char));
    std::memcpy(copy, str, length * sizeof(Utf16Char));
    return copy;
}

const char* StringUtil::concat(const char* str1, const char* str2)
{
    if (!str1 && !str2)
    {
        return nullptr;
    }
    if (!str1)
    {
        return strdup(str2);
    }
    if (!str2)
    {
        return strdup(str1);
    }
    size_t len1 = std::strlen(str1);
    size_t len2 = std::strlen(str2);
    char* result = (char*)alloc::GeneralAllocation::malloc(len1 + len2 + 1);
    std::memcpy(result, str1, len1);
    std::memcpy(result + len1, str2, len2 + 1);
    return result;
}

bool StringUtil::equals_ignorecase(const char* s1, const char* s2)
{
    if (!s1 && !s2)
        return true;
    if (!s1 || !s2)
        return false;

#if LEANCLR_PLATFORM_WIN
    return _stricmp(s1, s2) == 0;
#elif LEANCLR_PLATFORM_POSIX
    return strcasecmp(s1, s2) == 0;
#else
    return portable_equals_ignorecase(s1, s2);
#endif
}

bool StringUtil::equals_ignorecase_n(const char* s1, const char* s2, size_t len)
{
    if (!s1 && !s2)
        return true;
    if (!s1 || !s2)
        return false;
#if LEANCLR_PLATFORM_WIN
    return _strnicmp(s1, s2, len) == 0;
#elif LEANCLR_PLATFORM_POSIX
    return strncasecmp(s1, s2, len) == 0;
#else
    return portable_equals_ignorecase_n(s1, s2, len);
#endif
}
} // namespace utils
} // namespace leanclr

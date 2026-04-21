#include "string_util.h"

#include "alloc/general_allocation.h"
#include "3rd/utf8/utf8.h"
#include "string_builder.h"

namespace leanclr
{
namespace utils
{
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

#ifdef _WIN32
    return _stricmp(s1, s2) == 0;
#else
    return strcasecmp(s1, s2) == 0;
#endif
}

bool StringUtil::equals_ignorecase_n(const char* s1, const char* s2, size_t len)
{
    if (!s1 && !s2)
        return true;
    if (!s1 || !s2)
        return false;
#ifdef _WIN32
    return _strnicmp(s1, s2, len) == 0;
#else
    return strncasecmp(s1, s2, len) == 0;
#endif
}

void StringUtil::utf16_to_utf8(const Utf16Char* utf16_str, size_t utf16_len, StringBuilder& out_utf8_str)
{
    if (!utf16_str || utf16_len == 0)
    {
        out_utf8_str.sure_null_terminator_but_not_append();
        return;
    }

    // UTF-16 characters can be converted to UTF-8 with at most 4 bytes per character
    // Add extra space for safety
    size_t max_utf8_size = utf16_len * 4 + 1;
    out_utf8_str.reserve(max_utf8_size);

    // Convert UTF-16 to UTF-8
    char* end = utf8::unchecked::utf16to8(utf16_str, utf16_str + utf16_len, out_utf8_str.get_current_write_ptr());
    *end = '\0';

    // Calculate actual size written
    size_t actual_size = static_cast<size_t>(std::distance(out_utf8_str.get_mut_data(), end));

    // Assign to output string
    out_utf8_str.resize(actual_size);
}
} // namespace utils
} // namespace leanclr

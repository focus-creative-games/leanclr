#include "string_util.h"

#include "alloc/general_allocation.h"
#include "build_config.h"

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
        if (s1[i] == '\0')
            return true;
    }
    return true;
}

} // namespace

namespace
{
template <typename T>
T duplicate_or_panic(RtResult<T> result)
{
    if (result.is_err())
        leanclr::panic("String allocation failed");
    return result.unwrap();
}
} // namespace

RtResult<const char*> StringUtil::try_strdup(const char* str)
{
    if (!str)
        return static_cast<const char*>(nullptr);
    size_t bytes;
    if (!CheckedSize::add(std::strlen(str), 1, bytes))
        return RtErr::OutOfMemory;
    char* copy = static_cast<char*>(alloc::GeneralAllocation::malloc(bytes));
    if (!copy)
        return RtErr::OutOfMemory;
    std::memcpy(copy, str, bytes);
    return static_cast<const char*>(copy);
}

RtResult<const Utf16Char*> StringUtil::try_strdup_utf16_with_null_terminator(const Utf16Char* str, size_t length)
{
    if (!str)
        return static_cast<const Utf16Char*>(nullptr);
    size_t count, bytes;
    if (!CheckedSize::add(length, 1, count) || !CheckedSize::multiply(count, sizeof(Utf16Char), bytes))
        return RtErr::OutOfMemory;
    auto* copy = static_cast<Utf16Char*>(alloc::GeneralAllocation::malloc(bytes));
    if (!copy)
        return RtErr::OutOfMemory;
    if (length)
        std::memcpy(copy, str, length * sizeof(Utf16Char));
    copy[length] = 0;
    return static_cast<const Utf16Char*>(copy);
}

RtResult<const Utf16Char*> StringUtil::try_strdup_utf16_without_null_terminator(const Utf16Char* str, size_t length)
{
    if (!str)
        return static_cast<const Utf16Char*>(nullptr);
    size_t bytes;
    if (!CheckedSize::multiply(length, sizeof(Utf16Char), bytes))
        return RtErr::OutOfMemory;
    auto* copy = static_cast<Utf16Char*>(alloc::GeneralAllocation::malloc(bytes ? bytes : sizeof(Utf16Char)));
    if (!copy)
        return RtErr::OutOfMemory;
    if (bytes)
        std::memcpy(copy, str, bytes);
    return static_cast<const Utf16Char*>(copy);
}

RtResult<const char*> StringUtil::try_concat(const char* str1, const char* str2)
{
    if (!str1)
        return try_strdup(str2);
    if (!str2)
        return try_strdup(str1);
    const size_t len1 = std::strlen(str1), len2 = std::strlen(str2);
    size_t length, bytes;
    if (!CheckedSize::add(len1, len2, length) || !CheckedSize::add(length, 1, bytes))
        return RtErr::OutOfMemory;
    auto* result = static_cast<char*>(alloc::GeneralAllocation::malloc(bytes));
    if (!result)
        return RtErr::OutOfMemory;
    std::memcpy(result, str1, len1);
    std::memcpy(result + len1, str2, len2 + 1);
    return static_cast<const char*>(result);
}

const char* StringUtil::strdup(const char* str)
{
    return duplicate_or_panic(try_strdup(str));
}

const Utf16Char* StringUtil::strdup_utf16_with_null_terminator(const Utf16Char* str, size_t length)
{
    return duplicate_or_panic(try_strdup_utf16_with_null_terminator(str, length));
}

const Utf16Char* StringUtil::strdup_utf16_without_null_terminator(const Utf16Char* str, size_t length)
{
    return duplicate_or_panic(try_strdup_utf16_without_null_terminator(str, length));
}

const char* StringUtil::concat(const char* str1, const char* str2)
{
    return duplicate_or_panic(try_concat(str1, str2));
}

bool StringUtil::equals_ignorecase(const char* s1, const char* s2)
{
    if (!s1 && !s2)
        return true;
    if (!s1 || !s2)
        return false;

    return portable_equals_ignorecase(s1, s2);
}

bool StringUtil::equals_ignorecase_n(const char* s1, const char* s2, size_t len)
{
    if (!s1 && !s2)
        return true;
    if (!s1 || !s2)
        return false;
    return portable_equals_ignorecase_n(s1, s2, len);
}
} // namespace utils
} // namespace leanclr

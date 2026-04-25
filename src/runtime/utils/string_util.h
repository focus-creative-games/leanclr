#pragma once

#include <cstring>
#include <string>

#include "core/rt_base.h"
#include "hash_util.h"

namespace leanclr
{
namespace utils
{

#define DUP_STR_TO_LOCAL_TEMP_ZERO_END_STR(local_temp_str, str, str_len)    \
    char* local_temp_str = (char*)alloca(static_cast<size_t>(str_len) + 1); \
    std::memcpy(local_temp_str, str, static_cast<size_t>(str_len));         \
    local_temp_str[static_cast<size_t>(str_len)] = '\0';

class StringBuilder;

class StringUtil
{
  public:
    static const char* strdup(const char* str);

    static const Utf16Char* strdup_utf16_with_null_terminator(const Utf16Char* str, size_t length);

    static const Utf16Char* strdup_utf16_without_null_terminator(const Utf16Char* str, size_t length);

    static const char* concat(const char* str1, const char* str2);

    static bool equals(const char* str1, const char* str2, size_t len)
    {
        return std::strncmp(str1, str2, len) == 0 && str1[len] == '\0';
    }

    static bool equals_ignorecase(const char* s1, const char* s2);

    static bool equals_ignorecase_n(const char* s1, const char* s2, size_t len);

    static std::string_view trim(std::string_view str)
    {
        size_t start = 0;
        size_t end = str.size();

        while (start < end && std::isspace(static_cast<unsigned char>(str[start])))
        {
            ++start;
        }

        while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
        {
            --end;
        }

        return str.substr(start, end - start);
    }

    static int32_t get_utf16chars_length(const Utf16Char* chars)
    {
        int32_t length = 0;
        while (chars[length] != 0)
        {
            ++length;
        }
        return length;
    }

    static void utf16_to_utf8(const Utf16Char* utf16_str, size_t utf16_len, StringBuilder& out_utf8_str);
};

struct CStrCompare
{
    bool operator()(const char* a, const char* b) const
    {
        return std::strcmp(a, b) == 0;
    }
};

struct CStrHasher
{
    size_t operator()(const char* key) const noexcept
    {
        return std::hash<std::string_view>{}(std::string_view(key));
    }
};

struct Utf16StrWithLen
{
    const Utf16Char* str;
    size_t length;

    Utf16StrWithLen(const Utf16Char* s, size_t len) : str(s), length(len)
    {
    }

    Utf16StrWithLen() : str(nullptr), length(0)
    {
    }
};

struct Utf16StrCompare
{
    bool operator()(const Utf16StrWithLen& a, const Utf16StrWithLen& b) const
    {
        return a.length == b.length && std::memcmp(a.str, b.str, sizeof(Utf16Char) * a.length) == 0;
    }
};

struct Utf16StrHasher
{
    size_t operator()(const Utf16StrWithLen& key) const noexcept
    {
        // Hash by string contents, not pointer addresses
        size_t h = 0;
        for (size_t i = 0; i < key.length; ++i)
        {
            h = HashUtil::combine_hash(h, key.str[i]);
        }
        return h;
    }
};

struct FullNameStr
{
    const char* namespace_name;
    const char* name;

    FullNameStr(const char* ns, const char* n) : namespace_name(ns), name(n)
    {
    }
};

struct FullNameStrCompare
{
    bool operator()(const FullNameStr& a, const FullNameStr& b) const
    {
        return std::strcmp(a.namespace_name, b.namespace_name) == 0 && std::strcmp(a.name, b.name) == 0;
    }
};

// Optional hasher type if users prefer explicit hasher parameter
struct FullNameStrHasher
{
    size_t operator()(const FullNameStr& key) const noexcept
    {
        // Hash by string contents, not pointer addresses
        std::string_view ns = key.namespace_name ? std::string_view(key.namespace_name) : std::string_view();
        std::string_view n = key.name ? std::string_view(key.name) : std::string_view();
        size_t h1 = std::hash<std::string_view>{}(ns);
        size_t h2 = std::hash<std::string_view>{}(n);
        return HashUtil::combine_hash(h1, h2);
    }
};
} // namespace utils
} // namespace leanclr

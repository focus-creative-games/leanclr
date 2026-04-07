#pragma once

#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{

class String
{
  public:
    static RtResultVoid initialize();
    static RtString* get_empty_string();
    static RtString* create_string_from_utf16chars(const uint16_t* str, int32_t length);
    static RtString* create_string_from_utf8chars(const char* str, int32_t length);
    static RtString* create_string_from_utf8cstr(const char* str)
    {
        return create_string_from_utf8chars(str, static_cast<int32_t>(std::strlen(str)));
    }
    static const metadata::RtMethodInfo* get_redirected_ctor_method();
    static constexpr int32_t get_offset_to_string_data()
    {
#ifdef _MSC_VER
        // MSVC: suppress C4840 (unsupported offsetof usage)
#pragma warning(push)
#pragma warning(disable : 4840)
#else
        // GCC/Clang: suppress -Winvalid-offsetof
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
        return static_cast<int32_t>(offsetof(RtString, first_char));
#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif
    }

    static const Utf16Char* get_chars_ptr(RtString* str)
    {
        return &str->first_char;
    }

    static RtResult<Utf16Char> get_chars(RtString* str, int32_t index)
    {
        if ((uint32_t)index >= (uint32_t)str->length)
        {
            RET_ERR(RtErr::IndexOutOfRange);
        }
        RET_OK((&str->first_char)[index]);
    }

    static Utf16Char get_chars_unchecked(RtString* str, int32_t index)
    {
        return (&str->first_char)[index];
    }

    static int32_t get_length(RtString* str)
    {
        return str->length;
    }

    static int32_t get_hash_code(RtString* str);

    // Runtime helpers mirrored from Rust vm::string
    static int32_t get_string_allocation_size(int32_t length);
    static RtString* fast_allocate_string(int32_t length); // Declaration retained
    static RtString* intern_string(RtString* s);
    static bool is_interned_string(RtString* s);
};
} // namespace vm
} // namespace leanclr

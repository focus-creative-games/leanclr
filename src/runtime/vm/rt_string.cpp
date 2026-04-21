
#include "rt_string.h"
#include "gc/garbage_collector.h"
#include "class.h"
#include "field.h"
#include "3rd/utf8/utf8.h"
#include <vector>
#include <string>
#include <cstring>
#include "utils/hashset.h"
#include "utils/hash_util.h"

namespace leanclr
{
namespace vm
{

static metadata::RtClass* g_stringClass = nullptr;
static RtString** g_emptyString_ptr = nullptr;
static const metadata::RtMethodInfo* g_redirectedCtorMethod = nullptr;
namespace
{
struct RtStringHash
{
    size_t operator()(const RtString* s) const noexcept
    {
        const char* bytes = reinterpret_cast<const char*>(&s->first_char);
        std::string_view v(bytes, static_cast<size_t>(s->length) * sizeof(Utf16Char));
        return std::hash<std::string_view>{}(v);
    }
};

struct RtStringEqual
{
    bool operator()(const RtString* a, const RtString* b) const noexcept
    {
        if (a == b)
            return true;
        if (a->length != b->length)
            return false;
        return std::memcmp(&a->first_char, &b->first_char, static_cast<size_t>(a->length) * sizeof(Utf16Char)) == 0;
    }
};
} // namespace

static utils::HashSet<RtString*, RtStringHash, RtStringEqual> g_internTable;

// Removed key builder; Hash/Eq operate directly on RtString contents

static RtResultVoid init_empty_string(metadata::RtClass* stringClass)
{
    assert(g_emptyString_ptr == nullptr);
    g_emptyString_ptr = (RtString**)gc::GarbageCollector::allocate_fixed_reference_array(1);
    RtString* emptyString = String::fast_allocate_string(0);

    *g_emptyString_ptr = emptyString;
    RET_VOID_OK();
}

static RtResultVoid init_static_empty_string(metadata::RtClass* stringClass)
{
    const metadata::RtFieldInfo* emptyField = Class::get_field_for_name(stringClass, "Empty", false);
    if (!emptyField)
    {
        assert(false && "String.Empty field not found");
        RET_ERR(RtErr::ExecutionEngine);
    }
    assert(*g_emptyString_ptr);
    assert((*g_emptyString_ptr)->length == 0);
    RET_ERR_ON_FAIL(Field::set_static_value(emptyField, g_emptyString_ptr));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, empty_str, Field::get_value_object(emptyField, nullptr));
    assert(empty_str == *g_emptyString_ptr);
    RET_VOID_OK();
}

static RtResultVoid init_redirected_ctor_method(metadata::RtClass* stringClass)
{
    for (uint16_t i = 0; i < stringClass->method_count; ++i)
    {
        const metadata::RtMethodInfo* method = stringClass->methods[i];
        if (std::strcmp(method->name, "Ctor") == 0 && method->parameter_count == 4)
        {
            g_redirectedCtorMethod = method;
            RET_VOID_OK();
        }
    }
    assert(false && "String directcted ctor method not found");
    RET_ERR(RtErr::ExecutionEngine);
}

RtResultVoid String::initialize()
{
    metadata::RtClass* stringClass = Class::get_corlib_types().cls_string;
    g_stringClass = stringClass;
    RET_ERR_ON_FAIL(init_empty_string(stringClass));
    RET_ERR_ON_FAIL(init_static_empty_string(stringClass));
    RET_ERR_ON_FAIL(init_redirected_ctor_method(stringClass));
    RET_VOID_OK();
}

constexpr size_t OVER_SIZE_OF_STRING = 4;

RtString* String::get_empty_string()
{
    return *g_emptyString_ptr;
}

const metadata::RtMethodInfo* String::get_redirected_ctor_method()
{
    return g_redirectedCtorMethod;
}

RtString* String::create_string_from_utf16chars(const Utf16Char* str, int32_t length)
{
    RtString* newString = fast_allocate_string(length);

    std::memcpy(&newString->first_char, str, static_cast<size_t>(length) * sizeof(Utf16Char));
    return newString;
}

RtString* String::create_string_from_utf8chars(const char* str, int32_t length)
{
    // First, convert UTF-8 to UTF-16 and count characters
    utils::Vector<Utf16Char> utf16_buffer;
    utf16_buffer.reserve(static_cast<size_t>(length)); // Reserve space (UTF-16 could be shorter or longer)

    const char* start = str;
    const char* end = str + static_cast<size_t>(length);

    while (start < end)
    {
        uint32_t codepoint = utf8::unchecked::next(start);
        utf8::unchecked::append16(codepoint, std::back_inserter(utf16_buffer));
    }

    return create_string_from_utf16chars(utf16_buffer.data(), static_cast<int32_t>(utf16_buffer.size()));
}

int32_t String::get_hash_code(RtString* str)
{
    int32_t hash = 5381;
    const Utf16Char* char_data_ptr = &str->first_char;
    const int32_t* int32_data_ptr = reinterpret_cast<const int32_t*>(char_data_ptr);
    assert((size_t)int32_data_ptr % 4 == 0);
    for (int32_t i = 0, n = str->length / 2; i < n; ++i)
    {
        hash = ((hash << 5) + hash) + int32_data_ptr[i];
    }
    if (str->length % 2 != 0)
    {
        hash = ((hash << 5) + hash) + static_cast<int32_t>(char_data_ptr[str->length - 1]);
    }
    return hash;
}

int32_t String::get_string_allocation_size(int32_t length)
{
    const size_t bytes = sizeof(RtString) - OVER_SIZE_OF_STRING + sizeof(Utf16Char) /* extra one character*/ +
                         static_cast<size_t>(length) * sizeof(Utf16Char);
    return static_cast<int32_t>(bytes);
}

RtString* String::fast_allocate_string(int32_t length)
{
    // String::GetLegacyNonRandomizedHashCode need zero terminated string, so we allocate one extra character.
    // TODO: can we optimize it out? we have redirected String::GetHashCode and String::GetLegacyNonRandomizedHashCode to
    // the intrinsic implementation which does not require zero-termination.
    RtString* newString = (RtString*)gc::GarbageCollector::allocate_object_not_contains_references(
        g_stringClass, static_cast<size_t>(get_string_allocation_size(length)));
    newString->length = static_cast<int32_t>(length);
    return newString;
}

RtString* String::intern_string(RtString* s)
{
    if (s == nullptr)
        return s;
    auto it = g_internTable.find(s);
    if (it != g_internTable.end())
        return *it;
    g_internTable.emplace(s);
    return s;
}

bool String::is_interned_string(RtString* s)
{
    if (s == nullptr)
        return false;
    return g_internTable.find(s) != g_internTable.end();
}

} // namespace vm
} // namespace leanclr

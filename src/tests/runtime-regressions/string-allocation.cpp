#include "utils/string_util.h"
#include "alloc/general_allocation.h"
#include <cstdio>
#include <cstdlib>
using namespace leanclr;
static alloc::MemoryCallbacks original;
static bool fail;
static size_t live, attempts;

static void* allocate(size_t size)
{
    ++attempts;
    if (fail)
        return nullptr;
    void* p = original.malloc(size);
    if (p)
        ++live;
    return p;
}

static void release(void* p)
{
    --live;
    original.free(p);
}

static void check(bool b)
{
    if (!b)
        std::abort();
}

template <typename T>
static void oom(RtResult<T> r)
{
    check(r.is_err() && r.unwrap_err() == RtErr::OutOfMemory);
}

template <typename T>
static T take(RtResult<T> r)
{
    check(r.is_ok());
    return r.unwrap();
}

int main()
{
    check(utils::StringUtil::equals_ignorecase("AsSeMbLy.DLL", "assembly.dll"));
    check(!utils::StringUtil::equals_ignorecase("I", "\xc4\xb1"));
    check(!utils::StringUtil::equals_ignorecase("\xc3\x84", "\xc3\xa4"));
    const char a[] = {'A', 0, 'x'}, b[] = {'a', 0, 'y'};
    check(utils::StringUtil::equals_ignorecase_n(a, b, 100));
    check(utils::StringUtil::equals_ignorecase_n("abcd", "ABxx", 2));
    check(!utils::StringUtil::equals_ignorecase_n("abcd", "ABxx", 3));

    original = alloc::GeneralAllocation::get_memory_callbacks();
    auto cb = original;
    cb.malloc = allocate;
    cb.free = release;
    check(alloc::GeneralAllocation::set_memory_callbacks(cb));
    const Utf16Char text[] = {0x4e2d, 0xd83d, 0xde00, 0};
    check(take(utils::StringUtil::try_strdup(nullptr)) == nullptr);
    check(take(utils::StringUtil::try_concat(nullptr, nullptr)) == nullptr);
    const char* s = take(utils::StringUtil::try_concat("ab", "cd"));
    check(std::strcmp(s, "abcd") == 0);
    release(const_cast<char*>(s));
    s = take(utils::StringUtil::try_concat(nullptr, ""));
    check(s && !*s);
    release(const_cast<char*>(s));
    auto u = take(utils::StringUtil::try_strdup_utf16_with_null_terminator(text, 3));
    check(std::memcmp(u, text, sizeof(text)) == 0);
    release(const_cast<Utf16Char*>(u));
    u = take(utils::StringUtil::try_strdup_utf16_without_null_terminator(text, 3));
    check(std::memcmp(u, text, 6) == 0);
    release(const_cast<Utf16Char*>(u));
    u = take(utils::StringUtil::try_strdup_utf16_without_null_terminator(text, 0));
    check(u != nullptr);
    release(const_cast<Utf16Char*>(u));
    size_t before = attempts;
    oom(utils::StringUtil::try_strdup_utf16_with_null_terminator(text, SIZE_MAX));
    oom(utils::StringUtil::try_strdup_utf16_with_null_terminator(text, SIZE_MAX / 2));
    oom(utils::StringUtil::try_strdup_utf16_without_null_terminator(text, SIZE_MAX));
    check(attempts == before);
    fail = true;
    oom(utils::StringUtil::try_strdup("x"));
    oom(utils::StringUtil::try_concat("a", "b"));
    oom(utils::StringUtil::try_strdup_utf16_with_null_terminator(text, 3));
    oom(utils::StringUtil::try_strdup_utf16_without_null_terminator(text, 3));
    check(live == 0);
    fail = false;
    s = utils::StringUtil::strdup("recovered");
    check(std::strcmp(s, "recovered") == 0);
    release(const_cast<char*>(s));
    check(live == 0);
    return 0;
}

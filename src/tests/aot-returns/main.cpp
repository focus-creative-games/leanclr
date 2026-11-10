#include "codegen/aot_result.h"
#include "test_assert.h"
#include <cstdio>
#include <cstdlib>
#include <limits>

struct alignas(8) Object
{
    uintptr_t value;
};

using Pointer = leanclr::codegen::AotPointerResult<Object*>;
using Void = leanclr::codegen::AotVoidResult;
using leanclr::core::RtErr;
using leanclr::core::Unit;
static_assert(std::is_trivially_copyable<Pointer>::value, "pointer result must be trivial even in Debug");
static_assert(sizeof(Pointer) == sizeof(uintptr_t), "pointer result must be a single machine word");

static void check(bool value, const char* name)
{
    if (!value)
    {
        std::fprintf(stderr, "%s\n", name);
        std::exit(1);
    }
}

// Exported entry points also serve as LLVM IR ABI probes. They intentionally
// take runtime inputs rather than only returning compile-time success values.
extern "C" Pointer aot_pointer_probe(Object* value, uint8_t error)
{
    if (error)
        return static_cast<RtErr>(error);
    return value;
}

extern "C" Void aot_void_probe(uint8_t error)
{
    if (error)
        return static_cast<RtErr>(error);
    return Unit{};
}

extern "C" leanclr::core::Result<Object*> ordinary_pointer_probe(Object* value, uint8_t error)
{
    if (error)
        return static_cast<RtErr>(error);
    return value;
}

extern "C" leanclr::core::ResultVoid ordinary_void_probe(uint8_t error)
{
    if (error)
        return static_cast<RtErr>(error);
    return Unit{};
}

int main()
{
    configure_test_assertions();
    Object object{};
    const uintptr_t addresses[] = {0, reinterpret_cast<uintptr_t>(&object), std::numeric_limits<uintptr_t>::max() & ~uintptr_t(7),
                                   uintptr_t(1) << (sizeof(uintptr_t) * 8 - 1)};
    for (uintptr_t address : addresses)
    {
        auto* value = reinterpret_cast<Object*>(address); // Never dereferenced.
        auto result = aot_pointer_probe(value, 0);
        check(result.is_ok() && result.unwrap() == value, "pointer/null/high-bit success lost");
        leanclr::core::Result<Object*> boundary = result;
        check(boundary.is_ok() && boundary.unwrap() == value, "pointer runtime boundary");
        Pointer back(std::move(boundary));
        check(back.is_ok() && back.unwrap() == value, "pointer AOT boundary");
    }
    for (unsigned code = 1; code <= static_cast<unsigned>(RtErr::ModuleAlreadyLoaded); ++code)
    {
        auto error = static_cast<RtErr>(code);
        Pointer p = aot_pointer_probe(nullptr, static_cast<uint8_t>(code));
        check(p.is_err() && p.unwrap_err() == error, "pointer error encoding (including OOM)");
        leanclr::core::Result<Object*> old = p;
        check(old.is_err() && old.unwrap_err() == error, "pointer error boundary");
        Pointer back(std::move(old));
        check(back.is_err() && back.unwrap_err() == error, "pointer error reverse boundary");
        Void v = aot_void_probe(static_cast<uint8_t>(code));
        leanclr::core::ResultVoid old_void = v;
        check(old_void.is_err() && old_void.unwrap_err() == error, "void error boundary");
        Void reverse(std::move(old_void));
        check(reverse.is_err() && reverse.unwrap_err() == error, "void reverse boundary");
    }
    Void success = aot_void_probe(0);
    leanclr::core::ResultVoid ordinary = success;
    check(ordinary.is_ok(), "void success must not become error None");
    Void reverse(std::move(ordinary));
    check(reverse.is_ok(), "void success reverse boundary");
    Pointer outer(&object);
    auto nested = aot_pointer_probe(nullptr, static_cast<uint8_t>(RtErr::OutOfMemory));
    check(nested.is_err() && outer.unwrap() == &object, "nested error contaminated earlier success");
    std::puts("AOT return protocol boundary checks passed");
}

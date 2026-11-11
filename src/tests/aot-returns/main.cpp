#include "codegen/aot_result.h"
#include "test_assert.h"
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <initializer_list>

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

extern "C" leanclr::codegen::AotInt32Result<int32_t> aot_int32_probe(int32_t value, uint8_t error)
{
    if (error)
        return static_cast<RtErr>(error);
    return value;
}

extern "C" leanclr::codegen::AotInt32Result<uint32_t> aot_uint32_probe(uint32_t value, uint8_t error)
{
    if (error)
        return static_cast<RtErr>(error);
    return value;
}

extern "C" leanclr::core::Result<int32_t> ordinary_int32_probe(int32_t value, uint8_t error)
{
    if (error)
        return static_cast<RtErr>(error);
    return value;
}

template <typename T>
static void integer_boundaries(T value)
{
    leanclr::codegen::AotInt32Result<T> packed(value);
    check(packed.is_ok() && packed.unwrap() == value, "integer payload including sign/high bit");
    leanclr::core::Result<T> ordinary = packed;
    check(ordinary.is_ok() && ordinary.unwrap() == value, "integer legacy boundary");
    leanclr::codegen::AotInt32Result<T> back(std::move(ordinary));
    check(back.is_ok() && back.unwrap() == value, "integer packed boundary");
}

struct WidePacket
{
    int64_t integer;
    double floating;
    Object* reference;
};

extern "C" RtErr aot_wide_probe(WidePacket value, uint8_t error, WidePacket* output) noexcept
{
    if (error)
        return static_cast<RtErr>(error);
    *output = value;
    return RtErr::None;
}

static void wide_boundaries()
{
    Object marker = {};
    WidePacket input = {INT64_MIN, -0.125, &marker};
    auto success = leanclr::codegen::invoke_aot_status_out<WidePacket>(aot_wide_probe, input, uint8_t(0));
    check(success.is_ok() && success.unwrap().integer == INT64_MIN && success.unwrap().floating == -0.125 && success.unwrap().reference == &marker,
          "wide status-out payload");
    for (unsigned code = 1; code <= 255; ++code)
    {
        WidePacket output = {17, 0.25, nullptr};
        auto status = aot_wide_probe(input, static_cast<uint8_t>(code), &output);
        check(status == static_cast<RtErr>(code) && output.integer == 17 && output.floating == 0.25 && output.reference == nullptr,
              "wide failure must leave output untouched");
        auto result = leanclr::codegen::invoke_aot_status_out<WidePacket>(aot_wide_probe, input, static_cast<uint8_t>(code));
        check(result.is_err() && result.unwrap_err() == status, "wide adapter error propagation");
    }
}

int main()
{
    wide_boundaries();
    configure_test_assertions();
    for (unsigned code = 1; code <= 255; ++code)
    {
        auto signed_error = aot_int32_probe(std::numeric_limits<int32_t>::min(), static_cast<uint8_t>(code));
        auto unsigned_error = aot_uint32_probe(std::numeric_limits<uint32_t>::max(), static_cast<uint8_t>(code));
        check(signed_error.is_err() && signed_error.unwrap_err() == static_cast<RtErr>(code), "signed error code encoding");
        leanclr::core::Result<uint32_t> old = unsigned_error;
        check(old.is_err() && old.unwrap_err() == static_cast<RtErr>(code), "unsigned error legacy boundary");
        leanclr::codegen::AotInt32Result<uint32_t> back(std::move(old));
        check(back.is_err() && back.unwrap_err() == static_cast<RtErr>(code), "unsigned error reverse boundary");
    }
    uint32_t bits = 0;
    for (unsigned i = 0; i < 4096; ++i)
    {
        int32_t signed_value;
        std::memcpy(&signed_value, &bits, sizeof(bits));
        integer_boundaries(signed_value);
        integer_boundaries(bits);
        bits = bits * 1664525u + 1013904223u;
    }
    for (uint32_t value : {0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu})
    {
        int32_t signed_value;
        std::memcpy(&signed_value, &value, sizeof(value));
        integer_boundaries(signed_value);
        integer_boundaries(value);
    }
    leanclr::codegen::AotInt32Result<uint32_t> outer_integer(UINT32_MAX);
    auto inner_integer = aot_int32_probe(0, static_cast<uint8_t>(RtErr::OutOfMemory));
    check(inner_integer.is_err() && outer_integer.unwrap() == UINT32_MAX, "nested integer result independence");
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

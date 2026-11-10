#pragma once
#include "core/rt_result.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <type_traits>

#if defined(_MSC_VER)
#define BENCH_NOINLINE __declspec(noinline)
#else
#define BENCH_NOINLINE __attribute__((noinline))
#endif

namespace result_bench
{
using leanclr::core::Result;
using leanclr::core::ResultVoid;
using leanclr::core::RtErr;
using leanclr::core::Unit;

struct Block
{
    uint64_t a, b;
};

struct alignas(8) Item
{
    uint32_t value;
};

// An experiment, not a replacement for Result's non-trivial payload support.
template <class T>
struct TrivialResult
{
    static_assert(std::is_trivially_copyable<T>::value, "Only trivial payloads");
    T value;
    RtErr error;

    TrivialResult(T v) : value(v), error(RtErr::None)
    {
    }

    TrivialResult(RtErr e) : value(), error(e)
    {
    }

    TrivialResult(const TrivialResult&) = delete;
    TrivialResult(TrivialResult&&) = default;

    bool is_err() const
    {
        return error != RtErr::None;
    }

    RtErr unwrap_err() const
    {
        return error;
    }

    T unwrap() const
    {
        return value;
    }
};

template <>
struct TrivialResult<Item*>
{
    uintptr_t bits;

    TrivialResult(Item* v) : bits(reinterpret_cast<uintptr_t>(v))
    {
    }

    TrivialResult(RtErr e) : bits((static_cast<uintptr_t>(e) << 1) | 1)
    {
    }

    TrivialResult(const TrivialResult&) = delete;
    TrivialResult(TrivialResult&&) = default;

    bool is_err() const
    {
        return (bits & 1) != 0;
    }

    RtErr unwrap_err() const
    {
        return static_cast<RtErr>(bits >> 1);
    }

    Item* unwrap() const
    {
        return reinterpret_cast<Item*>(bits);
    }
};

static_assert(std::is_trivially_copyable<TrivialResult<uint32_t>>::value, "ABI experiment must stay trivial");
static_assert(std::is_trivially_copyable<TrivialResult<Item*>>::value, "Pointer experiment must stay trivial");

template <class T>
struct Packed
{
    // A full-width 64-bit value or large struct has no spare error bits.
    typedef TrivialResult<T> Type;

    static Type ok(T v)
    {
        return Type(v);
    }

    static Type fail(RtErr e)
    {
        return Type(e);
    }

    static bool bad(const Type& r)
    {
        return r.is_err();
    }

    static RtErr error(const Type& r)
    {
        return r.unwrap_err();
    }

    static T value(const Type& r)
    {
        return r.unwrap();
    }
};

template <>
struct Packed<uint32_t>
{
    typedef uint64_t Type;

    static Type ok(uint32_t v)
    {
        return v;
    }

    static Type fail(RtErr e)
    {
        return static_cast<uint64_t>(e) << 32;
    }

    static bool bad(Type r)
    {
        return (r >> 32) != 0;
    }

    static RtErr error(Type r)
    {
        return static_cast<RtErr>(r >> 32);
    }

    static uint32_t value(Type r)
    {
        return static_cast<uint32_t>(r);
    }
};

template <>
struct Packed<Item*>
{
    typedef uintptr_t Type;

    static Type ok(Item* v)
    {
        return reinterpret_cast<uintptr_t>(v);
    }

    static Type fail(RtErr e)
    {
        return (static_cast<uintptr_t>(e) << 1) | 1;
    }

    static bool bad(Type r)
    {
        return (r & 1) != 0;
    }

    static RtErr error(Type r)
    {
        return static_cast<RtErr>(r >> 1);
    }

    static Item* value(Type r)
    {
        return reinterpret_cast<Item*>(r);
    }
};

struct Context
{
    RtErr error;
};

struct Failure
{
    RtErr error;
};

struct Observation
{
    uint64_t checksum;
    uint64_t errors;
};

Observation run(unsigned workload, uint32_t iterations, uint32_t seed, uint32_t error_period);
Observation reference(unsigned workload, uint32_t iterations, uint32_t seed, uint32_t error_period);
bool verify();
} // namespace result_bench

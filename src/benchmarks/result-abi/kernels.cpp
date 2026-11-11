#include "bench.h"
#include <limits>

namespace result_bench
{
namespace
{
Item items[2] = {{17}, {29}};

uint32_t advance(uint32_t x)
{
    return x * 1664525u + 1013904223u;
}
template <class T>
T payload(uint32_t x);

template <>
uint32_t payload<uint32_t>(uint32_t x)
{
    return advance(x);
}

template <>
uint64_t payload<uint64_t>(uint32_t x)
{
    return (uint64_t(advance(x)) << 32) | x;
}

template <>
Block payload<Block>(uint32_t x)
{
    return {uint64_t(advance(x)) << 32 | x, uint64_t(x) * 0x123456789ULL};
}

template <>
Item* payload<Item*>(uint32_t x)
{
    return (x & 3) == 0 ? nullptr : items + (x & 1);
}
// Callers do work after each successful call, preventing tail-call elimination
// alone from deciding this experiment.
template <class T>
T adjust(T x);

template <>
uint32_t adjust<uint32_t>(uint32_t x)
{
    return x * 3u + 7u;
}

template <>
uint64_t adjust<uint64_t>(uint64_t x)
{
    return x * 3u + 7u;
}

template <>
Block adjust<Block>(Block x)
{
    return {x.a * 3u + 7u, x.b ^ x.a};
}

template <>
Item* adjust<Item*>(Item* x)
{
    return x == nullptr ? nullptr : x == items ? items + 1 : items;
}
template <class T>
uint64_t digest(T x);

template <>
uint64_t digest<uint32_t>(uint32_t x)
{
    return x;
}

template <>
uint64_t digest<uint64_t>(uint64_t x)
{
    return x;
}

template <>
uint64_t digest<Block>(Block x)
{
    return x.a ^ x.b;
}

template <>
uint64_t digest<Item*>(Item* x)
{
    return x ? x->value : 0;
}

#if BENCH_PROTOCOL <= 1
#if BENCH_PROTOCOL == 0
template <class T>
using Reply = Result<T>;
#else
template <class T>
using Reply = TrivialResult<T>;
#endif
template <class T, unsigned Depth>
BENCH_NOINLINE Reply<T> call(uint32_t x, bool fail)
{
    auto r = call<T, Depth - 1>(x, fail);
    if (r.is_err())
        return r.unwrap_err();
    return adjust<T>(r.unwrap());
}

template <class T>
struct Leaf
{
    static BENCH_NOINLINE Reply<T> invoke(uint32_t x, bool fail)
    {
        if (fail)
            return RtErr::Argument;
        return payload<T>(x);
    }
};
#elif BENCH_PROTOCOL == 2
template <class T>
using Reply = typename Packed<T>::Type;

template <class T, unsigned Depth>
BENCH_NOINLINE Reply<T> call(uint32_t x, bool fail)
{
    auto r = call<T, Depth - 1>(x, fail);
    if (Packed<T>::bad(r))
        return Packed<T>::fail(Packed<T>::error(r));
    return Packed<T>::ok(adjust<T>(Packed<T>::value(r)));
}

template <class T>
struct Leaf
{
    static BENCH_NOINLINE Reply<T> invoke(uint32_t x, bool fail)
    {
        return fail ? Packed<T>::fail(RtErr::Argument) : Packed<T>::ok(payload<T>(x));
    }
};
#elif BENCH_PROTOCOL == 3
template <class T, unsigned Depth>
BENCH_NOINLINE RtErr call(uint32_t x, bool fail, T* output)
{
    const auto err = call<T, Depth - 1>(x, fail, output);
    if (err != RtErr::None)
        return err;
    *output = adjust<T>(*output);
    return RtErr::None;
}

template <class T>
struct Leaf
{
    static BENCH_NOINLINE RtErr invoke(uint32_t x, bool fail, T* output)
    {
        if (fail)
            return RtErr::Argument;
        *output = payload<T>(x);
        return RtErr::None;
    }
};
#elif BENCH_PROTOCOL == 4
template <class T, unsigned Depth>
BENCH_NOINLINE T call(uint32_t x, bool fail, Context& ctx)
{
    T value = call<T, Depth - 1>(x, fail, ctx);
    if (ctx.error != RtErr::None)
        return T{};
    return adjust<T>(value);
}

template <class T>
struct Leaf
{
    static BENCH_NOINLINE T invoke(uint32_t x, bool fail, Context& ctx)
    {
        if (fail)
        {
            ctx.error = RtErr::Argument;
            return T{};
        }
        return payload<T>(x);
    }
};
#else
template <class T, unsigned Depth>
BENCH_NOINLINE T call(uint32_t x, bool fail)
{
    return adjust<T>(call<T, Depth - 1>(x, fail));
}

template <class T>
struct Leaf
{
    static BENCH_NOINLINE T invoke(uint32_t x, bool fail)
    {
#if BENCH_PROTOCOL == 5
        if (fail)
            throw Failure{RtErr::Argument};
#else
        (void)fail;
#endif
        return payload<T>(x);
    }
};
#endif

// Terminate recursion without C++17 if-constexpr or partial function specialization.
#if BENCH_PROTOCOL == 3
#define LEAF(T)                                                    \
    template <>                                                    \
    BENCH_NOINLINE RtErr call<T, 0>(uint32_t x, bool fail, T* out) \
    {                                                              \
        return Leaf<T>::invoke(x, fail, out);                      \
    }
#elif BENCH_PROTOCOL == 4
#define LEAF(T)                                                      \
    template <>                                                      \
    BENCH_NOINLINE T call<T, 0>(uint32_t x, bool fail, Context& ctx) \
    {                                                                \
        return Leaf<T>::invoke(x, fail, ctx);                        \
    }
#elif BENCH_PROTOCOL <= 2
#define LEAF(T)                                               \
    template <>                                               \
    BENCH_NOINLINE Reply<T> call<T, 0>(uint32_t x, bool fail) \
    {                                                         \
        return Leaf<T>::invoke(x, fail);                      \
    }
#else
#define LEAF(T)                                        \
    template <>                                        \
    BENCH_NOINLINE T call<T, 0>(uint32_t x, bool fail) \
    {                                                  \
        return Leaf<T>::invoke(x, fail);               \
    }
#endif
LEAF(uint32_t)
LEAF(uint64_t)
LEAF(Block)
LEAF(Item*)
#undef LEAF

template <class T>
Observation execute(uint32_t iterations, uint32_t seed, uint32_t error_period)
{
    Observation total = {};
    for (uint32_t i = 0; i < iterations; ++i)
    {
        seed = advance(seed);
        const bool fail = error_period != 0 && i % error_period == 0;
#if BENCH_PROTOCOL <= 1
        auto r = call<T, 3>(seed, fail);
        if (r.is_err())
        {
            total.errors += static_cast<unsigned>(r.unwrap_err());
            continue;
        }
        total.checksum += digest<T>(r.unwrap());
#elif BENCH_PROTOCOL == 2
        auto r = call<T, 3>(seed, fail);
        if (Packed<T>::bad(r))
        {
            total.errors += static_cast<unsigned>(Packed<T>::error(r));
            continue;
        }
        total.checksum += digest<T>(Packed<T>::value(r));
#elif BENCH_PROTOCOL == 3
        T output = {};
        const RtErr err = call<T, 3>(seed, fail, &output);
        if (err != RtErr::None)
        {
            total.errors += static_cast<unsigned>(err);
            continue;
        }
        total.checksum += digest<T>(output);
#elif BENCH_PROTOCOL == 4
        Context ctx = {RtErr::None};
        const T output = call<T, 3>(seed, fail, ctx);
        if (ctx.error != RtErr::None)
        {
            total.errors += static_cast<unsigned>(ctx.error);
            continue;
        }
        total.checksum += digest<T>(output);
#elif BENCH_PROTOCOL == 5
        try
        {
            total.checksum += digest<T>(call<T, 3>(seed, fail));
        }
        catch (const Failure& e)
        {
            total.errors += static_cast<unsigned>(e.error);
        }
#else
        total.checksum += digest<T>(call<T, 3>(seed, fail));
#endif
    }
    return total;
}

template <class T>
Observation expected(uint32_t iterations, uint32_t seed, uint32_t period)
{
    Observation total = {};
    for (uint32_t i = 0; i < iterations; ++i)
    {
        seed = advance(seed);
        if (period && i % period == 0)
            total.errors += static_cast<unsigned>(RtErr::Argument);
        else
            total.checksum += digest<T>(adjust<T>(adjust<T>(adjust<T>(payload<T>(seed)))));
    }
    return total;
}
} // namespace

// Keep the real ResultVoid in this test: Result<Unit> has a different ABI.
#if BENCH_PROTOCOL == 0
using VoidReply = ResultVoid;
#elif BENCH_PROTOCOL <= 3
using VoidReply = RtErr;
#else
using VoidReply = void;
#endif
template <unsigned Depth>
BENCH_NOINLINE VoidReply void_call(uint32_t* state, bool fail, Context& ctx)
{
#if BENCH_PROTOCOL == 0
    auto r = void_call<Depth - 1>(state, fail, ctx);
    if (r.is_err())
        return r.unwrap_err();
    *state = adjust<uint32_t>(*state);
    return Unit{};
#elif BENCH_PROTOCOL <= 3
    const auto err = void_call<Depth - 1>(state, fail, ctx);
    if (err != RtErr::None)
        return err;
    *state = adjust<uint32_t>(*state);
    return RtErr::None;
#else
    void_call<Depth - 1>(state, fail, ctx);
#if BENCH_PROTOCOL == 4
    if (ctx.error != RtErr::None)
        return;
#endif
    *state = adjust<uint32_t>(*state);
#endif
}

template <>
BENCH_NOINLINE VoidReply void_call<0>(uint32_t* state, bool fail, Context& ctx)
{
#if BENCH_PROTOCOL == 0
    if (fail)
        return RtErr::Argument;
    *state = advance(*state);
    return Unit{};
#elif BENCH_PROTOCOL <= 3
    if (fail)
        return RtErr::Argument;
    *state = advance(*state);
    return RtErr::None;
#elif BENCH_PROTOCOL == 4
    if (fail)
    {
        ctx.error = RtErr::Argument;
        return;
    }
    *state = advance(*state);
#elif BENCH_PROTOCOL == 5
    if (fail)
        throw Failure{RtErr::Argument};
    *state = advance(*state);
#else
    (void)fail;
    (void)ctx;
    *state = advance(*state);
#endif
}

static Observation execute_void(uint32_t n, uint32_t seed, uint32_t period)
{
    Observation total = {};
    for (uint32_t i = 0; i < n; ++i)
    {
        seed = advance(seed);
        uint32_t state = seed;
        const bool fail = period && i % period == 0;
        Context ctx = {RtErr::None};
#if BENCH_PROTOCOL == 0
        auto r = void_call<3>(&state, fail, ctx);
        if (r.is_err())
        {
            total.errors += static_cast<unsigned>(r.unwrap_err());
            continue;
        }
#elif BENCH_PROTOCOL <= 3
        const auto error = void_call<3>(&state, fail, ctx);
        if (error != RtErr::None)
        {
            total.errors += static_cast<unsigned>(error);
            continue;
        }
#elif BENCH_PROTOCOL == 4
        void_call<3>(&state, fail, ctx);
        if (ctx.error != RtErr::None)
        {
            total.errors += static_cast<unsigned>(ctx.error);
            continue;
        }
#elif BENCH_PROTOCOL == 5
        try
        {
            void_call<3>(&state, fail, ctx);
        }
        catch (const Failure& e)
        {
            total.errors += static_cast<unsigned>(e.error);
            continue;
        }
#else
        void_call<3>(&state, fail, ctx);
#endif
        total.checksum += state;
    }
    return total;
}

Observation run(unsigned workload, uint32_t n, uint32_t seed, uint32_t period)
{
    switch (workload)
    {
    case 0:
        return execute<uint32_t>(n, seed, period);
    case 1:
        return execute<uint64_t>(n, seed, period);
    case 2:
        return execute<Block>(n, seed, period);
    case 3:
        return execute<Item*>(n, seed, period);
    default:
        return execute_void(n, seed, period);
    }
}

Observation reference(unsigned workload, uint32_t n, uint32_t seed, uint32_t period)
{
    switch (workload)
    {
    case 0:
        return expected<uint32_t>(n, seed, period);
    case 1:
        return expected<uint64_t>(n, seed, period);
    case 2:
        return expected<Block>(n, seed, period);
    case 3:
        return expected<Item*>(n, seed, period);
    default:
        return expected<uint32_t>(n, seed, period);
    }
}

bool verify()
{
    for (unsigned w = 0; w < 5; ++w)
        for (uint32_t seed : {0u, 1u, 0x80000000u, 0xffffffffu})
            for (uint32_t period : {0u, 1u, 7u, 1024u})
            {
#if BENCH_PROTOCOL == 6
                if (period)
                    continue;
#endif
                const auto a = run(w, 4097, seed, period);
                const auto b = reference(w, 4097, seed, period);
                if (a.checksum != b.checksum || a.errors != b.errors)
                    return false;
            }
    // Full-domain packing, including a successful null, is independent of timing.
    for (uint32_t x : {0u, 1u, 0x80000000u, 0xffffffffu})
        if (Packed<uint32_t>::bad(Packed<uint32_t>::ok(x)) || Packed<uint32_t>::value(Packed<uint32_t>::ok(x)) != x)
            return false;
    if (Packed<Item*>::bad(Packed<Item*>::ok(nullptr)) || Packed<Item*>::value(Packed<Item*>::ok(nullptr)) != nullptr)
        return false;
    return true;
}
} // namespace result_bench

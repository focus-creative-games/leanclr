#include "bench.h"
using namespace result_bench;

extern "C"
{
    BENCH_NOINLINE Result<uint32_t> current_u32(uint32_t x, bool fail)
    {
        if (fail)
            return RtErr::Argument;
        return x;
    }

    BENCH_NOINLINE Result<Item*> current_pointer(Item* x, bool fail)
    {
        if (fail)
            return RtErr::Argument;
        return x;
    }

    BENCH_NOINLINE ResultVoid current_void(bool fail)
    {
        if (fail)
            return RtErr::Argument;
        return Unit{};
    }

    BENCH_NOINLINE TrivialResult<uint32_t> trivial_u32(uint32_t x, bool fail)
    {
        if (fail)
            return RtErr::Argument;
        return x;
    }

    BENCH_NOINLINE TrivialResult<Item*> trivial_pointer(Item* x, bool fail)
    {
        if (fail)
            return RtErr::Argument;
        return x;
    }

    BENCH_NOINLINE RtErr scalar_void(bool fail)
    {
        return fail ? RtErr::Argument : RtErr::None;
    }

    BENCH_NOINLINE uint64_t packed_u32(uint32_t x, bool fail)
    {
        return fail ? Packed<uint32_t>::fail(RtErr::Argument) : Packed<uint32_t>::ok(x);
    }

    BENCH_NOINLINE RtErr status_u32(uint32_t x, bool fail, uint32_t* output)
    {
        if (fail)
            return RtErr::Argument;
        *output = x;
        return RtErr::None;
    }
}

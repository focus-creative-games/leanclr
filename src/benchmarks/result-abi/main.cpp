#include "bench.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv)
{
    using namespace result_bench;
    if (!verify())
    {
        std::fprintf(stderr, "Protocol semantics mismatch\n");
        return 1;
    }
    if (argc > 1 && std::strcmp(argv[1], "--verify") == 0)
        return 0;
    const uint32_t iterations = argc > 1 ? static_cast<uint32_t>(std::strtoul(argv[1], nullptr, 10)) : 2000000;
    const char* names[] = {"u32-chain", "u64-chain", "struct16-chain", "pointer-chain", "void-chain"};
    for (unsigned w = 0; w < 5; ++w)
        for (uint32_t period : {0u, 1024u, 1u})
        {
#if BENCH_PROTOCOL == 6
            if (period)
                continue;
#endif
            const uint32_t count = period == 1 && iterations > 20000 ? 20000 : iterations;
            const auto warm = run(w, count / 8 + 1, 19, period);
            volatile uint64_t sink = warm.checksum;
            (void)sink;
            const auto begin = std::chrono::steady_clock::now();
            const auto result = run(w, count, 19, period);
            const auto end = std::chrono::steady_clock::now();
            const auto expected = reference(w, count, 19, period);
            if (result.checksum != expected.checksum || result.errors != expected.errors)
                return 2;
            const double ns = std::chrono::duration<double, std::nano>(end - begin).count();
            std::printf(
                "{\"protocol\":\"%s\",\"workload\":\"%s\",\"error_period\":%u,\"iterations\":%u,\"ns_per_op\":%.6f,\"checksum\":\"%llu\",\"errors\":%llu}\n",
                BENCH_NAME, names[w], period, count, ns / count, static_cast<unsigned long long>(result.checksum),
                static_cast<unsigned long long>(result.errors));
        }
    return 0;
}

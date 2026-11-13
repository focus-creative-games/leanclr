#include "vm/calli.h"
#include "interp/interp_defs.h"
#include <cstring>
extern "C" int32_t calli_bench_add(int32_t);
#if LEANCLR_CALLI_SIZE_NATIVE
extern const leanclr::metadata::RtCalliBridge g_calli_bridges[];
extern const uint32_t g_calli_bridge_count;
#endif
int main(int argc, char**)
{
#if LEANCLR_CALLI_SIZE_NATIVE
    for (uint32_t i = 0; i < g_calli_bridge_count; ++i)
    {
        const auto& bridge = g_calli_bridges[i];
        if (std::strcmp(bridge.diagnostic, "1:8(8)") != 0)
            continue;
        leanclr::interp::RtStackObject args{}, ret{};
        args.i32 = argc + 40;
        auto result = leanclr::vm::Calli::invoke_native(&bridge, reinterpret_cast<void*>(calli_bench_add), &args, &ret);
        return result.is_err() || ret.i32 != argc + 41;
    }
    return 2;
#else
    return calli_bench_add(argc + 40) != argc + 41;
#endif
}

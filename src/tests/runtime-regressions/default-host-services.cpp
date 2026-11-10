#include "host/services.h"
#include "build_config.h"
#include <cstdlib>
#include <cstdio>

int main()
{
    using namespace leanclr::host;
    const int64_t before = Services::monotonic_nanoseconds();
    const int64_t utc = Services::utc_ticks_100ns();
    const int64_t after = Services::monotonic_nanoseconds();
    if (after < before || utc <= 0)
        std::abort();
    uint8_t bytes[34] = {};
    bytes[0] = 91;
    bytes[33] = 92;
    const bool entropy = Services::random_bytes(bytes + 1, 32, RandomKind::Cryptographic);
#if LEANCLR_PLATFORM_WIN || LEANCLR_PLATFORM_POSIX
    if (!entropy)
        std::abort();
#else
    (void)entropy; // A portable sandbox can require an injected entropy source.
#endif
    if (bytes[0] != 91 || bytes[33] != 92)
        std::abort();
    std::puts("default host services passed");
}

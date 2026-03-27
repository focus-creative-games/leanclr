#include "rng_crypto_service_provider.h"

#include <cstdlib>
#include <cstring>
#include "alloc/general_allocation.h"

namespace leanclr
{
namespace misc
{

namespace
{
struct RngState
{
    uint64_t s;
};

inline uint8_t next_u8(RngState& state)
{
    // xorshift64*
    uint64_t x = state.s;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    x = x * static_cast<uint64_t>(2685821657736338717ULL);
    state.s = x;
    return static_cast<uint8_t>(x & 0xFF);
}

inline uint64_t fold_seed(const uint8_t* seed_ptr, intptr_t seed_len)
{
    if (seed_ptr != nullptr && seed_len > 0)
    {
        uint64_t acc = 0x9E3779B97F4A7C15ULL;
        size_t len = static_cast<size_t>(seed_len);
        for (size_t i = 0; i < len; ++i)
        {
            uint64_t b = static_cast<uint64_t>(seed_ptr[i]);
            acc ^= b << ((i % 8) * 8);
            // rotate left 13 then xor constant
            acc = (acc << 13) | (acc >> (64 - 13));
            acc ^= 0xBF58476D1CE4E5B9ULL;
        }
        return acc;
    }

    // Derive a non-zero-ish seed from pointer value and a constant when no seed provided
    return (reinterpret_cast<uintptr_t>(seed_ptr)) ^ 0xD1342543DE82EF95ULL;
}
} // namespace

bool RngCryptoServiceProvider::open()
{
    return true;
}

intptr_t RngCryptoServiceProvider::initialize(const uint8_t* seed, intptr_t seed_len)
{
    uint64_t folded = fold_seed(seed, seed_len);
    RngState* state = alloc::GeneralAllocation::malloc_any_zeroed<RngState>();
    if (!state)
    {
        return 0;
    }
    state->s = folded;
    return reinterpret_cast<intptr_t>(state);
}

intptr_t RngCryptoServiceProvider::get_bytes(intptr_t handle, uint8_t* buffer, intptr_t length)
{
    RngState* state = reinterpret_cast<RngState*>(handle);
    size_t len = static_cast<size_t>(length);
    for (size_t i = 0; i < len; ++i)
    {
        buffer[i] = next_u8(*state);
    }
    return handle;
}

void RngCryptoServiceProvider::close(intptr_t handle)
{
    alloc::GeneralAllocation::free(reinterpret_cast<void*>(handle));
}
} // namespace misc
} // namespace leanclr

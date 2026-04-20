#include "bcrypt.h"

#include <cstdint>

#include "rt_time.h"

namespace leanclr
{
namespace platform
{

namespace
{

// A tiny, non-cryptographic PRNG used to back the managed BCryptGenRandom
// P/Invoke. We intentionally pick xorshift64* (Marsaglia, 2003) -- it is a
// few lines of code, has reasonable statistical quality for non-security
// purposes, needs only 64 bits of state, and has no external dependencies.
//
// NOTE: This is NOT cryptographically secure. If managed code relies on
// BCryptGenRandom for security-sensitive randomness, wire this up to the
// platform CSPRNG (BCryptGenRandom on Windows, /dev/urandom on POSIX,
// crypto.getRandomValues in wasm) instead.

class XorShift64
{
  public:
    explicit XorShift64(uint64_t seed) noexcept : _state(seed != 0 ? seed : kFallbackSeed)
    {
    }

    uint64_t next() noexcept
    {
        uint64_t x = _state;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        _state = x;
        // The multiplier gives xorshift64* its improved avalanche over plain xorshift64.
        return x * 0x2545F4914F6CDD1DULL;
    }

  private:
    static constexpr uint64_t kFallbackSeed = 0x9E3779B97F4A7C15ULL; // 2^64 / phi
    uint64_t _state;
};

// Lazy-initialized, process-wide PRNG. Access is NOT synchronized: under a
// concurrent call the worst case is that two threads observe overlapping
// state, which only affects statistical distribution -- acceptable for a
// non-cryptographic generator. Introducing a mutex or <atomic> here would be
// overkill and isn't used elsewhere in the runtime.
uint64_t build_initial_seed() noexcept
{
    // Mix nanosecond clock with the address of a local variable. The address
    // adds a bit of per-process entropy (ASLR) and cannot be predicted solely
    // from the time.
    uint64_t t = static_cast<uint64_t>(os::Time::get_current_time_nanos());
    uint64_t a = reinterpret_cast<uintptr_t>(&t);
    // SplitMix64 finalizer for a strong avalanche from the combined bits.
    uint64_t z = t ^ (a + 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    z = z ^ (z >> 31);
    return z;
}

XorShift64& get_rng() noexcept
{
    static XorShift64 rng(build_initial_seed());
    return rng;
}

} // namespace

void Bcrypt::gen_random(intptr_t /*algo_handle*/, uint8_t* buffer, int32_t length, int32_t /*flags*/)
{
    if (buffer == nullptr || length <= 0)
        return;

    XorShift64& rng = get_rng();

    int32_t i = 0;
    // Fill 8 bytes at a time.
    while (length - i >= 8)
    {
        uint64_t r = rng.next();
        buffer[i + 0] = static_cast<uint8_t>(r);
        buffer[i + 1] = static_cast<uint8_t>(r >> 8);
        buffer[i + 2] = static_cast<uint8_t>(r >> 16);
        buffer[i + 3] = static_cast<uint8_t>(r >> 24);
        buffer[i + 4] = static_cast<uint8_t>(r >> 32);
        buffer[i + 5] = static_cast<uint8_t>(r >> 40);
        buffer[i + 6] = static_cast<uint8_t>(r >> 48);
        buffer[i + 7] = static_cast<uint8_t>(r >> 56);
        i += 8;
    }

    // Tail (< 8 bytes).
    if (i < length)
    {
        uint64_t r = rng.next();
        for (int shift = 0; i < length; ++i, shift += 8)
            buffer[i] = static_cast<uint8_t>(r >> shift);
    }
}

} // namespace platform
} // namespace leanclr

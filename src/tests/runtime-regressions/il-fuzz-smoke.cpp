#include <cstdint>
#include <cstddef>
extern "C" int LLVMFuzzerTestOneInput(const uint8_t*, size_t);

int main()
{
    uint32_t state = 0x13579bdf;
    uint8_t bytes[512];
    for (size_t test = 0; test < 10000; ++test)
    {
        for (auto& b : bytes)
        {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            b = static_cast<uint8_t>(state);
        }
        LLVMFuzzerTestOneInput(bytes, test % sizeof(bytes));
    }
    return 0;
}

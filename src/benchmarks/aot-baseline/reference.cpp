#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif
struct Pair
{
    uint32_t x, y;
};

struct ITransform
{
    virtual uint32_t apply(uint32_t x) = 0;

    virtual ~ITransform()
    {
    }
};

struct Transform final : ITransform
{
    uint32_t apply(uint32_t x) override
    {
        return x * 1664525u + 1013904223u;
    }
};

static Transform transformer;
static ITransform* volatile target = &transformer;
static uint32_t data[1024];

static NOINLINE uint32_t step(uint32_t x)
{
    return x * 1664525u + 1013904223u;
}

static NOINLINE uint32_t chain1(uint32_t x)
{
    return step(x);
}

static NOINLINE uint32_t chain2(uint32_t x)
{
    return chain1(x);
}

static NOINLINE Pair pair_step(Pair p)
{
    p.x = step(p.x);
    p.y ^= p.x;
    return p;
}

template <class T>
static NOINLINE T identity(T x)
{
    return x;
}

static uint32_t run(unsigned kind, int n, uint32_t x)
{
    Pair p = {x, 17};
    ITransform* t = target;
    for (int i = 0; i < n; ++i)
    {
        switch (kind)
        {
        case 0:
        case 2:
            x = x * 1664525u + 1013904223u;
            break;
        case 1:
            x = chain2(x);
            break;
        case 3:
            x = x * 1664525u + data[i & 1023];
            break;
        case 4:
            x = t->apply(x);
            break;
        case 5:
            p = pair_step(p);
            break;
        case 6:
            x = step(identity(x));
            break;
        }
    }
    return kind == 5 ? p.x ^ p.y : x;
}

int main(int argc, char** argv)
{
    const int n = argc > 1 ? std::atoi(argv[1]) : 2000000;
    if (n <= 0)
        return 1;
    for (unsigned i = 0; i < 1024; ++i)
        data[i] = i * 17u;
    const char* names[] = {"arithmetic", "calls", "properties", "array", "interfaces", "structs", "generics"};
    for (unsigned k = 0; k < 7; ++k)
    {
        volatile uint32_t warm = run(k, n / 8 + 1, 19);
        (void)warm;
        const auto start = std::chrono::steady_clock::now();
        const uint32_t checksum = run(k, n, 19);
        const auto end = std::chrono::steady_clock::now();
        const double ns = std::chrono::duration<double, std::nano>(end - start).count() / n;
        std::printf("{\"workload\":\"%s\",\"execution\":\"cpp-reference\",\"iterations\":%d,\"checksum\":\"%u\",\"ns_per_op\":%.6f}\n", names[k], n, checksum,
                    ns);
    }
}

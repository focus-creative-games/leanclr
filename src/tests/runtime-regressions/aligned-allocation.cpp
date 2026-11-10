#include "alloc/mem_pool.h"
#include "alloc/general_allocator.h"
#include <cstdlib>
#include <cstring>
#include <vector>
using namespace leanclr;
static alloc::MemoryCallbacks original;
static size_t live, aligned_live;
static bool fail;

static void check(bool b)
{
    if (!b)
        std::abort();
}

// Valid ordinary allocations, deliberately not aligned to 64 bytes.
static void* ordinary(size_t n)
{
    if (fail || n > SIZE_MAX - 128)
        return nullptr;
    void* base = std::malloc(n + 128);
    if (!base)
        return nullptr;
    uintptr_t address = (reinterpret_cast<uintptr_t>(base) + sizeof(void*) + 63) & ~uintptr_t(63);
    address += 16;
    void* result = reinterpret_cast<void*>(address);
    static_cast<void**>(result)[-1] = base;
    ++live;
    return result;
}

static void release(void* p)
{
    --live;
    std::free(static_cast<void**>(p)[-1]);
}

static void* zero(size_t count, size_t n)
{
    void* p = ordinary(count * n);
    if (p)
        std::memset(p, 0, count * n);
    return p;
}

static void* aligned(size_t n, size_t a)
{
    if (fail)
        return nullptr;
    void* p = original.aligned_malloc(n, a);
    if (p)
        ++aligned_live;
    return p;
}

static void aligned_release(void* p)
{
    --aligned_live;
    original.aligned_free(p);
}

struct alignas(64) Wide
{
    unsigned char bytes[64];
};

struct alignas(256) Wider
{
    unsigned char bytes[256];
};

static bool is_aligned(const void* p, size_t a)
{
    return p && reinterpret_cast<uintptr_t>(p) % a == 0;
}

int main()
{
    original = alloc::GeneralAllocation::get_memory_callbacks();
    auto cb = original;
    cb.malloc = ordinary;
    cb.calloc = zero;
    cb.free = release;
    cb.aligned_malloc = aligned;
    cb.aligned_free = aligned_release;
    check(alloc::GeneralAllocation::set_memory_callbacks(cb));
    {
        alloc::MemPool pool(4096, 4096, 4096);

        struct Block
        {
            uint8_t* data;
            size_t size;
        };

        std::vector<Block> blocks;
        for (size_t i = 0; i < 96; ++i)
        {
            size_t a = size_t(64) << (i % 7);
            size_t n = a * 2;
            auto* p = pool.malloc_zeroed(n, a);
            check(is_aligned(p, a));
            for (size_t j = 0; j < n; ++j)
                check(p[j] == 0);
            std::memset(p, 0x5a, n);
            blocks.push_back({p, n});
        }
        check(!pool.malloc_zeroed(SIZE_MAX - 63, 64));
        check(!pool.malloc_zeroed(64, 0) && !pool.malloc_zeroed(64, 3));
        fail = true;
        check(!pool.malloc_zeroed(1024 * 1024, 64));
        fail = false;
        auto* tail = pool.malloc_any_zeroed<Wider>();
        check(is_aligned(tail, 256));
        for (auto b : blocks)
            for (size_t j = 0; j < b.size; ++j)
                check(b.data[j] == 0x5a);
    }
    check(live == 0);
    alloc::GeneralAllocator<Wide> allocator;
    auto* p = allocator.allocate(3);
    check(is_aligned(p, 64) && aligned_live == 1);
    allocator.deallocate(p, 3);
    check(!allocator.allocate(SIZE_MAX));
    auto* w = alloc::GeneralAllocation::new_any<Wider>();
    check(is_aligned(w, 256));
    for (auto b : w->bytes)
        check(b == 0);
    alloc::GeneralAllocation::delete_any(w);
    auto* array = alloc::GeneralAllocation::calloc_any<Wide>(3);
    check(is_aligned(array, 64));
    for (size_t i = 0; i < 3; ++i)
        for (auto b : array[i].bytes)
            check(b == 0);
    alloc::GeneralAllocation::free_any(array);
    fail = true;
    check(!allocator.allocate(1) && !alloc::GeneralAllocation::new_any<Wider>());
    fail = false;
    check(live == 0 && aligned_live == 0);
    return 0;
}

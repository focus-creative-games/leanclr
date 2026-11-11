#include <cstdint>
#include "core/stl_compat.h"
#include "core/rt_result.h"
#include "metadata/rt_metadata.h"
#include "utils/hashmap.h"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <new>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace leanclr;

static void check(bool value, const char* message)
{
    if (!value)
    {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

// Count requested payload bytes through the host allocation contract. Header
// and CRT overhead, input strings, RSS and unrelated runtime heaps are excluded.
struct alignas(std::max_align_t) Header
{
    size_t bytes;
};

static alloc::MemoryCallbacks original;
static size_t allocations, frees, live_bytes, peak_bytes, total_bytes;

static void* allocate(size_t size)
{
    if (size > SIZE_MAX - sizeof(Header))
        return nullptr;
    void* storage = original.malloc(size + sizeof(Header));
    if (!storage)
        return nullptr;
    auto* header = new (storage) Header{size};
    ++allocations;
    live_bytes += size;
    total_bytes += size;
    if (live_bytes > peak_bytes)
        peak_bytes = live_bytes;
    return header + 1;
}

static void release(void* pointer)
{
    if (!pointer)
        return;
    auto* header = static_cast<Header*>(pointer) - 1;
    live_bytes -= header->bytes;
    ++frees;
    original.free(header);
}

static void* zero(size_t count, size_t size)
{
    if (size && count > SIZE_MAX / size)
        return nullptr;
    size_t bytes = count * size;
    auto* pointer = allocate(bytes);
    if (pointer)
        std::memset(pointer, 0, bytes);
    return pointer;
}

static void* resize(void* pointer, size_t size)
{
    if (!pointer)
        return allocate(size);
    if (!size)
    {
        release(pointer);
        return nullptr;
    }
    if (size > SIZE_MAX - sizeof(Header))
        return nullptr;
    auto* old = static_cast<Header*>(pointer) - 1;
    size_t previous = old->bytes;
    void* storage = original.realloc(old, size + sizeof(Header));
    if (!storage)
        return nullptr;
    auto* header = new (storage) Header{size};
    live_bytes = live_bytes - previous + size;
    total_bytes += size;
    if (live_bytes > peak_bytes)
        peak_bytes = live_bytes;
    return header + 1;
}

struct Tracked
{
    static int live;
    int value;

    explicit Tracked(int v) : value(v)
    {
        ++live;
    }

    Tracked(const Tracked& v) : value(v.value)
    {
        ++live;
    }

    Tracked(Tracked&& v) noexcept : value(v.value)
    {
        ++live;
    }

    ~Tracked()
    {
        --live;
    }
};

int Tracked::live = 0;

static int callback(int value) noexcept
{
    return value;
}

static_assert(std::is_same<core::function_return<decltype(callback)>::type, int>::value, "noexcept callback traits");

int main(int argc, char** argv)
{
    unsigned iterations = argc > 1 ? static_cast<unsigned>(std::strtoul(argv[1], nullptr, 10)) : 1000000;
    check(iterations > 0, "positive iterations");
    {
        std::optional<Tracked> first(Tracked(17));
        std::optional<Tracked> moved(std::move(first));
        check(moved.has_value() && moved->value == 17, "optional move retains value");
        moved.reset();
        first.reset();
    }
    check(Tracked::live == 0, "optional owns each lifetime exactly once");
    std::string_view empty;
    check(empty.empty() && empty.size() == 0, "default empty view");
    const char text[] = "prefix-tail";
    std::string_view prefix(text, 6);
    check(std::string(prefix.data(), prefix.size()) == "prefix" && prefix.data()[prefix.size()] == '-', "view need not terminate with NUL");
    check(prefix.substr(1, 3) == std::string_view("ref"), "bounded substring");
    const char other[] = "prefix";
    check(std::hash<std::string_view>{}(prefix) == std::hash<std::string_view>{}(std::string_view(other)), "equal views hash equally");
    check(std::hash<std::string_view>{}(empty) == std::hash<std::string_view>{}(std::string_view("", 0)), "empty views hash equally");

    original = alloc::GeneralAllocation::get_memory_callbacks();
    auto callbacks = original;
    callbacks.malloc = allocate;
    callbacks.calloc = zero;
    callbacks.realloc = resize;
    callbacks.free = release;
    check(alloc::GeneralAllocation::set_memory_callbacks(callbacks), "install tracking before allocation");
    std::vector<std::string> names;
    for (unsigned i = 0; i < 4096; ++i)
    {
        char name[64];
        std::snprintf(name, sizeof(name), "metadata-name-%04u-suffix-%u", i, i * 13);
        names.emplace_back(name);
    }
    uint64_t checksum = 0;
    size_t buckets = 0, steady_bytes = 0;
    double build_ns = 0, lookup_ns = 0;
    {
        utils::HashMap<std::string_view, uint32_t> map;
        auto start = std::chrono::steady_clock::now();
        for (unsigned i = 0; i < names.size(); ++i)
            map.emplace(std::string_view(names[i].data(), names[i].size()), i * 3);
        auto end = std::chrono::steady_clock::now();
        build_ns = std::chrono::duration<double, std::nano>(end - start).count() / names.size();
        buckets = map.bucket_count();
        steady_bytes = live_bytes;
        uint32_t seed = 19;
        start = std::chrono::steady_clock::now();
        for (unsigned i = 0; i < iterations; ++i)
        {
            seed = seed * 1664525u + 1013904223u;
            unsigned index = seed & 4095;
            const auto& name = names[index];
            auto found = map.find(std::string_view(name.data(), name.size()));
            check(found != map.end() && found->second == index * 3, "view lookup retains owned input lifetime");
            checksum += found->second;
        }
        end = std::chrono::steady_clock::now();
        lookup_ns = std::chrono::duration<double, std::nano>(end - start).count() / iterations;
    }
    check(live_bytes == 0 && allocations == frees, "all callback allocations released");
#if defined(_MSVC_LANG)
    const long standard = _MSVC_LANG;
#else
    const long standard = __cplusplus;
#endif
    std::printf("{\"standard\":%ld,\"cplusplus\":%ld,\"iterations\":%u,\"checksum\":\"%llu\",\"build_ns\":%.6f,\"lookup_ns\":%.6f,"
                "\"allocations\":%zu,\"frees\":%zu,\"peak_payload_bytes\":%zu,\"steady_payload_bytes\":%zu,\"total_payload_bytes\":%zu,\"buckets\":%zu,"
                "\"optional_u64_bytes\":%zu,\"optional_method_body_bytes\":%zu,\"string_view_bytes\":%zu,\"result_u32_bytes\":%zu,\"method_info_bytes\":%zu}\n",
                standard, static_cast<long>(__cplusplus), iterations, static_cast<unsigned long long>(checksum), build_ns, lookup_ns, allocations, frees,
                peak_bytes, steady_bytes, total_bytes, buckets, sizeof(std::optional<uint64_t>), sizeof(std::optional<metadata::RtMethodBody>),
                sizeof(std::string_view), sizeof(core::Result<uint32_t>), sizeof(metadata::RtMethodInfo));
}

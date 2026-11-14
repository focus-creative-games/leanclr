#include "codegen/leanclr_common.h"
#include "bcl/mono/managed_layouts.h"
#include "vm/rt_exception.h"
#include "host/services.h"
#include "alloc/general_allocation.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/generic_method.h"
#include "metadata/metadata_cache.h"
#include "vm/runtime.h"
#include "vm/settings.h"
#include "profile/profile.h"
#include "gc/mark_sweep/small_heap_arena.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>

using namespace leanclr;
#ifdef HOTSPOT_AOT
extern metadata::RtAotModulesData g_aot_modules_data;
#endif
static std::string library_path;

static bool load(const char* name, const char* extension, host::FileData& data)
{
    std::ifstream file(library_path + "/" + name + "." + extension, std::ios::binary | std::ios::ate);
    if (!file)
        return false;
    const auto length = file.tellg();
    if (length < 0)
        return false;
    auto* bytes = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(static_cast<size_t>(length)));
    if (!bytes)
        return false;
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(bytes), length))
    {
        alloc::GeneralAllocation::free(bytes);
        return false;
    }
    data = {bytes, static_cast<size_t>(length), false};
    return true;
}

static void require(bool ok, const char* message)
{
    if (!ok)
    {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

static alloc::MemoryCallbacks original_allocators;
static bool deny_allocations;
static int mallocs_before_failure = -1;
static size_t live_aligned_allocations;

static void* failing_calloc(size_t count, size_t size)
{
    return deny_allocations ? nullptr : original_allocators.calloc(count, size);
}

static void* failing_malloc(size_t size)
{
    if (deny_allocations || mallocs_before_failure == 0)
        return nullptr;
    if (mallocs_before_failure > 0)
        --mallocs_before_failure;
    return original_allocators.malloc(size);
}

static void* counted_aligned_malloc(size_t size, size_t alignment)
{
    void* result = original_allocators.aligned_malloc(size, alignment);
    if (result)
        ++live_aligned_allocations;
    return result;
}

static void counted_aligned_free(void* ptr)
{
    if (ptr)
    {
        require(live_aligned_allocations > 0, "aligned allocation ownership");
        --live_aligned_allocations;
    }
    original_allocators.aligned_free(ptr);
}

static void cached_arrays()
{
    vm::SzArrayAllocationCache cache{};
    auto* ints = vm::Class::get_corlib_types().cls_int32;
    auto* longs = vm::Class::get_corlib_types().cls_int64;
    auto negative = LEANCLR_CODEGEN_NEW_SZARRAY_CACHED(ints, -1, cache, "native-cache-test");
    require(negative.is_err() && negative.unwrap_err() == RtErr::Overflow && !cache.klass, "negative length must not publish cache");
    auto empty = LEANCLR_CODEGEN_NEW_SZARRAY_CACHED(ints, 0, cache, "native-cache-test");
    require(empty.is_ok() && empty.unwrap()->length == 0, "empty cached array");
    auto* int_array_class = cache.klass;
    auto switched = LEANCLR_CODEGEN_NEW_SZARRAY_CACHED(longs, 17, cache, "native-cache-test");
    require(switched.is_ok() && cache.klass != int_array_class && cache.element_size == 8, "cache element change");
    auto* bytes = reinterpret_cast<uint8_t*>(&switched.unwrap()->first_data);
    for (size_t i = 0; i < 17 * sizeof(int64_t); ++i)
        require(bytes[i] == 0, "cached arrays must be zero initialized");
    auto warm = LEANCLR_CODEGEN_NEW_SZARRAY_CACHED(ints, 1, cache, "native-cache-test");
    require(warm.is_ok(), "warm cache before injected OOM");
#if LEANCLR_GC_DEBUG
    auto* site = static_cast<const gc::GcAllocSite*>(warm.unwrap()->__sync_block);
    require(site && site->kind == gc::GcAllocSiteKind::Codegen, "array allocation must preserve GC debug site");
#endif
    const int64_t before = gc::GarbageCollector::get_used_size();
    const int64_t pressure_before = gc::GcPressure::get_bytes_allocated_since_last_gc();
    const int64_t lifetime_before = gc::GcPressure::get_total_allocated_bytes();
    deny_allocations = true;
    auto oom = LEANCLR_CODEGEN_NEW_SZARRAY_CACHED(ints, INT32_MAX, cache, "native-cache-test");
    deny_allocations = false;
    require(oom.is_err() && oom.unwrap_err() == RtErr::OutOfMemory, "cached array OOM must propagate");
    require(gc::GarbageCollector::get_used_size() == before, "failed allocation must not charge bytes");
    require(gc::GcPressure::get_bytes_allocated_since_last_gc() == pressure_before && gc::GcPressure::get_total_allocated_bytes() == lifetime_before,
            "failed allocation must not alter GC pressure/lifetime bytes");
    require(cache.klass == int_array_class, "failed allocation must preserve valid metadata cache");
    require(LEANCLR_CODEGEN_NEW_SZARRAY_CACHED(ints, 3, cache, "native-cache-test").is_ok(), "cache must recover after OOM");
    static_assert(sizeof(vm::SzArrayAllocationCache) <= 2 * sizeof(void*), "cache size budget");
}

static void arena_boundaries()
{
    // Cursor state reuses the old count slot; do not increase arena metadata.
    static_assert(sizeof(gc::SmallHeapArena) == 4 * sizeof(void*) + 256 + sizeof(uint64_t), "arena metadata size budget");
    const size_t cases[][2] = {{16, 2048}, {24, 65}, {96, 170}, {256, 64}, {8192, 2}};
    auto* object_class = vm::Class::get_corlib_types().cls_object;
    for (const auto& entry : cases)
    {
        const size_t block_size = entry[0], count = entry[1];
        // Include unusable trailing bytes: the cursor may never reach them.
        void* memory = alloc::GeneralAllocation::aligned_malloc(block_size * count + 7, gc::GC_ALIGN);
        require(memory != nullptr, "allocate test arena");
        std::memset(memory, 0, block_size * count + 7);
        gc::SmallHeapArena arena(memory, block_size * count + 7, block_size);
        std::vector<vm::RtObject*> initial;
        std::unordered_set<void*> live;
        const size_t first = count / 2;
        gc::GCAliveObjectBitmap marks;
        for (size_t i = 0; i < first; ++i)
        {
            auto* obj = static_cast<vm::RtObject*>(arena.allocate_block());
            require(obj != nullptr, "fresh arena allocation");
            std::memset(obj, 0x5A, block_size);
            obj->klass = object_class;
            initial.push_back(obj);
            if ((i & 1) == 0)
            {
                marks.mark(obj);
                live.insert(obj);
            }
        }
        require(!arena.is_full(), "partially unused arena remains available");
        require(arena.find_object_containing(static_cast<uint8_t*>(memory) + first * block_size) == nullptr, "virgin blocks are not allocated objects");
        require(arena.sweep(marks) == first / 2, "partial sweep preserves marked blocks");
        size_t added = 0;
        while (auto* raw = arena.allocate_block())
        {
            require(live.insert(raw).second, "allocation may not overlap surviving object");
            auto* bytes = static_cast<uint8_t*>(raw);
            for (size_t i = 0; i < block_size; ++i)
                require(bytes[i] == 0, "fresh/recycled arena block must be entirely zeroed");
            static_cast<vm::RtObject*>(raw)->klass = object_class;
            ++added;
        }
#if LEANCLR_GC_DEBUG
        require(added == count - first, "quarantined blocks must not be reused");
#else
        require(added == count - (first + 1) / 2, "recycled blocks and virgin tail must both remain allocatable");
#endif
        require(arena.is_full() && arena.allocate_block() == nullptr, "full cursor is bounded");
        for (size_t i = 0; i < first; i += 2)
        {
            auto* bytes = reinterpret_cast<uint8_t*>(initial[i]);
            for (size_t j = sizeof(vm::RtObject); j < block_size; ++j)
                require(bytes[j] == 0x5A, "surviving payload must remain intact");
        }
        gc::GCAliveObjectBitmap empty;
        arena.sweep(empty);
#if LEANCLR_GC_DEBUG
        require(arena.is_full(), "debug quarantine must keep exhausted arena unavailable");
        // Directly owned arenas are outside the heap's lifetime; release their
        // diagnostic copies when this test destroys the backing storage.
        for (size_t i = 0; i < count; ++i)
        {
            auto* obj = reinterpret_cast<vm::RtObject*>(static_cast<uint8_t*>(memory) + i * block_size);
            alloc::GeneralAllocation::free(obj->__sync_block);
            obj->__sync_block = nullptr;
        }
#else
        require(arena.is_empty(), "fully swept arena becomes empty");
        for (size_t i = 0; i < count; ++i)
        {
            auto* raw = static_cast<uint8_t*>(arena.allocate_block());
            require(raw != nullptr, "refill after full sweep");
            for (size_t j = 0; j < block_size; ++j)
                require(raw[j] == 0, "swept blocks remain zero initialized");
        }
        require(arena.allocate_block() == nullptr, "recycled full arena is bounded");
#endif
    }
#if !LEANCLR_GC_DEBUG
    gc::SizeClassPool pool(16384, 96, gc::GC_ALIGN);
    deny_allocations = true;
    require(pool.allocate_block() == nullptr, "arena-vector OOM rolls back refill");
    deny_allocations = false;
    const size_t aligned_before = live_aligned_allocations;
    // Permit vector capacity growth, then fail the arena metadata allocation
    // after its aligned payload was obtained. The payload must roll back.
    mallocs_before_failure = 1;
    require(pool.allocate_block() == nullptr, "arena metadata OOM propagates");
    mallocs_before_failure = -1;
    require(live_aligned_allocations == aligned_before, "partial arena payload must be freed on OOM");
    auto* recovered = static_cast<vm::RtObject*>(pool.allocate_block());
    require(recovered != nullptr, "pool refill recovers after OOM");
    recovered->klass = object_class;
    gc::GCAliveObjectBitmap empty;
    int64_t freed = 0;
    pool.sweep(empty, freed);
    require(freed == 96, "recovered arena remains sweepable and correctly charged");
    require(live_aligned_allocations == aligned_before, "recovered arena payload must be freed after sweep");
#endif
    std::puts("Arena virgin/recycled/tail/bitmap/quarantine/OOM boundaries passed");
}

int main(int argc, char** argv)
{
    require(argc == 2, "Usage: hotspot-tests managed-dir");
    library_path = argv[1];
    original_allocators = alloc::GeneralAllocation::get_memory_callbacks();
    auto callbacks = original_allocators;
    callbacks.calloc = failing_calloc;
    callbacks.malloc = failing_malloc;
    callbacks.aligned_malloc = counted_aligned_malloc;
    callbacks.aligned_free = counted_aligned_free;
    require(alloc::GeneralAllocation::set_memory_callbacks(callbacks), "install failure injection before startup");
    require(host::Services::set_file_loader(load), "Set loader");
#ifdef HOTSPOT_AOT
    vm::Settings::set_aot_modules_data(&g_aot_modules_data);
#endif
    require(vm::Runtime::initialize().is_ok(), "Initialize runtime");
    auto assembly = vm::Assembly::load_by_name("HotspotFixtures");
    require(assembly.is_ok(), "Load fixture");
    auto type = assembly.unwrap()->mod->get_class_by_name("HotspotFixtures", false, false);
    require(type.is_ok() && type.unwrap(), "Find fixture class");
    require(vm::Class::initialize_all(type.unwrap()).is_ok(), "Initialize fixture class");
    for (const char* name : {"Casts", "Delegates", "Arrays", "Errors"})
    {
        auto* method = vm::Method::find_matched_method_in_class_by_name(type.unwrap(), name);
        require(method != nullptr, "Find boundary entry");
#ifdef HOTSPOT_AOT
        require(std::strcmp(name, "Errors") == 0 || method->invoker_type == metadata::RtInvokerType::Aot, "Boundary entry must be AOT");
#else
        require(method->invoker_type == metadata::RtInvokerType::Interpreter, "Boundary entry must be interpreted");
#endif
        auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, nullptr);
        if (result.is_err())
        {
            auto* error = vm::Exception::get_and_clear_current_exception();
            if (error && error->message)
            {
                auto chars = vm::String::get_chars_ptr(error->message);
                for (int32_t i = 0; i < vm::String::get_length(error->message); ++i)
                    std::fputc(chars[i] < 128 ? static_cast<char>(chars[i]) : '?', stderr);
                std::fputc('\n', stderr);
            }
            if (error)
                (void)vm::Exception::report_unhandled_exception(error);
            std::fprintf(stderr, "%s failed: %d %s\n", name, static_cast<int>(result.unwrap_err()),
                         error ? reinterpret_cast<vm::RtObject*>(error)->klass->name : "no managed exception");
        }
        require(result.is_ok() && result.unwrap() && *reinterpret_cast<int32_t*>(result.unwrap() + 1) == 0, name);
        std::printf("%s passed\n", name);
    }
    const auto* fallback = vm::Method::find_matched_method_in_class_by_name(type.unwrap(), "StaticInterpreted");
    require(fallback && fallback->invoker_type == metadata::RtInvokerType::Interpreter, "Delegate fallback target must be interpreted");
    cached_arrays();
    arena_boundaries();
    std::puts("Cached array negative/zero/mixed-element/OOM/recovery tests passed");
    // Exercise runtime/IL-level bindings directly. Mono BCL CreateDelegate's
    // reflection validator currently rejects enum/underlying return aliases,
    // whereas .NET accepts them. The typed fast path must nevertheless never
    // reinterpret a packed primitive ABI as a legacy enum ABI.
    const char* factories[] = {"GetEnumDelegate", "GetIntDelegate", "GetWideEnumDelegate", "GetWideIntDelegate"};
    const char* targets[] = {"IntReturn", "EnumReturn", "WideIntReturn", "WideEnumReturn"};
    const char* callers[] = {"CallEnum", "CallInt", "CallWideEnum", "CallWideInt"};
    for (int direction = 0; direction < 4; ++direction)
    {
        const auto* get = vm::Method::find_matched_method_in_class_by_name(type.unwrap(), factories[direction]);
        require(get != nullptr, "Find alias delegate factory");
        auto value = vm::Runtime::invoke_with_run_cctor(get, nullptr, nullptr);
        require(value.is_ok(), "Create return-alias delegate");
        require(value.unwrap() != nullptr, "Non-null alias delegate");
        auto* delegate_value = reinterpret_cast<vm::RtMulticastDelegate*>(value.unwrap());
        delegate_value->dele.method = vm::Method::find_matched_method_in_class_by_name(type.unwrap(), targets[direction]);
        const auto* call = vm::Method::find_matched_method_in_class_by_name(type.unwrap(), callers[direction]);
        const void* args[] = {delegate_value};
        require(call && delegate_value->dele.method, "Find alias target/caller");
        auto result = vm::Runtime::invoke_with_run_cctor(call, nullptr, args);
        require(result.is_ok() && result.unwrap(), "Invoke return-alias delegate");
        const uint64_t bits = direction < 2 ? static_cast<uint32_t>(*reinterpret_cast<int32_t*>(result.unwrap() + 1))
                                            : static_cast<uint64_t>(*reinterpret_cast<int64_t*>(result.unwrap() + 1));
        require(bits == (direction < 2 ? UINT64_C(0xFEDCBA98) : UINT64_C(0xFEDCBA9876543210)), "Return alias must use target's actual ABI");
    }
    std::puts("Enum/underlying return ABI alias tests passed");
    int64_t previous_used = 0;
    for (int repeat = 0; repeat < 8; ++repeat)
    {
        for (const char* name : {"PrepareGC", "VerifyGC", "ReleaseGC"})
        {
            const auto* method = vm::Method::find_matched_method_in_class_by_name(type.unwrap(), name);
            require(method != nullptr, "Find GC boundary entry");
            auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, nullptr);
            require(result.is_ok(), "Cached reference-containing arrays survive collection");
            // Collection is deliberately at the host boundary, never inserted
            // into an allocation helper with unverified safepoint assumptions.
            gc::GarbageCollector::collect();
        }
        const int64_t used = gc::GarbageCollector::get_used_size();
        if (repeat > 1)
            require(used == previous_used, "Repeated allocation/release must reach bounded retained object bytes");
        previous_used = used;
    }
    std::puts("GC survival and bounded retained-object accounting passed");
    auto invoke_finalizer_case = [&](const char* name)
    {
        const auto* method = vm::Method::find_matched_method_in_class_by_name(type.unwrap(), name);
        require(method != nullptr, "Find array/finalizer entry");
        auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, nullptr);
        require(result.is_ok(), name);
    };
    invoke_finalizer_case("PrepareFinalizerArray");
    gc::GarbageCollector::collect();
    gc::GarbageCollector::collect();
    invoke_finalizer_case("VerifyFinalizerAlive");
    invoke_finalizer_case("ReleaseGC");
    gc::GarbageCollector::collect();
    gc::GarbageCollector::collect();
    gc::GarbageCollector::collect();
    invoke_finalizer_case("VerifyFinalizerReleased");
    std::puts("Reference-array element finalization passed");
    return 0;
}

#include "../aot-returns/test_assert.h"
#include "vm/gchandle.h"
#include "vm/class.h"
#include "vm/runtime.h"
#include "vm/settings.h"
#include "vm/rt_array.h"
#include "vm/array_class.h"
#include "vm/reflection.h"
#include "alloc/general_allocation.h"
#include <fstream>
#include <string>
#include "gc/garbage_collector.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <vector>
#if defined(__EMSCRIPTEN__)
#include <emscripten/heap.h>
#elif defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#elif defined(__linux__)
#include <sys/resource.h>
#endif

using namespace leanclr;

static void check(bool condition)
{
    if (!condition)
        std::abort();
}

static std::string runtime_directory;

static bool load(const char* name, const char* extension, vm::FileData& data)
{
    std::ifstream file(runtime_directory + "/" + name + "." + extension, std::ios::binary | std::ios::ate);
    if (!file)
        return false;
    const auto size = file.tellg();
    if (size < 0)
        return false;
    auto* bytes = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(static_cast<size_t>(size)));
    if (!bytes)
        return false;
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(bytes), size))
    {
        alloc::GeneralAllocation::free(bytes);
        return false;
    }
    data = {bytes, static_cast<size_t>(size), false};
    return true;
}

int main(int argc, char** argv)
{
    configure_test_assertions();
    const size_t count = argc > 1 ? static_cast<size_t>(std::strtoul(argv[1], nullptr, 10)) : 16384;
    const size_t iterations = argc > 2 ? static_cast<size_t>(std::strtoul(argv[2], nullptr, 10)) : 100000;
    check(count != 0 && count <= 1048576 && (count & (count - 1)) == 0 && iterations != 0);
    vm::RtObject fallback{};
    std::vector<vm::RtObject*> objects;
    const bool membership = argc > 3 && argv[3][0] != 0;
    if (membership)
    {
        runtime_directory = argv[3];
        vm::Settings::set_file_loader(load);
        check(vm::Runtime::initialize().is_ok());
        auto reference_array = vm::ArrayClass::get_szarray_class_from_element_class(vm::Class::get_corlib_types().cls_string);
        check(reference_array.is_ok());
        auto early_handle = vm::Reflection::get_net10_method_table(reference_array.unwrap()->by_val);
        check(early_handle.is_ok() && vm::Class::initialize_all(reference_array.unwrap()).is_ok());
        auto ready_handle = vm::Reflection::get_net10_method_table(reference_array.unwrap()->by_val);
        check(ready_handle.is_ok() && ready_handle.unwrap() == early_handle.unwrap());
        check((*static_cast<const uint32_t*>(ready_handle.unwrap()) & 0x01000000u) != 0);
        const auto* integer = vm::Class::get_corlib_types().cls_int32;
        auto existing = vm::Reflection::get_net10_method_table(integer->by_val);
        check(existing.is_ok());
        auto candidate = vm::ArrayClass::get_array_class_from_element_klass(integer, 7);
        check(candidate.is_ok());
        const size_t old_limit = vm::Settings::get_max_runtime_type_handles();
        const size_t old_count = vm::Reflection::get_net10_type_handle_count();
        vm::Settings::set_max_runtime_type_handles(old_count);
        for (int i = 0; i < 32; ++i)
        {
            auto rejected = vm::Reflection::get_net10_method_table(candidate.unwrap()->by_val);
            check(rejected.is_err() && rejected.unwrap_err() == RtErr::OutOfMemory);
            check(vm::Reflection::get_net10_type_handle_count() == old_count);
            auto retained = vm::Reflection::get_net10_method_table(integer->by_val);
            check(retained.is_ok() && retained.unwrap() == existing.unwrap());
        }
        vm::Settings::set_max_runtime_type_handles(old_limit);
        check(vm::Reflection::get_net10_method_table(candidate.unwrap()->by_val).is_ok());
        auto* byte_type = vm::Class::get_corlib_types().cls_byte;
        for (int i = 0; i < 64; ++i)
            for (int j = 0; j < 128; ++j)
            {
                auto array = LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(byte_type, (i + 1) * 8, "resource benchmark");
                check(array.is_ok());
                objects.push_back(reinterpret_cast<vm::RtObject*>(array.unwrap()));
            }
    }
    else
        objects.push_back(&fallback);
    std::vector<void*> handles;
    std::vector<vm::RtObject**> slots;
    for (size_t i = 0; i < count; ++i)
    {
        auto* handle = vm::GCHandle::new_handle(objects[i % objects.size()], false);
        check(handle != nullptr);
        handles.push_back(handle);
        slots.push_back(vm::GCHandle::get_target_slot(handle));
    }
    uintptr_t checksum = 0;
    const auto start = std::chrono::steady_clock::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        const size_t index = (i * 131) & (count - 1);
        auto* handle = vm::GCHandle::get_handle_by_target_slot(slots[index]);
        check(handle == handles[index]);
        checksum ^= reinterpret_cast<uintptr_t>(handle);
    }
    const auto middle = std::chrono::steady_clock::now();
    for (size_t i = 0; membership && i < iterations; ++i)
    {
        auto* object = objects[(i * 131) % objects.size()];
        const auto* interior = reinterpret_cast<const uint8_t*>(object) + sizeof(vm::RtObject);
        check(gc::GarbageCollector::find_object_containing(interior) == object);
        checksum ^= reinterpret_cast<uintptr_t>(object);
        check(gc::GarbageCollector::find_object_containing(reinterpret_cast<const void*>(~uintptr_t{0})) == nullptr);
    }
    const auto end = std::chrono::steady_clock::now();
    const double slot_ns = std::chrono::duration<double, std::nano>(middle - start).count() / iterations;
    const double membership_ns = std::chrono::duration<double, std::nano>(end - middle).count() / iterations;
    size_t wasm_heap = 0;
    size_t peak_rss = 0;
#if defined(__EMSCRIPTEN__)
    wasm_heap = emscripten_get_heap_size();
#elif defined(_WIN32)
    PROCESS_MEMORY_COUNTERS memory{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &memory, sizeof(memory)))
        peak_rss = memory.PeakWorkingSetSize;
#elif defined(__linux__)
    struct rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) == 0)
        peak_rss = static_cast<size_t>(usage.ru_maxrss) * 1024;
#endif
    std::printf("{\"handles\":%zu,\"objects\":%zu,\"iterations\":%zu,\"slot_ns\":%.3f,\"membership_pair_ns\":%.3f,\"managed_heap_bytes\":%lld,\"wasm_heap_"
                "bytes\":%zu,\"peak_rss_bytes\":%zu,\"checksum\":%llu}\n",
                count, objects.size(), iterations, slot_ns, membership_ns, static_cast<long long>(gc::GarbageCollector::get_used_size()), wasm_heap, peak_rss,
                static_cast<unsigned long long>(checksum));
    auto* old_slot = slots.front();
    for (auto* handle : handles)
        vm::GCHandle::free_handle(handle);
    check(vm::GCHandle::get_handle_by_target_slot(old_slot) == nullptr);
    check(vm::GCHandle::get_handle_by_target_slot(reinterpret_cast<vm::RtObject**>(uintptr_t{1})) == nullptr);
    if (membership)
    {
        gc::GarbageCollector::collect();
        check(gc::GarbageCollector::find_object_containing(objects.front()) == nullptr);
        gc::GarbageCollector::collect();
        check(gc::GarbageCollector::find_object_containing(objects.back()) == nullptr);
    }
}

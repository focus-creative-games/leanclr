#include "host/services.h"
#include "gc/garbage_collector.h"
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
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

using namespace leanclr;
#ifdef BENCH_AOT
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

int main(int argc, char** argv)
{
    require(argc == 5, "Usage: il-host managed-dir method iterations seed");
    library_path = argv[1];
    int n = std::atoi(argv[3]);
    uint32_t seed = static_cast<uint32_t>(std::strtoul(argv[4], nullptr, 10));
    require(n > 0, "Positive iteration count required");
    require(host::Services::set_file_loader(load), "Set loader");
#ifdef BENCH_AOT
    vm::Settings::set_aot_modules_data(&g_aot_modules_data);
#endif
    require(vm::Runtime::initialize().is_ok(), "Initialize runtime");
    auto assembly = vm::Assembly::load_by_name("AotBaseline");
    require(assembly.is_ok(), "Load assembly");
    auto type = assembly.unwrap()->mod->get_class_by_name("Benchmarks", false, false);
    require(type.is_ok() && type.unwrap(), "Find type");
    require(vm::Class::initialize_all(type.unwrap()).is_ok(), "Initialize type");
    const auto* method = vm::Method::find_matched_method_in_class_by_name(type.unwrap(), argv[2]);
    require(method != nullptr, "Find method");
    bool aot = method->invoker_type == metadata::RtInvokerType::Aot;
#ifndef BENCH_AOT
    require(!aot, "Interpreter host unexpectedly registered AOT");
#endif
    int warm_n = n < 16384 ? n : 16384;
    const void* warm_args[] = {&warm_n, &seed};
    for (int w = 0; w < 3; ++w)
        require(vm::Runtime::invoke_with_run_cctor(method, nullptr, warm_args).is_ok(), "Warmup");
    const void* args[] = {&n, &seed};
    const int before_gc = gc::GarbageCollector::get_collection_count();
    const int64_t before_bytes = gc::GarbageCollector::get_used_size();
    auto start = std::chrono::steady_clock::now();
    auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
    auto end = std::chrono::steady_clock::now();
    require(result.is_ok() && result.unwrap(), "Invoke benchmark");
    uint32_t checksum = *reinterpret_cast<const uint32_t*>(result.unwrap() + 1);
    double ns = std::chrono::duration<double, std::nano>(end - start).count() / n;
    std::printf("{\"workload\":\"%s\",\"iterations\":%d,\"checksum\":\"%u\",\"ns_per_op\":%.6f,\"entry\":\"%s\",\"interpreted_ir\":%s,\"gc_count_delta\":%d,"
                "\"used_bytes_delta\":%lld}\n",
                argv[2], n, checksum, ns, aot ? "aot" : "interpreter", method->interp_data ? "true" : "false",
                gc::GarbageCollector::get_collection_count() - before_gc, static_cast<long long>(gc::GarbageCollector::get_used_size() - before_bytes));
    return 0;
}

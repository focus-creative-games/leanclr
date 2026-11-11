#include "alloc/general_allocation.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/generic_method.h"
#include "metadata/metadata_cache.h"
#include "vm/runtime.h"
#include "vm/settings.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

using namespace leanclr;
extern metadata::RtAotModulesData g_aot_modules_data;
static std::string library_path;

static bool load(const char* name, const char* extension, vm::FileData& data)
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
    require(argc >= 2, "Usage: aot-baseline <managed-dir> [iterations]");
    library_path = argv[1];
    const int n = argc >= 3 ? std::atoi(argv[2]) : 2000000;
    require(n > 0, "Iterations must be positive");
    vm::Settings::set_file_loader(load);
    vm::Settings::set_aot_modules_data(&g_aot_modules_data);
    auto init = vm::Runtime::initialize();
    require(init.is_ok(), "Initialization failed");
    auto assembly = vm::Assembly::load_by_name("AotBaseline");
    require(assembly.is_ok(), "Load benchmark assembly failed");
    auto* module = assembly.unwrap()->mod;
    auto type = module->get_class_by_name("Benchmarks", false, false);
    require(type.is_ok() && type.unwrap() != nullptr, "Benchmark type missing");
    auto* klass = type.unwrap();
    auto initialized_type = vm::Class::initialize_all(klass);
    require(initialized_type.is_ok(), "Benchmark class initialization failed");
    const auto* identity_def = vm::Method::find_matched_method_in_class_by_name(klass, "Identity");
    const metadata::RtTypeSig* identity_type = vm::Class::get_corlib_types().cls_uint32->by_val;
    auto identity_inst = metadata::MetadataCache::get_pooled_generic_inst(&identity_type, 1);
    require(identity_inst.is_ok(), "Generic benchmark arguments");
    auto identity_method = vm::GenericMethod::get_method(identity_def, nullptr, identity_inst.unwrap());
    require(identity_method.is_ok(), "Generic benchmark method");
    const bool generic_is_aot = identity_method.unwrap()->invoker_type == metadata::RtInvokerType::Aot;
    const char* methods[] = {"Arithmetic",
                             "Calls",
                             "Properties",
                             "ArrayLoop",
                             "Interfaces",
                             "Structs",
                             "Generics",
                             "Exceptions",
                             "VoidReturns",
                             "ReferenceReturns",
                             "InterfacesAlternating",
                             "InterfacesGeneric",
                             "InterfacesStraightLine",
                             "InterfacesThreeTypes",
                             "InterfacesPhaseChange",
                             "InterfacesManySites",
                             "UInt32Returns",
                             "Int32Returns",
                             "GenericEhInt",
                             "GenericEhString",
                             "WideLong",
                             "WideDouble",
                             "WideStruct",
                             "SharingString",
                             "SharingObject",
                             "SharingBox",
                             "SharingArray"};
    const char* workloads[] = {"arithmetic",
                               "calls",
                               "properties",
                               "array",
                               "interfaces",
                               "structs",
                               "generics",
                               "exceptions",
                               "void-returns",
                               "reference-returns",
                               "interfaces-alternating",
                               "interfaces-generic",
                               "interfaces-straight-line",
                               "interfaces-three-types",
                               "interfaces-phase-change",
                               "interfaces-many-sites",
                               "uint32-returns",
                               "int32-returns",
                               "generic-eh-int",
                               "generic-eh-string",
                               "wide-long",
                               "wide-double",
                               "wide-struct",
                               "sharing-string",
                               "sharing-object",
                               "sharing-box",
                               "sharing-array"};
    const bool sharing_mode = argc > 3 && std::strcmp(argv[3], "reference-sharing") == 0;
    const bool wide_mode = argc > 3 && std::strcmp(argv[3], "wide-returns") == 0;
    const bool generic_eh_mode = argc > 3 && std::strcmp(argv[3], "generic-eh") == 0;
    const bool integer_mode = argc > 3 && std::strcmp(argv[3], "integer-returns") == 0;
    const bool interface2_mode = argc > 3 && std::strcmp(argv[3], "interfaces2") == 0;
    const bool interface_mode = interface2_mode || (argc > 3 && std::strcmp(argv[3], "interfaces") == 0);
    if (interface_mode)
    {
        for (const char* name : {"Transform", "OtherTransform", "ThirdTransform", "FourthTransform"})
        {
            auto target_type = vm::Class::find_nested_class_by_name(klass, name, false);
            require(target_type.is_ok() && target_type.unwrap() && vm::Class::initialize_all(target_type.unwrap()).is_ok(), "Interface benchmark target type");
            const auto* target = vm::Method::find_matched_method_in_class_by_name(target_type.unwrap(), "Apply");
            require(target && target->invoker_type == metadata::RtInvokerType::Aot, "Interface benchmark target must be AOT");
        }
        auto generic_type = vm::Class::find_nested_class_by_name(klass, "GenericTransform", false);
        require(generic_type.is_ok() && generic_type.unwrap(), "Generic interface benchmark class");
        require(vm::Class::initialize_all(generic_type.unwrap()).is_ok(), "Generic interface benchmark initialization");
        const auto* definition = vm::Method::find_matched_method_in_class_by_name(generic_type.unwrap(), "Apply");
        const auto* argument = vm::Class::get_corlib_types().cls_int32->by_val;
        auto inst = metadata::MetadataCache::get_pooled_generic_inst(&argument, 1);
        require(definition && inst.is_ok(), "Generic interface benchmark instance");
        auto implementation = vm::GenericMethod::get_method(definition, nullptr, inst.unwrap());
        require(implementation.is_ok() && implementation.unwrap()->invoker_type == metadata::RtInvokerType::Aot,
                "Generic interface benchmark implementation must be AOT");
    }
    const unsigned workload_count = argc > 3 && std::strcmp(argv[3], "returns") == 0 ? 10 : 8;
    for (unsigned index = 0; index < (sharing_mode      ? 4
                                      : wide_mode       ? 3
                                      : generic_eh_mode ? 2
                                      : integer_mode    ? 12
                                      : interface_mode  ? (interface2_mode ? 7 : 4)
                                                        : workload_count);
         ++index)
    {
        const unsigned interface_indices[] = {4, 10, 11, 12, 13, 14, 15};
        const unsigned k = sharing_mode      ? 23 + index
                           : wide_mode       ? 20 + index
                           : generic_eh_mode ? 18 + index
                           : integer_mode    ? (index < 10 ? index : index + 6)
                           : interface_mode  ? interface_indices[index]
                                             : index;
        const auto* method = vm::Method::find_matched_method_in_class_by_name(klass, methods[k]);
        require(method != nullptr, "Benchmark method missing");
        const bool is_aot = method->invoker_type == metadata::RtInvokerType::Aot;
        require(is_aot || k == 7, "Non-EH benchmark must use generated AOT code");
        const metadata::RtMethodInfo* eh_loop = nullptr;
        bool eh_loop_aot = false;
        if (generic_eh_mode)
        {
            const auto* definition = vm::Method::find_matched_method_in_class_by_name(klass, "GenericEhLoop");
            const auto* argument = index == 0 ? vm::Class::get_corlib_types().cls_int32->by_val : vm::Class::get_corlib_types().cls_string->by_val;
            auto inst = metadata::MetadataCache::get_pooled_generic_inst(&argument, 1);
            require(definition && inst.is_ok(), "generic EH benchmark arguments");
            auto closed = vm::GenericMethod::get_method(definition, nullptr, inst.unwrap());
            require(closed.is_ok(), "generic EH benchmark method");
            eh_loop = closed.unwrap();
            eh_loop_aot = eh_loop->invoker_type == metadata::RtInvokerType::Aot;
            require(eh_loop->interp_data == nullptr, "generic EH must not eagerly translate IR");
        }
        uint32_t seed = 19;
        int warm_n = n / 8 + 1;
        const void* warm_args[] = {&warm_n, &seed};
        auto warm = vm::Runtime::invoke_with_run_cctor(method, nullptr, warm_args);
        require(warm.is_ok(), "Warmup failed");
        if (eh_loop_aot)
            require(eh_loop->interp_data == nullptr, "normal generic EH warmup created IR");
        if (k == 7 && is_aot)
            require(method->interp_data != nullptr, "Exception benchmark must hand off to interpreter");
        const void* args[] = {&n, &seed};
        const auto start = std::chrono::steady_clock::now();
        auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
        const auto end = std::chrono::steady_clock::now();
        require(result.is_ok() && result.unwrap(), "Benchmark invocation failed");
        if (eh_loop_aot)
            require(eh_loop->interp_data == nullptr, "normal generic EH loop created IR");
        const uint32_t checksum = *reinterpret_cast<const uint32_t*>(result.unwrap() + 1);
        const double ns = std::chrono::duration<double, std::nano>(end - start).count() / n;
        std::printf("{\"workload\":\"%s\",\"execution\":\"%s\",\"iterations\":%d,\"checksum\":\"%u\",\"ns_per_op\":%.6f}\n", workloads[k],
                    generic_eh_mode             ? (eh_loop_aot ? "aot-normal" : "aot+interpreter")
                    : k == 6 && !generic_is_aot ? "aot+interpreter"
                    : k == 7 && is_aot          ? "aot-normal/interpreter-eh"
                    : is_aot                    ? "aot"
                                                : "interpreter",
                    n, checksum, ns);
    }
    return 0;
}

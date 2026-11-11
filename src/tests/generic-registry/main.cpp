#include "../aot-returns/test_assert.h"
#include "registry_checks.h"
#include "alloc/general_allocation.h"
#include "metadata/aot_module.h"
#include "metadata/metadata_cache.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/generic_class.h"
#include "vm/generic_method.h"
#include "vm/method.h"
#include "vm/object.h"
#include "vm/runtime.h"
#include "vm/rt_exception.h"
#include "vm/settings.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <type_traits>

using namespace leanclr;
extern metadata::RtAotModulesData g_aot_modules_data;
static std::string dlls;

static void check(bool value, const char* message)
{
    if (!value)
    {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

static bool load(const char* name, const char* extension, vm::FileData& data)
{
    std::ifstream stream(dlls + "/" + name + "." + extension, std::ios::binary | std::ios::ate);
    if (!stream)
        return false;
    const auto size = stream.tellg();
    if (size < 0)
        return false;
    auto* bytes = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(static_cast<size_t>(size)));
    if (!bytes)
        return false;
    stream.seekg(0);
    if (!stream.read(reinterpret_cast<char*>(bytes), size))
    {
        alloc::GeneralAllocation::free(bytes);
        return false;
    }
    data = {bytes, static_cast<size_t>(size), false};
    return true;
}

static metadata::RtClass* type(const char* assembly_name, const char* full_name)
{
    auto assembly = vm::Assembly::load_by_name(assembly_name);
    check(assembly.is_ok(), assembly_name);
    auto klass = assembly.unwrap()->mod->get_class_by_name(full_name, false, true);
    check(klass.is_ok() && klass.unwrap(), full_name);
    check(vm::Class::initialize_all(klass.unwrap()).is_ok(), "initialize class");
    return klass.unwrap();
}

static const metadata::RtMethodInfo* inflate(metadata::RtClass* klass, const char* name, const metadata::RtTypeSig* argument)
{
    const auto* definition = vm::Method::find_matched_method_in_class_by_name(klass, name);
    check(definition != nullptr, name);
    auto instance = metadata::MetadataCache::get_pooled_generic_inst(&argument, 1);
    check(instance.is_ok(), "generic arguments");
    auto method = vm::GenericMethod::get_method(definition, nullptr, instance.unwrap());
    check(method.is_ok(), "inflate method");
    return method.unwrap();
}

static void backend(const metadata::RtMethodInfo* method, bool expected, const char* message)
{
    check((method->invoker_type == metadata::RtInvokerType::Aot) == expected, message);
    const auto* entry = metadata::AotModule::find_aot_generic_method_impl(method);
    check((entry != nullptr) == expected, "registry and bound backend disagree");
    if (entry)
    {
        check(method->method_ptr == entry->implementation.method_ptr && method->virtual_method_ptr == entry->implementation.virtual_method_ptr &&
                  method->invoke_method_ptr == entry->implementation.invoke_method_ptr && method->aot_return_protocol == entry->return_protocol,
              "single binding lost a method pointer or its return protocol");
    }
}

int main(int argc, char** argv)
{
    configure_test_assertions();
    check(argc >= 2, "DLL directory required");
    dlls = argv[1];
    vm::Settings::set_file_loader(load);
    vm::Settings::set_aot_modules_data(&g_aot_modules_data);
    check(vm::Runtime::initialize().is_ok(), "initialize runtime");
#ifdef GENERIC_REGISTRY_DISABLED
    const bool expected = false;
#else
    const bool expected = true;
#endif
    check((g_aot_modules_data.generic_method_entry_count > 0) == expected, "global instance table coverage");
    auto* tests = type("AotTests", "AotTests.TC_GenericAot");
    auto object = LEANCLR_NEWOBJ(tests, gc::GcAllocSite::make_internal(__FILE__, __LINE__, "formal AOT generic tests"));
    check(object.is_ok(), "allocate test instance");
    const char* cases[] = {"SignedValues",
                           "UnsignedAndFloating",
                           "NullAndReferences",
                           "ValueWithReferences",
                           "NestedValues",
                           "SameSignatureContexts",
                           "SameNamesDifferentAssemblies",
                           "DeepTypeIdentity",
                           "ThreeAssemblyContexts",
                           "GenericStatics",
                           "Constraints",
                           "InterfaceContexts",
                           "VirtualGeneric",
                           "VirtualFallback",
                           "Delegates",
                           "ReflectionRegistered",
                           "ReflectionUnregistered",
                           "Recursion",
                           "ArraysAndEnums",
                           "Boxing",
                           "ByRef",
                           "ExceptionPropagation",
                           "ExplicitFallbacks",
                           "GenericInheritance"};
    for (const char* name : cases)
    {
        const auto* method = vm::Method::find_matched_method_in_class_by_name(tests, name);
        check(method && method->invoker_type == metadata::RtInvokerType::Aot, name);
        auto result = vm::Runtime::invoke_with_run_cctor(method, object.unwrap(), nullptr);
        if (result.is_err())
        {
            std::fprintf(stderr, "Managed case failed: %s, error %u\n", name, static_cast<unsigned>(result.unwrap_err()));
            auto* exception = vm::Exception::get_current_exception();
            const auto* inner_getter = vm::Method::find_matched_method_in_class_by_name(vm::Class::get_corlib_types().cls_exception, "get_InnerException");
            for (unsigned depth = 0; exception && depth < 8; ++depth)
            {
                utils::Utf8StringBuilder message;
                vm::Exception::format_exception(exception, message);
                std::fprintf(stderr, "%.*s\n", static_cast<int>(message.length()), message.get_const_chars());
                if (!inner_getter)
                    break;
                auto inner = vm::Runtime::invoke_with_run_cctor(inner_getter, reinterpret_cast<vm::RtObject*>(exception), nullptr);
                if (inner.is_err())
                    break;
                exception = reinterpret_cast<vm::RtException*>(inner.unwrap());
            }
            return 1;
        }
        std::printf("Passed: AotTests.TC_GenericAot.%s\n", name);
    }
    auto* functions = type("Common", "GenericAotLibrary.Functions");
    check(functions->image->get_aot_module_data() == nullptr, "definition assembly must stay outside ordinary AOT roots");
    const auto& corlib = vm::Class::get_corlib_types();
    auto* identity = inflate(functions, "Identity", corlib.cls_int32->by_val);
    backend(identity, expected, "external definition AOT binding");
    backend(inflate(functions, "Identity", corlib.cls_string->by_val), expected, "reference generic binding");
    backend(inflate(functions, "Identity", type("Common", "GenericCollision.Payload")->by_val), expected, "Common identity binding");
    const auto* external = inflate(functions, "Identity", type("GenericAotArguments", "GenericCollision.Payload")->by_val);
    backend(external, expected, "third-assembly identity binding");
    const auto* missing = inflate(functions, "Identity", type("mscorlib", "System.DateTime")->by_val);
    backend(missing, false, "unregistered reflection target");
    auto nest = vm::Class::find_nested_class_by_name(tests, "RefNest`1", false);
    check(nest.is_ok() && nest.unwrap(), "deep argument class");
    const auto* deep = corlib.cls_int32->by_val;
    for (unsigned i = 0; i < 16; ++i)
    {
        auto inst = metadata::MetadataCache::get_pooled_generic_inst(&deep, 1);
        check(inst.is_ok(), "nested type arguments");
        auto closed = vm::GenericClass::get_class(vm::Class::get_type_def_gid(nest.unwrap()), inst.unwrap());
        check(closed.is_ok(), "nested type");
        deep = closed.unwrap()->by_val;
    }
    backend(inflate(functions, "Identity", deep), expected, "maximum supported type shape depth");
    auto local = vm::Class::find_nested_class_by_name(tests, "LocalReference", false);
    auto inherited = vm::Class::find_nested_class_by_name(tests, "GenericDerived`1", false);
    check(local.is_ok() && inherited.is_ok(), "generic inheritance definitions");
    const metadata::RtTypeSig* local_type = local.unwrap()->by_val;
    auto inherited_args = metadata::MetadataCache::get_pooled_generic_inst(&local_type, 1);
    check(inherited_args.is_ok(), "generic inheritance arguments");
    auto inherited_type = vm::GenericClass::get_class(vm::Class::get_type_def_gid(inherited.unwrap()), inherited_args.unwrap());
    check(inherited_type.is_ok() && vm::Class::initialize_all(inherited_type.unwrap()).is_ok(), "closed derived generic class");
    backend(vm::Method::find_matched_method_in_class_by_name(inherited_type.unwrap(), "ReadValue"), expected, "generic class virtual override binding");
    backend(inflate(functions, "WithCatch", corlib.cls_int32->by_val), false, "generic EH fallback");
    backend(inflate(functions, "Excluded", corlib.cls_int32->by_val), false, "attribute exclusion");
    auto derived = vm::Class::find_nested_class_by_name(tests, "Derived", false);
    check(derived.is_ok() && vm::Class::initialize_all(derived.unwrap()).is_ok(), "derived generic method class");
    backend(inflate(derived.unwrap(), "Echo", corlib.cls_int32->by_val), expected, "generic virtual override");
    backend(inflate(derived.unwrap(), "Dynamic", corlib.cls_int64->by_val), false, "unanchored virtual override fallback");
    const metadata::RtTypeSig* int_argument = corlib.cls_int32->by_val;
    auto args = metadata::MetadataCache::get_pooled_generic_inst(&int_argument, 1);
    auto* box_definition = type("Common", "GenericAotLibrary.Box`1");
    check(args.is_ok(), "Box arguments");
    auto box = vm::GenericClass::get_class(vm::Class::get_type_def_gid(box_definition), args.unwrap());
    check(box.is_ok() && vm::Class::initialize_all(box.unwrap()).is_ok(), "closed Box class");
    backend(vm::Method::find_matched_method_in_class_by_name(box.unwrap(), ".cctor"), expected, "field-only generic cctor binding");
    check(inflate(functions, "Identity", corlib.cls_int32->by_val) == identity, "pooled method cache must preserve bound identity");
    std::printf("Global generic registry: %u entries; entry size %zu; method size %zu; identity size %zu; module size %zu; global size %zu; pointer size %zu\n",
                g_aot_modules_data.generic_method_entry_count, sizeof(metadata::RtAotGenericMethodData), sizeof(metadata::RtMethodInfo),
                sizeof(metadata::RtAotTypeIdentity), sizeof(metadata::RtAotModuleData), sizeof(metadata::RtAotModulesData), sizeof(void*));
    verify_generic_registry(identity, external, missing, argc > 2 && std::strcmp(argv[2], "benchmark") == 0,
                            argc > 3 ? static_cast<unsigned>(std::atoi(argv[3])) : 20000);
    std::puts("Formal managed AOT generic tests passed: 24");
}

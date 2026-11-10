#include "alloc/general_allocation.h"
#include "metadata/metadata_cache.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/generic_method.h"
#include "vm/generic_class.h"
#include "vm/method.h"
#include "vm/runtime.h"
#include "vm/settings.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

using namespace leanclr;
extern metadata::RtAotModulesData g_aot_modules_data;
static std::string dlls;

static void check(bool ok, const char* message)
{
    if (!ok)
    {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

static bool load(const char* name, const char* ext, vm::FileData& data)
{
    std::ifstream file(dlls + "/" + name + "." + ext, std::ios::binary | std::ios::ate);
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

static const metadata::RtMethodInfo* inflate(metadata::RtClass* klass, const char* name, const metadata::RtTypeSig* arg)
{
    auto* def = vm::Method::find_matched_method_in_class_by_name(klass, name);
    check(def != nullptr, name);
    auto inst = metadata::MetadataCache::get_pooled_generic_inst(&arg, 1);
    check(inst.is_ok(), "pool generic arguments");
    auto method = vm::GenericMethod::get_method(def, nullptr, inst.unwrap());
    check(method.is_ok(), "inflate method");
    return method.unwrap();
}

static int32_t invoke_int(const metadata::RtMethodInfo* method, const void** args)
{
    auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
    check(result.is_ok() && result.unwrap(), "invoke closed generic method");
    return *reinterpret_cast<const int32_t*>(result.unwrap() + 1);
}

int main(int argc, char** argv)
{
    check(argc >= 2, "DLL directory required");
    dlls = argv[1];
    const bool expected_aot = argc < 3 || std::strcmp(argv[2], "interpreted") != 0;
    vm::Settings::set_file_loader(load);
    vm::Settings::set_aot_modules_data(&g_aot_modules_data);
    check(vm::Runtime::initialize().is_ok(), "initialize runtime");
    auto assembly = vm::Assembly::load_by_name("GenericFixtures");
    check(assembly.is_ok(), "load fixtures");
    auto type = assembly.unwrap()->mod->get_class_by_name("GenericFixtures", false, false);
    check(type.is_ok() && type.unwrap(), "fixture class");
    auto* klass = type.unwrap();
    check(vm::Class::initialize_all(klass).is_ok(), "initialize fixture type");
    auto* run = vm::Method::find_matched_method_in_class_by_name(klass, "Run");
    const int code = invoke_int(run, nullptr);
    if (code)
        std::fprintf(stderr, "Fixture failure code: %d\n", code);
    check(code == 0, "managed fixture behavior");
    const auto& corlib = vm::Class::get_corlib_types();
    auto* identity = inflate(klass, "Identity", corlib.cls_int32->by_val);
    check((identity->invoker_type == metadata::RtInvokerType::Aot) == expected_aot, "identity invocation must use expected backend");
    int32_t value = 73;
    const void* args[] = {&value};
    check(invoke_int(identity, args) == 73, "reflective/interop identity invocation");
    auto* no_int = inflate(klass, "NoArguments", corlib.cls_int32->by_val);
    auto* no_string = inflate(klass, "NoArguments", corlib.cls_string->by_val);
    check((no_int->invoker_type == metadata::RtInvokerType::Aot) == expected_aot, "no-args registration");
    check(invoke_int(no_int, nullptr) == 11 && invoke_int(no_string, nullptr) == 22, "distinct same-signature generic instances");
    auto* absent = inflate(klass, "Identity", corlib.cls_double->by_val);
    check(absent->invoker_type != metadata::RtInvokerType::Aot, "unregistered instance must not borrow another specialization");
    auto holder_def = assembly.unwrap()->mod->get_class_by_name("GenericHolder`1", false, false);
    check(holder_def.is_ok() && holder_def.unwrap(), "generic class definition");
    const metadata::RtTypeSig* holder_arg = corlib.cls_int32->by_val;
    auto holder_inst = metadata::MetadataCache::get_pooled_generic_inst(&holder_arg, 1);
    check(holder_inst.is_ok(), "generic class arguments");
    auto holder = vm::GenericClass::get_class(vm::Class::get_type_def_gid(holder_def.unwrap()), holder_inst.unwrap());
    check(holder.is_ok() && vm::Class::initialize_all(holder.unwrap()).is_ok(), "closed generic class");
    auto* forward = vm::Method::find_matched_method_in_class_by_name(holder.unwrap(), "Forward");
    check(forward && ((forward->invoker_type == metadata::RtInvokerType::Aot) == expected_aot), "generic class method registration");
    check(invoke_int(forward, args) == 73, "dynamic invocation on generic class");
    auto library = vm::Assembly::load_by_name("GenericLibrary");
    check(library.is_ok(), "load generic library");
    auto library_type = library.unwrap()->mod->get_class_by_name("GenericLibrary", false, false);
    check(library_type.is_ok() && vm::Class::initialize_all(library_type.unwrap()).is_ok(), "generic library type");
    auto* library_identity = inflate(library_type.unwrap(), "Identity", corlib.cls_int32->by_val);
    check((library_identity->invoker_type == metadata::RtInvokerType::Aot) == expected_aot, "cross-module specialization registration");
    check(invoke_int(library_identity, args) == 73, "cross-module generic invocation");
    auto* divide = inflate(klass, "Divide", corlib.cls_int32->by_val);
    value = 0;
    auto error = vm::Runtime::invoke_with_run_cctor(divide, nullptr, args);
    check(error.is_err(), "AOT generic exception must propagate");
    std::puts("Generic AOT end-to-end checks passed");
}

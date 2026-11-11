#include "../aot-returns/test_assert.h"
#include "registry_checks.h"
#include "interp/machine_state.h"
#include "interp/interpreter.h"
#include "codegen/interface_dispatch.h"
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
#include <vector>

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

static void interface_cache_checks()
{
    using namespace metadata;
    static_assert(sizeof(codegen::InterfaceCallCache) == 3 * sizeof(void*), "bounded stack cache layout");
    static_assert(std::is_trivial<codegen::InterfaceCallCache>::value, "cache has no constructor/destructor or managed values");
    RtTypeSig signature = {};
    signature.ele_type = RtElementType::Class;
    RtClass contract = {}, other_contract = {}, first = {}, second = {}, absent = {};
    contract.flags = other_contract.flags = static_cast<uint32_t>(RtTypeAttribute::Interface);
    contract.by_val = other_contract.by_val = &signature;
    RtMethodInfo declaration = {}, alternate = {}, first_impl = {}, second_impl = {};
    declaration.parent = &contract;
    alternate.parent = &other_contract;
    declaration.flags = alternate.flags = static_cast<uint16_t>(RtMethodAttribute::Virtual);
    RtVirtualInvokeData first_vtable[] = {{&declaration, &first_impl}, {&alternate, &second_impl}};
    RtVirtualInvokeData second_vtable[] = {{&declaration, &second_impl}};
    RtInterfaceOffset offsets[] = {{&contract, 0}, {&other_contract, 1}};
    first.vtable = first_vtable;
    first.interface_vtable_offsets = offsets;
    first.interface_vtable_offset_count = 2;
    second.vtable = second_vtable;
    second.interface_vtable_offsets = offsets;
    second.interface_vtable_offset_count = 1;
    vm::RtObject receiver = {};
    codegen::InterfaceCallCache cache = {};
    const auto resolve = [&](const RtClass* klass, const RtMethodInfo* method, const RtMethodInfo* expected)
    {
        receiver.klass = klass;
        auto result = codegen::get_interface_method_impl_cached(&receiver, method, cache);
        check(result.is_ok() && result.unwrap() == expected, "interface cache target identity");
    };
    resolve(&first, &declaration, &first_impl);
    resolve(&first, &declaration, &first_impl);
    resolve(&first, &alternate, &second_impl); // Same class, different declaration.
    resolve(&second, &declaration, &second_impl);
    receiver.klass = &absent;
    auto missing = codegen::get_interface_method_impl_cached(&receiver, &declaration, cache);
    check(missing.is_err() && missing.unwrap_err() == RtErr::MethodAccess, "resolution failure must propagate");
    check(cache.receiver_class == &second && cache.declaration == &declaration && cache.implementation == &second_impl,
          "failed resolution must not poison successful entry");
    resolve(&first, &declaration, &first_impl);
}

static void two_entry_cache_checks()
{
    using namespace metadata;
    static_assert(sizeof(codegen::InterfaceCallCache2) == 6 * sizeof(void*), "two entries preserve the per-method storage budget");
    static_assert(std::is_trivial<codegen::InterfaceCallCache2>::value, "two-entry cache has trivial lifetime");
    RtTypeSig signature = {};
    signature.ele_type = RtElementType::Class;
    RtClass contract = {}, other_contract = {}, first = {}, second = {}, third = {}, fourth = {};
    contract.flags = other_contract.flags = static_cast<uint32_t>(RtTypeAttribute::Interface);
    contract.by_val = other_contract.by_val = &signature;
    RtMethodInfo declaration = {}, alternate = {}, implementations[4] = {};
    declaration.parent = &contract;
    alternate.parent = &other_contract;
    declaration.flags = alternate.flags = static_cast<uint16_t>(RtMethodAttribute::Virtual);
    RtInterfaceOffset offsets[] = {{&contract, 0}, {&other_contract, 1}};
    RtVirtualInvokeData tables[4][2] = {};
    RtClass* classes[] = {&first, &second, &third, &fourth};
    for (unsigned i = 0; i < 4; ++i)
    {
        tables[i][0] = {&declaration, &implementations[i]};
        tables[i][1] = {&alternate, &implementations[3 - i]};
        classes[i]->vtable = tables[i];
        classes[i]->interface_vtable_offsets = offsets;
        classes[i]->interface_vtable_offset_count = i == 0 ? 2 : 1;
    }
    vm::RtObject receiver = {};
    codegen::InterfaceCallCache2 cache = {};
    const auto resolve = [&](unsigned index, const RtMethodInfo* method, const RtMethodInfo* expected)
    {
        receiver.klass = classes[index];
        auto result = codegen::get_interface_method_impl_cached(&receiver, method, cache);
        check(result.is_ok() && result.unwrap() == expected, "two-entry target identity");
    };
    resolve(0, &declaration, &implementations[0]);
    resolve(1, &declaration, &implementations[1]);
    resolve(0, &declaration, &implementations[0]);
    resolve(1, &declaration, &implementations[1]);
    check(cache.next_victim == 0 && cache.receiver_classes[0] == &first && cache.receiver_classes[1] == &second, "hits must not rotate replacement");
    receiver.klass = &second;
    auto missing = codegen::get_interface_method_impl_cached(&receiver, &alternate, cache);
    check(missing.is_err() && missing.unwrap_err() == RtErr::MethodAccess, "failed declaration resolution");
    check(cache.declaration == &declaration && cache.next_victim == 0 && cache.receiver_classes[0] == &first && cache.receiver_classes[1] == &second &&
              cache.implementations[0] == &implementations[0] && cache.implementations[1] == &implementations[1],
          "failure must preserve both entries and replacement cursor");
    resolve(2, &declaration, &implementations[2]);
    check(cache.next_victim == 1 && cache.receiver_classes[0] == &third && cache.receiver_classes[1] == &second, "first round-robin eviction");
    resolve(3, &declaration, &implementations[3]);
    resolve(2, &declaration, &implementations[2]);
    resolve(3, &declaration, &implementations[3]);
    check(cache.next_victim == 0 && cache.receiver_classes[0] == &third && cache.receiver_classes[1] == &fourth, "new two-class phase replaces old phase");
    resolve(0, &alternate, &implementations[3]);
    check(cache.declaration == &alternate && cache.receiver_classes[0] == &first && !cache.receiver_classes[1] && !cache.implementations[1] &&
              cache.next_victim == 1,
          "successful declaration change must invalidate both old entries");
    resolve(0, &declaration, &implementations[0]);
}

static void interface_cases(bool generic_aot)
{
    auto* tests = type("AotTests", "AotTests.TC_InterfaceDispatch");
    for (const char* name : {"Sum", "Reenter", "StraightLine", "Budget", "PhaseLoop"})
    {
        const auto* helper = vm::Method::find_matched_method_in_class_by_name(tests, name);
        check(helper && helper->invoker_type == metadata::RtInvokerType::Aot, "interface loop helper must be AOT");
    }
    for (const char* name : {"First", "Interpreted", "Generic"})
    {
        auto nested = vm::Class::find_nested_class_by_name(tests, name, false);
        check(nested.is_ok() && nested.unwrap() && vm::Class::initialize_all(nested.unwrap()).is_ok(), "interface target type");
        if (std::strcmp(name, "Generic") == 0)
        {
            for (const auto* argument : {vm::Class::get_corlib_types().cls_int32->by_val, vm::Class::get_corlib_types().cls_string->by_val})
            {
                check((inflate(tests, "GenericLoop", argument)->invoker_type == metadata::RtInvokerType::Aot) == generic_aot, "generic interface loop backend");
                check((inflate(nested.unwrap(), "Echo", argument)->invoker_type == metadata::RtInvokerType::Aot) == generic_aot,
                      "generic interface target backend");
            }
        }
        else
        {
            const auto expected = std::strcmp(name, "First") == 0 ? metadata::RtInvokerType::Aot : metadata::RtInvokerType::Interpreter;
            for (const char* member : {"Read", "Add", "Reference"})
            {
                const auto* implementation = vm::Method::find_matched_method_in_class_by_name(nested.unwrap(), member);
                check(implementation && implementation->invoker_type == expected, "ordinary interface target backend");
            }
        }
    }
    auto object = LEANCLR_NEWOBJ(tests, gc::GcAllocSite::make_internal(__FILE__, __LINE__, "formal interface dispatch tests"));
    check(object.is_ok(), "allocate interface test instance");
    const char* cases[] = {"Monomorphic",     "Polymorphic", "ThreeReceiverClasses", "ReceiverPhaseChange", "InterpreterTargets", "VoidAndReferences",
                           "GenericContexts", "Variance",    "BoxedReceiver",        "WarmTargetThrows",    "WarmTargetThenNull", "SecondaryTargetThrows",
                           "RecursiveFrames", "CacheBudget"};
    for (const char* name : cases)
    {
        const auto* method = vm::Method::find_matched_method_in_class_by_name(tests, name);
        check(method && method->invoker_type == metadata::RtInvokerType::Aot, name);
        auto result = vm::Runtime::invoke_with_run_cctor(method, object.unwrap(), nullptr);
        if (result.is_err())
        {
            std::fprintf(stderr, "Interface case failed: %s, error %u\n", name, static_cast<unsigned>(result.unwrap_err()));
            auto* exception = vm::Exception::get_current_exception();
            if (exception)
            {
                utils::Utf8StringBuilder message;
                vm::Exception::format_exception(exception, message);
                std::fprintf(stderr, "%.*s\n", static_cast<int>(message.length()), message.get_const_chars());
            }
            std::exit(1);
        }
        std::printf("Passed: AotTests.TC_InterfaceDispatch.%s\n", name);
        if (std::strcmp(name, "WarmTargetThrows") == 0 || std::strcmp(name, "WarmTargetThenNull") == 0 || std::strcmp(name, "SecondaryTargetThrows") == 0)
            check(method->interp_data != nullptr, "exception after cached dispatch must hand off to interpreter");
    }
    interface_cache_checks();
    two_entry_cache_checks();
    std::puts("Formal managed AOT interface tests passed: 14");
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

static void integer_cases()
{
    auto* tests = type("AotTests", "AotTests.TC_Int32Returns");
    auto object = LEANCLR_NEWOBJ(tests, gc::GcAllocSite::make_internal(__FILE__, __LINE__, "integer return tests"));
    check(object.is_ok(), "allocate integer tests");
    for (const char* name : {"Borders", "Contexts", "DispatchAndEnums", "Errors", "RuntimeBoundaries", "OtherReturns", "Delegates"})
    {
        const auto* method = vm::Method::find_matched_method_in_class_by_name(tests, name);
        check(method && method->invoker_type == metadata::RtInvokerType::Aot, "integer test must be AOT");
        auto result = vm::Runtime::invoke_with_run_cctor(method, object.unwrap(), nullptr);
        if (result.is_err())
        {
            std::fprintf(stderr, "Integer return case failed: %s, error %u\n", name, static_cast<unsigned>(result.unwrap_err()));
            std::exit(1);
        }
        std::printf("Passed: AotTests.TC_Int32Returns.%s\n", name);
    }
    const auto* catcher = vm::Method::find_matched_method_in_class_by_name(tests, "CatchDivide");
    check(catcher && catcher->invoker_type == metadata::RtInvokerType::Aot && catcher->interp_data, "integer EH must use normal-path AOT and handoff");
    std::puts("Formal managed integer return tests passed: 7");
}

static void generic_eh_cases(bool expected)
{
    auto* functions = type("Common", "GenericAotLibrary.Eh");
    check(functions->image->get_aot_module_data() == nullptr, "generic EH definition assembly stays outside ordinary roots");
    const auto& corlib = vm::Class::get_corlib_types();
    const auto* echo = inflate(functions, "Echo", corlib.cls_int32->by_val);
    backend(echo, expected, "scalar generic EH backend");
    if (expected)
        check(echo->interp_data == nullptr, "generic EH must not eagerly translate interpreter IR");
    int32_t value = INT32_MIN, divisor = 3;
    const void* args[] = {&value, &divisor};
    const auto top = interp::MachineState::get_global_machine_state().get_frame_stack_top();
    for (unsigned step = 0; step < 3; ++step)
    {
        divisor = step == 0 ? 3 : 0;
        auto result = vm::Runtime::invoke_with_run_cctor(echo, nullptr, args);
        check(result.is_ok() && result.unwrap() && *reinterpret_cast<const int32_t*>(result.unwrap() + 1) == INT32_MIN, "generic EH scalar result");
        check(interp::MachineState::get_global_machine_state().get_frame_stack_top() == top, "generic handoff leaked frames");
        if (expected)
            check((echo->interp_data != nullptr) == (step != 0), "IR must be created only at first exception");
    }
    auto* tests = type("AotTests", "AotTests.TC_GenericEh");
    auto object = LEANCLR_NEWOBJ(tests, gc::GcAllocSite::make_internal(__FILE__, __LINE__, "generic EH tests"));
    check(object.is_ok(), "generic EH test allocation");
    const char* cases[] = {"Scalars",
                           "References",
                           "ConstructedReferences",
                           "MutatedArguments",
                           "TypedCatches",
                           "NestedFinally",
                           "FinallyReplacesReturn",
                           "Rethrow",
                           "ClassAndMethodContexts",
                           "ConstructorHandoff",
                           "VoidAndArrays",
                           "PostCatchContinuation",
                           "ConservativeFallbacks"};
    for (const char* name : cases)
    {
        const auto* method = vm::Method::find_matched_method_in_class_by_name(tests, name);
        check(method && method->invoker_type == metadata::RtInvokerType::Aot, "generic EH test wrapper backend");
        auto result = vm::Runtime::invoke_with_run_cctor(method, object.unwrap(), nullptr);
        if (result.is_err())
        {
            std::fprintf(stderr, "Generic EH case failed: %s, error %u\n", name, static_cast<unsigned>(result.unwrap_err()));
            auto* exception = vm::Exception::get_current_exception();
            if (exception)
            {
                utils::Utf8StringBuilder message;
                vm::Exception::format_exception(exception, message);
                std::fprintf(stderr, "%.*s\n", static_cast<int>(message.length()), message.get_const_chars());
            }
            std::exit(1);
        }
        check(interp::MachineState::get_global_machine_state().get_frame_stack_top() == top, "generic EH case leaked frames");
        std::printf("Passed: AotTests.TC_GenericEh.%s\n", name);
    }
    for (const char* name : {"Mutate", "TypedCatch", "FinallyThrows", "Void", "Continuation"})
    {
        const auto* method = inflate(functions, name, corlib.cls_int32->by_val);
        backend(method, expected, "safe generic EH specialization backend");
    }
    auto* foreign_value = type("GenericAotArguments", "GenericCollision.Payload");
    backend(inflate(functions, "Echo", foreign_value->by_val), false, "value snapshot must stay interpreted");
    for (const char* name : {"ByRef", "Address", "Filter", "ComplexFinally"})
        backend(inflate(functions, name, corlib.cls_int32->by_val), false, "unsafe generic EH shape must stay interpreted");
    auto* holder_definition = type("Common", "GenericAotLibrary.EhHolder`1");
    const auto* reference_type = type("GenericAotArguments", "GenericAotArguments.ReferenceValue")->by_val;
    auto class_args = metadata::MetadataCache::get_pooled_generic_inst(&reference_type, 1);
    check(class_args.is_ok(), "EH holder class arguments");
    auto holder = vm::GenericClass::get_class(vm::Class::get_type_def_gid(holder_definition), class_args.unwrap());
    check(holder.is_ok() && vm::Class::initialize_all(holder.unwrap()).is_ok(), "EH holder class metadata");
    for (const char* name : {".ctor", ".cctor"})
    {
        const auto* method = vm::Method::find_matched_method_in_class_by_name(holder.unwrap(), name);
        backend(method, expected, "generic EH constructor backend");
        if (expected)
            check(method->interp_data == nullptr, "normal generic constructors must not translate IR");
    }
    const auto* combine_definition = vm::Method::find_matched_method_in_class_by_name(holder_definition, "Combine");
    for (const auto* argument : {corlib.cls_uint32->by_val, corlib.cls_double->by_val})
    {
        auto method_args = metadata::MetadataCache::get_pooled_generic_inst(&argument, 1);
        check(method_args.is_ok(), "EH holder method arguments");
        auto method = vm::GenericMethod::get_method(combine_definition, class_args.unwrap(), method_args.unwrap());
        check(method.is_ok(), "class and method EH context");
        backend(method.unwrap(), expected, "class/method generic EH backend");
        if (expected)
            check((method.unwrap()->interp_data != nullptr) == (argument == corlib.cls_uint32->by_val), "independent specialization handoff state");
    }
    if (expected)
    {
        const size_t slots = echo->interp_data->total_arg_and_local_stack_object_size;
        std::vector<interp::RtStackObject> invalid(slots + 1);
        auto wrong_size = interp::Interpreter::resume_exception(echo, invalid.data(), invalid.size(), 0, RtErr::DivideByZero);
        check(wrong_size.is_err() && wrong_size.unwrap_err() == RtErr::InvalidOperation, "closed EH snapshot size must be checked");
        auto wrong_ip = interp::Interpreter::resume_exception(echo, invalid.data(), slots, UINT32_MAX, RtErr::DivideByZero);
        check(wrong_ip.is_err() && wrong_ip.unwrap_err() == RtErr::InvalidOperation, "closed EH resume IP must be checked");
        check(interp::MachineState::get_global_machine_state().get_frame_stack_top() == top, "invalid generic resume leaked frames");
    }
    std::puts("Formal managed generic EH tests passed: 13");
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
    backend(inflate(functions, "WithCatch", corlib.cls_int32->by_val), expected, "safe generic EH binding");
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
    interface_cases(expected);
    integer_cases();
    generic_eh_cases(expected);
}

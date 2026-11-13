#include "host/services.h"
#include "alloc/general_allocation.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/runtime.h"
#include "vm/rt_exception.h"
#include "vm/field.h"
#include "vm/settings.h"
#include "vm/pinvoke.h"
#include "vm/object.h"
#include "utils/scope_guard.h"
#include "bcl/adapter.h"
#if LEANCLR_BCL_CORECLR_NET10
#include "bcl/coreclr-net10/managed_layouts.h"
#endif
#include "vm/marshal.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"
#include "gc/garbage_collector.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>

using namespace leanclr;
#if LEANCLR_REFLECTION_AOT
extern metadata::RtAotModulesData g_aot_modules_data;
#endif

namespace
{
alloc::MemoryCallbacks original;

std::unordered_map<void*, size_t>& live_allocations()
{
    static auto* value = new std::unordered_map<void*, size_t>();
    return *value;
}

size_t allocations;
size_t attempts;
size_t fail_at;

void check_impl(bool condition, int line)
{
    if (!condition)
    {
        std::fprintf(stderr, "Native reflection check failed at line %d\n", line);
        std::abort();
    }
}

#define check(condition) check_impl((condition), __LINE__)

void* remember(void* p, size_t size)
{
    if (p)
        live_allocations()[p] = size;
    return p;
}

bool fail()
{
    ++allocations;
    return fail_at && ++attempts == fail_at;
}

void* tracked_malloc(size_t size)
{
    return fail() ? nullptr : remember(original.malloc(size), size);
}

void* tracked_calloc(size_t count, size_t size)
{
    return fail() ? nullptr : remember(original.calloc(count, size), count * size);
}

void tracked_free(void* p)
{
    live_allocations().erase(p);
    original.free(p);
}

void* tracked_aligned(size_t size, size_t alignment)
{
    return fail() ? nullptr : remember(original.aligned_malloc(size, alignment), size);
}

void tracked_aligned_free(void* p)
{
    live_allocations().erase(p);
    original.aligned_free(p);
}

void* tracked_realloc(void* p, size_t size)
{
    if (fail())
        return nullptr;
    void* next = original.realloc(p, size);
    if (next || size == 0)
        live_allocations().erase(p);
    return remember(next, size);
}

void* tracked_aligned_realloc(void* p, size_t size, size_t alignment)
{
    if (fail())
        return nullptr;
    void* next = original.aligned_realloc(p, size, alignment);
    if (next || size == 0)
        live_allocations().erase(p);
    return remember(next, size);
}

const char* dirs[2];

bool loader(const char* name, const char* extension, host::FileData& data)
{
    for (const auto* dir : dirs)
    {
        std::ifstream stream(std::string(dir) + "/" + name + "." + extension, std::ios::binary | std::ios::ate);
        if (!stream)
            continue;
        const auto size = stream.tellg();
        if (size <= 0)
            continue;
        stream.seekg(0);
        auto* bytes = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(static_cast<size_t>(size)));
        if (!bytes)
            return false;
        if (!stream.read(reinterpret_cast<char*>(bytes), size))
        {
            alloc::GeneralAllocation::free(bytes);
            continue;
        }
        data = {bytes, static_cast<size_t>(size), false};
        return true;
    }
    return false;
}

RtResultVoid run()
{
#if LEANCLR_REFLECTION_AOT
    vm::Settings::set_aot_modules_data(&g_aot_modules_data);
#endif
    RET_ERR_ON_FAIL(vm::Runtime::initialize());
    const auto* file_not_found = vm::Class::get_corlib_types().cls_file_not_found_exception;
    check(file_not_found != nullptr && std::strcmp(file_not_found->namespaze, "System.IO") == 0 &&
          std::strcmp(file_not_found->name, "FileNotFoundException") == 0);
    const auto initialized_flags = file_not_found->init_flags;
    for (int i = 0; i < 2; ++i)
    {
        auto* error = vm::Exception::raise_error_as_exception(RtErr::FileNotFound, nullptr, nullptr);
        check(error != nullptr && reinterpret_cast<vm::RtObject*>(error)->klass == file_not_found);
        check(file_not_found->init_flags == initialized_flags);
        check(vm::Exception::get_and_clear_current_exception() == error);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtAssembly*, assembly, vm::Assembly::load_by_name("ReflectionInvokeTests"));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, program,
                                            assembly->mod->get_class_by_nested_full_name("ReflectionInvokeTests.Program", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(program));
#if LEANCLR_BCL_CORECLR_NET10
    const auto* qcall = vm::PInvokes::get_pinvoke("RuntimeMethodHandle_InvokeMethod");
    check(qcall && qcall->invoker);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, signature_class,
                                            vm::Assembly::get_corlib()->mod->get_class_by_name2("System", "Signature", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(signature_class));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, signature_object, LEANCLR_NEWOBJ_INTERNAL(signature_class, "ReflectionTest::signature"));
    auto* signature_slot = reinterpret_cast<vm::RtSignature*>(signature_object);
    gc::GcRoots::register_slot(&signature_object);
    auto signature_root = utils::make_scope_exit([&]() { gc::GcRoots::unregister_slot(&signature_object); });
    vm::RtObject* result = nullptr;
    vm::RtObject* target = nullptr;
    interp::RtStackObject call[5]{};
    call[0].ptr = &target;
    call[2].ptr = &signature_slot;
    call[4].ptr = &result;
    auto invoke = [&](const char* name, void** args) -> RtResultVoid
    {
        signature_slot->method = vm::Method::find_matched_method_in_class_by_name(program, name);
        if (!signature_slot->method)
            RET_ERR(RtErr::MissingMethod);
        call[1].ptr = args;
        return qcall->invoker(nullptr, nullptr, call, nullptr);
    };
    // The only live reference is inside a later raw value argument. Boxing the
    // first struct executes a .cctor that collects before that later value is boxed.
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, later_reference,
                                            LEANCLR_NEWOBJ_INTERNAL(vm::Class::get_corlib_types().cls_object, "ReflectionTest::later_reference"));
    int32_t first_value = 0;
    void* later_args[] = {&first_value, &later_reference};
    RET_ERR_ON_FAIL(invoke("LaterArgumentRoot", later_args));
    check(result == later_reference);
    int32_t value = 1;
    void* one[] = {&value};
    RET_ERR_ON_FAIL(invoke("Change", one));
    check(value == 42 && result == nullptr);
    value = 0;
    RET_ERR_ON_FAIL(invoke("Output", one));
    check(value == 43);
    vm::RtObject* reference = vm::String::create_string_from_utf8cstr("before");
    vm::RtObject* output = nullptr;
    void* two[] = {&reference, &output};
    RET_ERR_ON_FAIL(invoke("References", two));
    check(reference && output && reference->klass == vm::Class::get_corlib_types().cls_object);
    value = 1;
    reference = vm::String::create_string_from_utf8cstr("before");
    void* throwing[] = {&value, &reference};
    auto failed = invoke("Throw", throwing);
    check(failed.is_err() && value == 1 && vm::String::get_length(reinterpret_cast<vm::RtString*>(reference)) == 6);
    vm::Exception::set_current_exception(nullptr);
    void* invalid[] = {nullptr};
    auto invalid_result = invoke("Change", invalid);
    check(invalid_result.is_err() && invalid_result.unwrap_err() == RtErr::ArgumentNull);
    // Nullables are not boxed as Nullable<T>: materialization and copy-back must
    // honor both null and underlying-value boxes without losing the typed layout.
    auto* nullable_method = vm::Method::find_matched_method_in_class_by_name(program, "Nullable");
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, nullable_class, vm::Class::get_class_from_typesig(nullable_method->parameters[0]));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(nullable_class));
    uint8_t n1[32]{};
    uint8_t n2[32]{};
    n1[vm::Field::get_nullable_has_value_field(nullable_class)->offset] = 1;
    int32_t initial = 1;
    std::memcpy(n1 + vm::Field::get_nullable_value_field(nullable_class)->offset, &initial, 4);
    void* nullable_args[] = {n1, n2};
    RET_ERR_ON_FAIL(invoke("Nullable", nullable_args));
    int32_t nullable_value;
    std::memcpy(&nullable_value, n2 + vm::Field::get_nullable_value_field(nullable_class)->offset, 4);
    check(n1[vm::Field::get_nullable_has_value_field(nullable_class)->offset] == 0 &&
          n2[vm::Field::get_nullable_has_value_field(nullable_class)->offset] == 1 && nullable_value == 44);
#endif
    // The stackobject API consumes an explicit this argument, even for .ctor.
    // It must never silently allocate a replacement instance.
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, existing,
                                            assembly->mod->get_class_by_nested_full_name("ReflectionInvokeTests.Existing", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(existing));
    const auto* constructor = vm::Class::get_method_for_name(existing, ".ctor", 0, false);
    check(constructor != nullptr);
#if LEANCLR_REFLECTION_AOT
    check(constructor->invoker_type == metadata::RtInvokerType::Aot);
#endif
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, counters,
                                            assembly->mod->get_class_by_nested_full_name("ReflectionInvokeTests.ConstructorCounters", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(counters));
    const auto* calls_field = vm::Class::get_field_for_name(counters, "Calls", false);
    const auto* initializers_field = vm::Class::get_field_for_name(counters, "Initializers", false);
    const auto* last_field = vm::Class::get_field_for_name(counters, "Last", false);
    const auto* value_field = vm::Class::get_field_for_name(existing, "Value", false);
    int32_t initializers = -1;
    RET_ERR_ON_FAIL(vm::Field::get_static_value(initializers_field, &initializers));
    check(initializers == 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, instance, LEANCLR_NEWOBJ_INTERNAL(existing, "ReflectionTest::existing"));
    gc::GcRoots::register_slot(&instance);
    auto instance_root = utils::make_scope_exit([&]() { gc::GcRoots::unregister_slot(&instance); });
    interp::RtStackObject ctor_args[2]{};
    ctor_args[0].obj = instance;
    interp::RtStackObject ctor_return{};
    ctor_return.i64 = 0x12345678;
    RET_ERR_ON_FAIL(vm::Runtime::invoke_stackobject_arguments_with_run_cctor(constructor, ctor_args, &ctor_return));
    int32_t calls = 0;
    int32_t field_value = 0;
    vm::RtObject* last = nullptr;
    RET_ERR_ON_FAIL(vm::Field::get_instance_value(value_field, instance, &field_value));
    RET_ERR_ON_FAIL(vm::Field::get_static_value(calls_field, &calls));
    RET_ERR_ON_FAIL(vm::Field::get_static_value(initializers_field, &initializers));
    RET_ERR_ON_FAIL(vm::Field::get_static_value(last_field, &last));
    check(field_value == 42 && calls == 1 && initializers == 1 && last == instance && ctor_return.i64 == 0x12345678);
    RET_ERR_ON_FAIL(vm::Runtime::invoke_stackobject_arguments_without_run_cctor(constructor, ctor_args, &ctor_return));
    RET_ERR_ON_FAIL(vm::Field::get_static_value(calls_field, &calls));
    check(calls == 2);
    // Object invocation likewise uses an existing target, and only a null target
    // selects allocation semantics.
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, existing_result,
                                            vm::Runtime::invoke_object_arguments_with_run_cctor(constructor, instance, nullptr, 0));
    check(existing_result == nullptr);
    RET_ERR_ON_FAIL(vm::Field::get_static_value(last_field, &last));
    check(last == instance);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, allocated_result,
                                            vm::Runtime::invoke_object_arguments_with_run_cctor(constructor, nullptr, nullptr, 0));
    RET_ERR_ON_FAIL(vm::Field::get_static_value(last_field, &last));
    check(allocated_result && allocated_result != instance && last == allocated_result);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, value_class,
                                            assembly->mod->get_class_by_nested_full_name("ReflectionInvokeTests.ExistingValue", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(value_class));
    const auto* value_constructor = vm::Class::get_method_for_name(value_class, ".ctor", 1, false);
    check(value_constructor != nullptr);
#if LEANCLR_REFLECTION_AOT
    check(value_constructor->invoker_type == metadata::RtInvokerType::Aot);
#endif
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, boxed_value, LEANCLR_NEWOBJ_INTERNAL(value_class, "ReflectionTest::value"));
    gc::GcRoots::register_slot(&boxed_value);
    auto value_root = utils::make_scope_exit([&]() { gc::GcRoots::unregister_slot(&boxed_value); });
    ctor_args[0].ptr = boxed_value + 1;
    ctor_args[1].i32 = 99;
    RET_ERR_ON_FAIL(vm::Runtime::invoke_stackobject_arguments_with_run_cctor(value_constructor, ctor_args, &ctor_return));
    std::memcpy(&field_value, boxed_value + 1, sizeof(field_value));
    check(field_value == 99 && boxed_value->klass == value_class);
    RET_ERR_ON_FAIL(vm::Field::get_static_value(calls_field, &calls));
    RET_ERR_ON_FAIL(vm::Field::get_static_value(initializers_field, &initializers));
    check(calls == 5 && initializers == 1);
    std::puts("Reflection native invocation contracts passed");
    RET_VOID_OK();
}
} // namespace

// Full-runtime BCL globals may allocate before main. Install test callbacks before
// user global initializers; keep the bookkeeping map lazy to avoid init-order use.
namespace
{
struct InstallCallbacks
{
    InstallCallbacks()
    {
        original = alloc::GeneralAllocation::get_memory_callbacks();
        alloc::MemoryCallbacks callbacks{tracked_malloc, tracked_aligned, tracked_free,           tracked_aligned_free,
                                         tracked_calloc, tracked_realloc, tracked_aligned_realloc};
        check(alloc::GeneralAllocation::set_memory_callbacks(callbacks));
    }
};
#if defined(_MSC_VER)
#pragma init_seg(lib)
InstallCallbacks install_callbacks;

#else
InstallCallbacks install_callbacks __attribute__((init_priority(101)));
#endif
} // namespace

int main(int argc, char** argv)
{
    if (argc != 3)
        return 2;
    dirs[0] = argv[1];
    dirs[1] = argv[2];
    if (!host::Services::set_file_loader(loader))
        leanclr::panic("Configure file loader before host services are used");
    auto result = run();
    if (result.is_err())
    {
        std::fprintf(stderr, "Reflection test failed: %d\n", static_cast<int>(result.unwrap_err()));
        auto* error = reinterpret_cast<vm::RtObject*>(vm::Exception::get_current_exception());
        for (int i = 0; error && i < 8; ++i)
        {
            utils::Utf8StringBuilder diagnostic;
            vm::Exception::format_exception(reinterpret_cast<vm::RtException*>(error), diagnostic);
            std::fprintf(stderr, "%s\n", diagnostic.get_const_chars());
            const auto* inner = vm::Class::get_field_for_name(error->klass, "_innerException", true);
            error = inner ? *reinterpret_cast<vm::RtObject**>(reinterpret_cast<uint8_t*>(error) + vm::RT_OBJECT_HEADER_SIZE + inner->offset) : nullptr;
        }
        return 1;
    }
    vm::Runtime::shutdown();
    return 0;
}

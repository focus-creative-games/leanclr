#include "host/services.h"
#include "vm/calli.h"
#include "interp/machine_state.h"
#include "metadata/metadata_compare.h"
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
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>

using namespace leanclr;
extern "C" int32_t calli_bench_add(int32_t);
extern "C" void calli_fixture_set_callback(void (*callback)(void));
extern "C" int32_t calli_fixture_effect_count(void);
extern "C" void* calli_test_get_function(int32_t index);
#if LEANCLR_CALLI_BRIDGES
extern const metadata::RtCalliBridge g_calli_bridges[];
extern const uint32_t g_calli_bridge_count;
#endif

#if LEANCLR_CALLI_AOT
extern metadata::RtAotModulesData g_aot_modules_data;
#endif
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
        std::fprintf(stderr, "Native calli check failed at line %d\n", line);
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

bool benchmark_requested = false;
const metadata::RtMethodInfo* collect_entry = nullptr;
RtErr callback_error = RtErr::None;

void collect_callback()
{
    interp::RtStackObject result{};
    auto status = vm::Runtime::invoke_stackobject_arguments_with_run_cctor(collect_entry, nullptr, &result);
    if (status.is_err())
        callback_error = status.unwrap_err();
}

RtResultVoid named_add_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* args, interp::RtStackObject* ret)
{
    ret->i32 = calli_bench_add(args[0].i32);
    RET_VOID_OK();
}

RtResultVoid benchmark(metadata::RtClass* program)
{
    using Clock = std::chrono::steady_clock;
    const auto* calli = vm::Class::get_method_for_name(program, "BenchCalli", 1, false);
    const auto* named = vm::Class::get_method_for_name(program, "BenchNamed", 1, false);
#if LEANCLR_CALLI_AOT
    check(calli->invoker_type == metadata::RtInvokerType::Aot && named->invoker_type == metadata::RtInvokerType::Aot);
#endif
    interp::RtStackObject argument{}, ret{};
    argument.i32 = 1;
    auto start = Clock::now();
    RET_ERR_ON_FAIL(vm::Runtime::invoke_stackobject_arguments_with_run_cctor(calli, &argument, &ret));
    const auto cold_ns = std::chrono::duration<double, std::nano>(Clock::now() - start).count();
    argument.i32 = 1000;
    RET_ERR_ON_FAIL(vm::Runtime::invoke_stackobject_arguments_with_run_cctor(calli, &argument, &ret));
    RET_ERR_ON_FAIL(vm::Runtime::invoke_stackobject_arguments_with_run_cctor(named, &argument, &ret));
    const auto before = allocations;
    argument.i32 = 100000;
    double calli_samples[5], named_samples[5];
    for (int i = 0; i < 5; ++i)
    {
        start = Clock::now();
        RET_ERR_ON_FAIL(vm::Runtime::invoke_stackobject_arguments_with_run_cctor(calli, &argument, &ret));
        calli_samples[i] = std::chrono::duration<double, std::nano>(Clock::now() - start).count() / argument.i32;
        const int32_t sum = ret.i32;
        start = Clock::now();
        RET_ERR_ON_FAIL(vm::Runtime::invoke_stackobject_arguments_with_run_cctor(named, &argument, &ret));
        named_samples[i] = std::chrono::duration<double, std::nano>(Clock::now() - start).count() / argument.i32;
        check(sum == ret.i32 && sum == 12842320);
    }
    check(before == allocations);
    std::sort(calli_samples, calli_samples + 5);
    std::sort(named_samples, named_samples + 5);
    std::printf("CALLI_BENCH {\"aot\":%s,\"cold_one_call_ns\":%.0f,\"calli_ns\":%.2f,\"named_pinvoke_ns\":%.2f,\"warm_native_allocations\":0}\n",
#if LEANCLR_CALLI_AOT
                "true",
#else
                "false",
#endif
                cold_ns, calli_samples[2], named_samples[2]);
    RET_VOID_OK();
}

RtResultVoid run()
{
#if LEANCLR_CALLI_AOT
    vm::Settings::set_aot_modules_data(&g_aot_modules_data);
#endif
    RET_ERR_ON_FAIL(vm::Runtime::initialize());
    vm::PInvokes::register_pinvoke("[CalliTests]CalliTests.Program::NamedAdd", reinterpret_cast<vm::PInvokeFunction>(calli_bench_add), named_add_invoker);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtAssembly*, assembly, vm::Assembly::load_by_name("CalliTests"));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, program, assembly->mod->get_class_by_nested_full_name("CalliTests.Program", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(program));
    auto* method = vm::Class::get_method_for_name(program, "Add", 1, false);
    check(method != nullptr);
    metadata::RtMethodSig signature{};
    signature.return_type = vm::Class::get_corlib_types().cls_int32->by_val;
    signature.params.push_back(signature.return_type);
    auto unknown = vm::Calli::validate_managed_target(&signature, reinterpret_cast<const metadata::RtMethodInfo*>(uintptr_t(1)));
    check(unknown.is_err() && unknown.unwrap_err() == RtErr::Argument);
    RET_ERR_ON_FAIL(vm::Calli::register_managed_target(method));
    RET_ERR_ON_FAIL(vm::Calli::validate_managed_target(&signature, method));
    const auto before = allocations;
    for (int i = 0; i < 10000; ++i)
    {
        RET_ERR_ON_FAIL(vm::Calli::register_managed_target(method));
        RET_ERR_ON_FAIL(vm::Calli::validate_managed_target(&signature, method));
    }
    check(before == allocations);
    signature.return_type = vm::Class::get_corlib_types().cls_single->by_val;
    check(vm::Calli::validate_managed_target(&signature, method).is_err());
    signature.return_type = vm::Class::get_corlib_types().cls_int32->by_val;
    signature.params[0] = vm::Class::get_corlib_types().cls_single->by_val;
    check(vm::Calli::validate_managed_target(&signature, method).is_err());
    metadata::RtMethodSig other{};
    other.return_type = signature.return_type;
    check(!metadata::MetadataCompare::is_method_signature_equal(&signature, &other));
    check(!metadata::MetadataCompare::is_method_signature_equal(&other, &signature));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, signatures,
                                            assembly->mod->get_class_by_nested_full_name("CalliTests.NativeSignatures", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_fields(signatures));
    const auto* cdecl_signature = vm::Class::get_field_for_name(signatures, "Cdecl", false)->type_sig->data.method_sig;
    const auto* stdcall_signature = vm::Class::get_field_for_name(signatures, "Stdcall", false)->type_sig->data.method_sig;
    check(metadata::RtMetadata::decode_sig_type(cdecl_signature->flags) == metadata::RtSigType::C);
    check(metadata::RtMetadata::decode_sig_type(stdcall_signature->flags) == metadata::RtSigType::StdCall);
#if LEANCLR_BCL_CORECLR_NET10
    const auto* suppressed = vm::Class::get_field_for_name(signatures, "Suppressed", false)->type_sig->data.method_sig;
    check(metadata::RtMetadata::decode_sig_type(suppressed->flags) == metadata::RtSigType::Unmanaged);
    check(suppressed->native_call_conv == static_cast<uint8_t>(metadata::RtSigType::C) && suppressed->calli_flags == 2 &&
          suppressed->call_conv_modifier_count == 2);
#endif
#if LEANCLR_CALLI_BRIDGES
    RET_ERR_ON_FAIL(vm::Calli::register_native_bridges(g_calli_bridges, g_calli_bridge_count));
#endif
    // Host registrations use complete ABI shapes and are reusable across modules.
    static const metadata::RtCalliAbiType abi_i4{8, sizeof(int32_t), alignof(int32_t), nullptr, 0};
    static const metadata::RtCalliAbiType* abi_args[]{&abi_i4};
    static const metadata::RtCalliBridge bridge{1, &abi_i4, abi_args, 1, [](void*, interp::RtStackObject*, interp::RtStackObject*) {}, "host int(int)"};
    RET_ERR_ON_FAIL(vm::Calli::register_native_bridges(&bridge, 1));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtCalliBridge*, bound, vm::Calli::bind_native(cdecl_signature));
    check(bound != nullptr);
    const auto bridge_allocations = allocations;
    for (int i = 0; i < 10000; ++i)
    {
        RET_ERR_ON_FAIL(vm::Calli::register_native_bridges(&bridge, 1));
        auto repeated = vm::Calli::bind_native(cdecl_signature);
        check(repeated.is_ok() && repeated.unwrap() == bound);
    }
    check(allocations == bridge_allocations);
    // An absent signature cannot alias the registered integer ABI by arity.
    metadata::RtMethodSig absent{};
    absent.flags = uint8_t(metadata::RtSigType::C);
    absent.return_type = vm::Class::get_corlib_types().cls_double->by_val;
    absent.params.push_back(vm::Class::get_corlib_types().cls_int32->by_val);
    check(vm::Calli::bind_native(&absent).is_err());
#if LEANCLR_BCL_CORECLR_NET10
    check(vm::Calli::bind_native(suppressed).is_err());
#endif
    // Native registration failure is transactional and leaves earlier bindings usable.
    static const metadata::RtCalliAbiType* many_args[64];
    static metadata::RtCalliBridge growth[65];
    for (unsigned i = 0; i < 64; ++i)
        many_args[i] = &abi_i4;
    for (unsigned i = 0; i < 65; ++i)
        growth[i] = {1, &abi_i4, many_args, i, bridge.invoke, "growth probe"};
    attempts = 0;
    fail_at = 1;
    auto growth_result = vm::Calli::register_native_bridges(growth, 65);
    fail_at = 0;
    check(growth_result.is_err() && growth_result.unwrap_err() == RtErr::OutOfMemory);
    auto retained = vm::Calli::bind_native(cdecl_signature);
    check(retained.is_ok() && retained.unwrap() == bound);
    RET_ERR_ON_FAIL(vm::Calli::register_native_bridges(growth, 65));
    static const metadata::RtCalliAbiType bad_i4{8, 8, 8, nullptr, 0};
    auto malformed = bridge;
    malformed.return_type = &bad_i4;
    check(vm::Calli::register_native_bridges(&malformed, 1).is_err());
    // A failed registry growth must leave existing entries intact and permit retry.
    auto* integer = vm::Class::get_corlib_types().cls_int32;
    RET_ERR_ON_FAIL(vm::Class::initialize_all(integer));
    bool failed_growth = false;
    for (uint16_t i = 0; i < integer->method_count; ++i)
    {
        attempts = 0;
        fail_at = 1;
        auto registered = vm::Calli::register_managed_target(integer->methods[i]);
        fail_at = 0;
        if (registered.is_err())
        {
            check(registered.unwrap_err() == RtErr::OutOfMemory);
            signature.params[0] = signature.return_type;
            RET_ERR_ON_FAIL(vm::Calli::validate_managed_target(&signature, method));
            RET_ERR_ON_FAIL(vm::Calli::register_managed_target(integer->methods[i]));
            failed_growth = true;
            break;
        }
    }
    check(failed_growth);
    for (const auto* name : {"SharedSuite", "Managed", "ManagedRef", "ManagedGeneric", "FromHandle", "GenericCaller", "StructReturn", "RefReturn", "Mixed",
                             "Collect", "StackRoot", "Exception", "BadTarget", "NullTarget", "WrongSignature"})
    {
        auto* entry = vm::Class::get_method_for_name(program, name, 0, false);
        check(entry != nullptr);
#if LEANCLR_CALLI_AOT
        check(entry->invoker_type == metadata::RtInvokerType::Aot);
#endif
        RET_ERR_ON_FAIL(vm::Runtime::invoke_array_arguments_with_run_cctor(entry, nullptr, nullptr));
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtAssembly*, il_assembly, vm::Assembly::load_by_name("CalliILTests"));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, il_program,
                                            il_assembly->mod->get_class_by_nested_full_name("CalliILTests.Program", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(il_program));
    for (const auto* name : {"Instance", "ExplicitThis", "Virtual", "ValueThis", "BoxedVirtual", "NullThis"})
    {
        auto* entry = vm::Class::get_method_for_name(il_program, name, 0, false);
        check(entry != nullptr);
#if LEANCLR_CALLI_AOT
        check(entry->invoker_type == metadata::RtInvokerType::Aot);
#endif
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, vm::Runtime::invoke_array_arguments_with_run_cctor(entry, nullptr, nullptr));
        int32_t value = 0;
        RET_ERR_ON_FAIL(vm::Object::unbox_any(result, vm::Class::get_corlib_types().cls_int32, &value, false));
        check(value == 42);
    }
#if LEANCLR_CALLI_AOT || LEANCLR_CALLI_BRIDGES
    for (int i = 0; i < 27; ++i)
    {
        char name[16];
        std::snprintf(name, sizeof(name), "N%d", i);
        void* address = calli_test_get_function(i);
        RET_ERR_ON_FAIL(vm::Field::set_static_value(vm::Class::get_field_for_name(program, name, false), &address));
    }
    collect_entry = vm::Class::get_method_for_name(program, "CollectInt", 0, false);
    calli_fixture_set_callback(collect_callback);
    const auto effects_before = calli_fixture_effect_count();
    for (const auto* name : {"Unmanaged", "NativeStructs", "NativeRejected", "NativeFailures", "NativeCollect", "NativeException", "NativeGeneric"
#if LEANCLR_PLATFORM_WIN
                             ,
                             "NativeChar"
#endif
         })
    {
        const auto* entry = vm::Class::get_method_for_name(program, name, 0, false);
#if LEANCLR_CALLI_AOT
        check(entry->invoker_type == metadata::RtInvokerType::Aot);
#endif
        const auto frames = interp::MachineState::get_global_machine_state().get_frame_stack_top();
        for (int repeat = 0; repeat < (std::strcmp(name, "NativeCollect") == 0 ? 8 : 1); ++repeat)
        {
            RET_ERR_ON_FAIL(vm::Runtime::invoke_array_arguments_with_run_cctor(entry, nullptr, nullptr));
            check(interp::MachineState::get_global_machine_state().get_frame_stack_top() == frames && interp::MethodArgumentRoots::current == nullptr &&
                  interp::IndirectArgumentRoots::current == nullptr);
        }
    }
    check(callback_error == RtErr::None && calli_fixture_effect_count() == effects_before + 1);
    if (benchmark_requested)
        RET_ERR_ON_FAIL(benchmark(program));
#endif
    std::puts("Calli signature and target contracts passed");
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
    if (argc != 3 && argc != 4)
        return 2;
    benchmark_requested = argc == 4 && std::strcmp(argv[3], "--benchmark") == 0;
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

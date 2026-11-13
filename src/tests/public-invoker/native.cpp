#include "callbacks.h"
#include "alloc/general_allocation.h"
#include "vm/runtime.h"
#include "vm/settings.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/rt_exception.h"
#include "vm/pinvoke.h"
#include "vm/internal_calls.h"
#include "vm/intrinsics.h"
#include "metadata/module_def.h"
#include <cstdio>
#include <fstream>
#include <string>
using namespace leanclr;

namespace
{
std::string runtime_dir;
alloc::MemoryCallbacks original;
size_t allocations;
bool fail_allocations;

void* count_malloc(size_t n)
{
    ++allocations;
    return original.malloc(n);
}

void* count_calloc(size_t n, size_t s)
{
    ++allocations;
    return fail_allocations ? nullptr : original.calloc(n, s);
}

void* count_realloc(void* p, size_t n)
{
    ++allocations;
    return original.realloc(p, n);
}

bool callbacks_installed;

struct InstallCallbacks
{
    InstallCallbacks()
    {
        original = alloc::GeneralAllocation::get_memory_callbacks();
        auto tracked = original;
        tracked.malloc = count_malloc;
        tracked.calloc = count_calloc;
        tracked.realloc = count_realloc;
        callbacks_installed = alloc::GeneralAllocation::set_memory_callbacks(tracked);
    }
};
#if defined(_MSC_VER)
#pragma init_seg(lib)
InstallCallbacks install_callbacks;
#else
InstallCallbacks install_callbacks __attribute__((init_priority(101)));
#endif
bool load(const char* name, const char* extension, vm::FileData& data)
{
    std::ifstream file(runtime_dir + "/" + name + "." + extension, std::ios::binary | std::ios::ate);
    if (!file)
        return false;
    auto length = file.tellg();
    file.seekg(0);
    auto* bytes = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(static_cast<size_t>(length)));
    if (!bytes)
        return false;
    if (!file.read(reinterpret_cast<char*>(bytes), length))
    {
        alloc::GeneralAllocation::free(bytes);
        return false;
    }
    data = {bytes, static_cast<size_t>(length), false};
    return true;
}

#define CHECK(x)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(x))                                                          \
        {                                                                  \
            std::fprintf(stderr, "ABI failure at %d: %s\n", __LINE__, #x); \
            return 1;                                                      \
        }                                                                  \
    } while (0)
} // namespace

int main(int argc, char** argv)
{
    CHECK(argc == 2);
    runtime_dir = argv[1];
    CHECK(callbacks_installed);
    vm::Settings::set_file_loader(load);
    CHECK(leanclr_initialize_runtime() == 0);
    const auto& types = vm::Class::get_corlib_types();
    metadata::RtMethodInfo methods[13]{};
    char names[13][40];
    const metadata::RtTypeSig* parameter = types.cls_int32->by_val;
    for (int i = 0; i < 13; ++i)
    {
        std::snprintf(names[i], sizeof(names[i]), "PublicAbi%d", i);
        methods[i].parent = types.cls_object;
        methods[i].name = names[i];
        methods[i].parameters = &parameter;
        methods[i].parameter_count = 1;
        methods[i].return_type = parameter;
        methods[i].flags = 0x10;
        methods[i].total_arg_stack_object_size = 1;
        methods[i].ret_stack_object_size = 1;
        methods[i].method_ptr = abi_method_marker;
    }
    metadata::RtInvokeMethodPointer bridges[13]{};
    for (int i = 0; i < 13; ++i)
    {
        std::string name = "System.Object::" + std::string(names[i]);
        switch (i)
        {
        case 0:
            name = "[" + std::string(types.cls_object->image->get_name_no_ext()) + "]" + name + "(System.Int32)";
            leanclr_register_pinvoke_func(name.c_str(), abi_method_marker, abi_callback);
            {
                auto registry = vm::PInvokes::get_pinvoke_by_method(&methods[i]);
                CHECK(registry.is_ok() && registry.unwrap());
                bridges[i] = registry.unwrap()->invoker;
            }
            break;
        case 2:
            leanclr_register_newobj_internal_call_func(name.c_str(), abi_callback);
            {
                auto registry = vm::InternalCalls::get_newobj_internal_call_by_method(&methods[i]);
                CHECK(registry.is_ok());
                bridges[i] = registry.unwrap();
            }
            break;
        case 3:
            leanclr_register_intrinsic_func(name.c_str(), abi_method_marker, abi_callback);
            {
                auto registry = vm::Intrinsics::get_intrinsic_by_method(&methods[i]);
                CHECK(registry.is_ok() && registry.unwrap());
                bridges[i] = registry.unwrap()->invoker;
            }
            break;
        case 4:
            leanclr_register_newobj_intrinsic_func(name.c_str(), abi_callback);
            {
                auto registry = vm::Intrinsics::get_newobj_intrinsic_by_method(&methods[i]);
                CHECK(registry.is_ok());
                bridges[i] = registry.unwrap();
            }
            break;
        default:
            leanclr_register_internal_call_func(name.c_str(), abi_method_marker, abi_callback);
            {
                if (i == 12)
                    bridges[i] = vm::InternalCalls::get_internal_call(name.c_str())->invoker;
                else
                {
                    auto registry = vm::InternalCalls::get_internal_call_by_method(&methods[i]);
                    CHECK(registry.is_ok() && registry.unwrap());
                    bridges[i] = registry.unwrap()->invoker;
                }
            }
            break;
        }
        CHECK(bridges[i]);
        methods[i].invoke_method_ptr = bridges[i];
    }
    auto* expected = vm::Exception::raise_error_as_exception(RtErr::InvalidOperation, nullptr, nullptr);
    CHECK(expected);
    abi_exception = reinterpret_cast<LeanclrException*>(expected);
    interp::RtStackObject argument{}, result{};
    argument.i32 = -12345;
    for (int i = 0; i < 12; ++i)
    {
        abi_expected_method = reinterpret_cast<const LeanclrMethodInfo*>(&methods[i]);
        auto called = vm::Runtime::invoke_stackobject_arguments_with_run_cctor(&methods[i], &argument, &result);
        CHECK(called.is_ok() && result.i32 == -12328 && !abi_failures);
        if (i < 5)
        {
            abi_raise = 1;
            auto raised = vm::Runtime::invoke_stackobject_arguments_with_run_cctor(&methods[i], &argument, &result);
            CHECK(raised.is_err() && raised.unwrap_err() == RtErr::ManagedException && vm::Exception::get_current_exception() == expected);
            abi_raise = 0;
        }
    }
    abi_expected_method = reinterpret_cast<const LeanclrMethodInfo*>(&methods[12]);
    const int before = abi_calls;
    fail_allocations = true;
    auto failed = vm::InternalCalls::get_internal_call_by_method(&methods[12]);
    fail_allocations = false;
    CHECK(failed.is_err() && failed.unwrap_err() == RtErr::OutOfMemory && abi_calls == before);
    CHECK(vm::InternalCalls::get_internal_call_by_method(&methods[12]).is_ok());
    CHECK(bridges[12](abi_method_marker, &methods[12], &argument, &result).is_ok());
    abi_expected_method = reinterpret_cast<const LeanclrMethodInfo*>(&methods[0]);
    const size_t warmed = allocations;
    for (int n = 0; n < 10000; ++n)
        CHECK(bridges[0](abi_method_marker, &methods[0], &argument, &result).is_ok());
    CHECK(allocations == warmed && !abi_failures);
    LeanclrException* output = abi_exception;
    auto* public_args = reinterpret_cast<const LeanclrStackObject*>(&argument);
    auto* public_result = reinterpret_cast<LeanclrStackObject*>(&result);
    leanclr_invoke_with_buffer(abi_expected_method, public_args, public_result, &output);
    CHECK(output == nullptr && result.i32 == -12328);
    abi_raise = 1;
    leanclr_invoke_with_buffer(abi_expected_method, public_args, public_result, &output);
    CHECK(output == abi_exception);
    abi_raise = 0;
    leanclr_invoke_with_buffer(abi_expected_method, public_args, public_result, &output);
    CHECK(output == nullptr);
    leanclr_invoke_with_buffer(abi_expected_method, public_args, public_result, nullptr);
    CHECK(leanclr_load_assembly(types.cls_object->image->get_name_no_ext(), &output) != nullptr && output == nullptr);
    output = abi_exception;
    LeanclrAssembly* assembly[1];
    CHECK(leanclr_get_assemblies(assembly, 1, &output) == 1 && output == nullptr);
    output = abi_exception;
    CHECK(leanclr_get_assemblies(nullptr, 0, &output) == 0 && output == nullptr);
    CHECK(leanclr_get_assemblies(nullptr, 1, &output) == 0 && output != nullptr);
    const int calls_before_invalid = abi_calls;
    leanclr_invoke_with_buffer(nullptr, public_args, public_result, &output);
    CHECK(output != nullptr && abi_calls == calls_before_invalid);
    leanclr_invoke_with_buffer(abi_expected_method, nullptr, public_result, &output);
    CHECK(output != nullptr && abi_calls == calls_before_invalid);
    leanclr_invoke_with_buffer(abi_expected_method, public_args, nullptr, &output);
    CHECK(output != nullptr && abi_calls == calls_before_invalid);
    leanclr_invoke_with_buffer(nullptr, nullptr, nullptr, nullptr);
    std::puts("PASS C public invoker ABI, exception handoff, OOM retry and allocation-free warm calls");
    return 0;
}

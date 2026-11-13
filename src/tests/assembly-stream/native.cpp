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
#include "bcl/coreclr-net10/assembly_load_context.h"
#include "vm/marshal.h"
#include "vm/gchandle.h"
#include "utils/scope_guard.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"
#include "gc/garbage_collector.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>

using namespace leanclr;

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
        std::fprintf(stderr, "Native assembly stream check failed at line %d\n", line);
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
    RET_ERR_ON_FAIL(vm::Runtime::initialize());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtAssembly*, assembly, vm::Assembly::load_by_name("AssemblyStreamTests"));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, program,
                                            assembly->mod->get_class_by_nested_full_name("AssemblyStreamTests.Program", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(program));
    auto invoke = [&](const char* name) -> RtResult<vm::RtObject*>
    {
        const auto* method = vm::Method::find_matched_method_in_class_by_name(program, name);
        if (!method)
            return RtErr::MissingMethod;
        return vm::Runtime::invoke_array_arguments_with_run_cctor(method, nullptr, nullptr);
    };
    RET_ERR_ON_FAIL(invoke("Symbols"));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, context_obj, invoke("GetDefaultContext"));
    const auto* context_field = vm::Class::get_field_for_name(context_obj->klass, "_nativeAssemblyLoadContext", true);
    check(context_field != nullptr);
    void* token;
    std::memcpy(&token, reinterpret_cast<uint8_t*>(context_obj) + vm::RT_OBJECT_HEADER_SIZE + context_field->offset, sizeof(token));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, data_class,
                                            assembly->mod->get_class_by_nested_full_name("AssemblyStreamTests.Data", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(data_class));
    const auto* image_method = vm::Method::find_matched_method_in_class_by_name(data_class, "Fixture");
    const auto* symbols_method = vm::Method::find_matched_method_in_class_by_name(data_class, "Symbols");
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, image, vm::Runtime::invoke_array_arguments_with_run_cctor(image_method, nullptr, nullptr));
    gc::GcRoots::register_slot(&image);
    auto image_root = utils::make_scope_exit([&]() { gc::GcRoots::unregister_slot(&image); });
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, symbols, vm::Runtime::invoke_array_arguments_with_run_cctor(symbols_method, nullptr, nullptr));
    gc::GcRoots::register_slot(&symbols);
    auto symbols_root = utils::make_scope_exit([&]() { gc::GcRoots::unregister_slot(&symbols); });
    auto* bytes = vm::Array::get_array_data_start_as<uint8_t>(reinterpret_cast<vm::RtArray*>(image));
    auto* pdb = vm::Array::get_array_data_start_as<uint8_t>(reinterpret_cast<vm::RtArray*>(symbols));
    int32_t size = vm::Array::get_array_length(reinterpret_cast<vm::RtArray*>(image));
    int32_t pdb_size = vm::Array::get_array_length(reinterpret_cast<vm::RtArray*>(symbols));
    auto* expected = vm::Assembly::find_by_name("StreamFixture");
    check(expected != nullptr && expected->mod->get_pdb_image() != nullptr);
    const auto before = live_allocations().size();
    for (int i = 0; i < 100; ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtAssembly*, result, bcl::AssemblyLoadContext::load(token, bytes, size, pdb, pdb_size));
        check(result == expected && live_allocations().size() == before);
        const uint8_t bad[3] = {1, 2, 3};
        auto invalid = bcl::AssemblyLoadContext::load(token, bad, 3, nullptr, 0);
        check(invalid.is_err() && invalid.unwrap_err() == RtErr::BadImageFormat && live_allocations().size() == before);
    }
    // Fail owned image, symbols and the loader's initial fallible pool allocations.
    for (size_t failure = 1; failure <= 5; ++failure)
    {
        attempts = 0;
        fail_at = failure;
        auto result = bcl::AssemblyLoadContext::load(token, bytes, size, pdb, pdb_size);
        fail_at = 0;
        // An optional initial pool-region allocation may recover on retry.
        if (result.is_err())
            check(result.unwrap_err() == RtErr::OutOfMemory);
        else
            check(result.unwrap() == expected);
        check(live_allocations().size() == before);
    }
    auto invalid_context = bcl::AssemblyLoadContext::load(reinterpret_cast<void*>(1), bytes, size, nullptr, 0);
    check(invalid_context.is_err() && invalid_context.unwrap_err() == RtErr::NotSupported);
    auto invalid_handle = bcl::AssemblyLoadContext::initialize(reinterpret_cast<void*>(1), true, false);
    check(invalid_handle.is_err() && invalid_handle.unwrap_err() == RtErr::Argument);
    void* unrelated = vm::GCHandle::new_handle(image, false);
    check(unrelated != nullptr);
    auto wrong_target = bcl::AssemblyLoadContext::initialize(unrelated, true, false);
    check(wrong_target.is_err() && wrong_target.unwrap_err() == RtErr::Argument && vm::GCHandle::get_target(unrelated) == image);
    vm::GCHandle::free_handle(unrelated);
    // Rejection must not retain strong handles or schedule unsupported finalizers.
    auto count_handles = []()
    {
        size_t count = 0;
        vm::GCHandle::foreach_strong_handles([](vm::RtObject*, void* context) { ++*static_cast<size_t*>(context); }, &count);
        return count;
    };
    RET_ERR_ON_FAIL(invoke("UnsupportedContexts"));
    const auto handles = count_handles();
    for (int i = 0; i < 5; ++i)
        RET_ERR_ON_FAIL(invoke("UnsupportedContexts"));
    check(count_handles() == handles);
    RET_ERR_ON_FAIL(invoke("Symbols"));
    std::puts("Assembly streams: copy ownership, OOM rollback, duplicates and context handles passed");
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
        std::fprintf(stderr, "Assembly stream test failed: %d\n", static_cast<int>(result.unwrap_err()));
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

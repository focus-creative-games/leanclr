#include "alloc/general_allocation.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/runtime.h"
#include "vm/rt_exception.h"
#include "vm/field.h"
#include "vm/settings.h"
#include "vm/struct_marshal.h"
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
        std::fprintf(stderr, "Native marshal check failed at line %d\n", line);
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

bool loader(const char* name, const char* extension, vm::FileData& data)
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
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtAssembly*, assembly, vm::Assembly::load_by_name("StructMarshalTests"));
    // Read real FieldMarshal blobs: multiple values in a payload must survive
    // subsequent reads, independently of native conversion support for that type.
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, metadata_class,
                                            assembly->mod->get_class_by_nested_full_name("StructMarshalTests.MarshalMetadata", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_fields(metadata_class));
    metadata::RtMarshalSpec spec{};
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, has_array,
                                            vm::Marshal::get_marshal_spec(vm::Class::get_field_for_name(metadata_class, "Dynamic", false), spec));
    check(has_array && spec.native_type == metadata::RtMarshalNativeType::Array);
    check(spec.array.array_element_type == metadata::RtMarshalNativeType::I2 && spec.array.param_index == metadata::RT_INVALID_PARAM_INDEX_OR_ELEMENT_COUNT &&
          spec.array.element_count == metadata::RT_INVALID_PARAM_INDEX_OR_ELEMENT_COUNT);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, has_safe, vm::Marshal::get_marshal_spec(vm::Class::get_field_for_name(metadata_class, "Safe", false), spec));
    check(has_safe && spec.native_type == metadata::RtMarshalNativeType::SafeArray);
    check(spec.safe_array.variant_type == metadata::RtMarshalVariantType::I4 && spec.safe_array.udt == nullptr);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, has_custom,
                                            vm::Marshal::get_marshal_spec(vm::Class::get_field_for_name(metadata_class, "Custom", false), spec));
    check(has_custom && spec.native_type == metadata::RtMarshalNativeType::CustomMarshaler);
    check(spec.custom_marshaler.guid.length == 0 && spec.custom_marshaler.type_name.length == 0);
    check(spec.custom_marshaler.custom_marshaler_type != nullptr);
    check(spec.custom_marshaler.cookie.length == 14 && std::memcmp(spec.custom_marshaler.cookie.data, "marshal-cookie", 14) == 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, program,
                                            assembly->mod->get_class_by_nested_full_name("StructMarshalTests.Program", false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(program));
    for (const char* name : {"All", "FailureCleanup", "Pressure"})
    {
        auto* method = vm::Method::find_matched_method_in_class_by_name(program, name);
        if (!method)
            RET_ERR(RtErr::MissingMethod);
        RET_ERR_ON_FAIL(vm::Runtime::invoke_array_arguments_with_run_cctor(method, nullptr, nullptr));
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, plain, assembly->mod->get_class_by_nested_full_name("StructMarshalTests.Plain", false, true));
    RET_ERR_ON_FAIL(vm::StructMarshal::size_of(plain));
    uint8_t managed_plain[64]{};
    uint8_t native_plain[64]{};
    const auto initial_allocations = allocations;
    for (int i = 0; i < 10000; ++i)
        RET_ERR_ON_FAIL(vm::StructMarshal::to_native_data(plain, managed_plain, native_plain, false));
    check(allocations == initial_allocations);
    // Test native ownership without managed allocations or JIT/lazy metadata noise.
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, assembly->mod->get_class_by_nested_full_name("StructMarshalTests.Text", false, true));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, size, vm::StructMarshal::size_of(klass));
    // Native null-string input creates a rooted object through the real reader.
    auto* native = static_cast<uint8_t*>(alloc::GeneralAllocation::calloc(1, static_cast<size_t>(size)));
    if (!native)
        RET_ERR(RtErr::OutOfMemory);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, vm::StructMarshal::from_native(native, klass));
    gc::GcRoots::register_slot(&obj);
    auto* text = vm::String::create_string_from_utf8cstr("owned-string");
    gc::GarbageCollector::write_barrier(reinterpret_cast<vm::RtObject**>(reinterpret_cast<uint8_t*>(obj) + vm::RT_OBJECT_HEADER_SIZE +
                                                                         vm::Class::get_field_for_name(klass, "Wide", false)->offset),
                                        text);
    auto* text2 = vm::String::create_string_from_utf8cstr("second-string");
    gc::GarbageCollector::write_barrier(reinterpret_cast<vm::RtObject**>(reinterpret_cast<uint8_t*>(obj) + vm::RT_OBJECT_HEADER_SIZE +
                                                                         vm::Class::get_field_for_name(klass, "Ansi", false)->offset),
                                        text2);
    RET_ERR_ON_FAIL(vm::StructMarshal::to_native(obj, native, false));
    const auto before = live_allocations().size();
    for (int i = 0; i < 100; ++i)
        RET_ERR_ON_FAIL(vm::StructMarshal::to_native(obj, native, true));
    check(live_allocations().size() == before);
    // Fail staging and each owned-string allocation. No leaks or mutation of old data.
    for (size_t failure = 1; failure <= 3; ++failure)
    {
        auto* saved = static_cast<uint8_t*>(std::malloc(static_cast<size_t>(size)));
        check(saved != nullptr);
        std::memcpy(saved, native, static_cast<size_t>(size));
        attempts = 0;
        fail_at = failure;
        auto result = vm::StructMarshal::to_native(obj, native, true);
        fail_at = 0;
        check(result.is_err() && result.unwrap_err() == RtErr::OutOfMemory);
        check(live_allocations().size() == before && std::memcmp(saved, native, static_cast<size_t>(size)) == 0);
        std::free(saved);
    }
    RET_ERR_ON_FAIL(vm::StructMarshal::destroy(native, klass));
    check(live_allocations().size() + 2 == before);
    gc::GcRoots::unregister_slot(&obj);
    alloc::GeneralAllocation::free(native);
    std::puts("Struct marshal: ownership and allocation-failure checks passed");
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
    vm::Settings::set_file_loader(loader);
    auto result = run();
    if (result.is_err())
    {
        std::fprintf(stderr, "Marshal test failed: %d\n", static_cast<int>(result.unwrap_err()));
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

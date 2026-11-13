#include "vm/calli.h"
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
    if (argc != 3)
        return 2;
    dirs[0] = argv[1];
    dirs[1] = argv[2];
    vm::Settings::set_file_loader(loader);
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

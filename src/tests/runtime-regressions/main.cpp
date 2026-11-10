#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <unordered_set>
#include "alloc/general_allocation.h"
#include "vm/runtime.h"
#include "vm/settings.h"
#include "vm/class.h"
#include "vm/rt_array.h"
#include "vm/gc.h"
#include "vm/gchandle.h"
#include "utils/binary_reader.h"
#include "metadata/cli_image.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/method.h"
#include "vm/rt_exception.h"
#include "vm/object.h"
#include "vm/field.h"
// Until B12, the public header needs this override for a static consumer.
#define LEANCLR_EXPORT
#define LEANCLR_IMPORT
#include "public/leanclr.h"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

using namespace leanclr;
static std::string dll_dir;
static void check(bool condition, const char* message)
{
    if (!condition) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}
static bool load_file(const char* name, const char* ext, vm::FileData& output)
{
    for (const char* sub : {"/", "/mono-4.5/"})
    {
        std::string path = dll_dir + sub + name + "." + ext;
#ifdef LEANCLR_REGRESSION_FIXTURE_DIR
        if (std::strcmp(name, "RegressionFixtures") == 0 || std::strcmp(name, "FailedModule") == 0)
            path = std::string(LEANCLR_REGRESSION_FIXTURE_DIR) + "/" + name + "." + ext;
#endif
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input) continue;
        auto length = input.tellg();
        if (length < 0) return false;
        auto* data = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(static_cast<size_t>(length)));
        if (!data) return false;
        input.seekg(0);
        if (!input.read(reinterpret_cast<char*>(data), length)) { alloc::GeneralAllocation::free(data); return false; }
        output = {data, static_cast<size_t>(length), false};
        return true;
    }
    return false;
}
static void zero_arrays()
{
    auto* element = vm::Class::get_corlib_types().cls_int32;
    const int32_t dimensions[][3] = {{0,1,2}, {1,0,2}, {1,2,0}, {0,0,0}, {INT32_MAX,INT32_MAX,0}};
    const int32_t lower[] = {-2, -3, 7};
    for (auto& dims : dimensions)
    {
        auto result = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(element, 3, dims, lower, gc::GcAllocSite::none());
        check(result.is_ok(), "empty array creation failed");
        auto* array = result.unwrap();
        check(array->length == 0, "empty array length");
        for (int i = 0; i < 3; ++i)
            check(array->bounds[i].length == dims[i] && array->bounds[i].lower_bound == lower[i], "empty array bounds");
    }
    const int32_t invalid[] = {0,-1};
    auto negative = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(element, 2, invalid, nullptr, gc::GcAllocSite::none());
    check(negative.is_err() && negative.unwrap_err() == core::RtErr::Overflow, "zero must not hide negative dimension");
    const int32_t huge[] = {INT32_MAX,2};
    auto overflow = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(element, 2, huge, nullptr, gc::GcAllocSite::none());
    check(overflow.is_err() && overflow.unwrap_err() == core::RtErr::Overflow, "array product overflow");
}
static void clone_arrays()
{
    auto* element = vm::Class::get_corlib_types().cls_int32;
    const int32_t shapes[][2] = {{2,3}, {100,30}, {0,1}};
    const int32_t lower[] = {-2,4};
    for (auto& dims : shapes)
    {
        auto source = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(element, 2, dims, lower, gc::GcAllocSite::none());
        check(source.is_ok(), "source array allocation");
        if (source.unwrap()->length) vm::Array::get_array_data_start_as<int32_t>(source.unwrap())[0] = 123;
        auto clone = LEANCLR_ARRAY_CLONE(source.unwrap(), gc::GcAllocSite::none());
        check(clone.is_ok(), "array clone");
        check(source.unwrap()->bounds != clone.unwrap()->bounds, "clone must own its bounds");
        void* keep = vm::GCHandle::new_handle(clone.unwrap(), false);
        void* weak_source = vm::GCHandle::new_weakref_handle(source.unwrap(), false);
        vm::GC::enable(); vm::GC::collect(0); vm::GC::disable();
        check(vm::GCHandle::get_target(weak_source) == nullptr, "source must be reclaimed");
        auto* array = reinterpret_cast<vm::RtArray*>(vm::GCHandle::get_target(keep));
        for (int i = 0; i < 2; ++i)
            check(array->bounds[i].length == dims[i] && array->bounds[i].lower_bound == lower[i], "clone bounds after source collection");
        if (array->length) check(vm::Array::get_array_data_start_as<int32_t>(array)[0] == 123, "clone data");
        vm::GCHandle::free_handle(keep); vm::GCHandle::free_handle(weak_source);
    }
}
static void metadata_bounds()
{
    utils::BinaryReader empty(nullptr, 0);
    const char* text; size_t text_length;
    check(!empty.try_peek_cstring(16, text, text_length), "empty reader has no string");
    const uint8_t data[] = {1,2,3,4,5,6};
    utils::BinaryReader reader(data, sizeof(data));
    check(reader.try_advance(1), "reader advance");
    const uint32_t* ptr;
    check(!reader.try_peek_any_ptr_range(SIZE_MAX / 4 + 1, ptr), "reader count overflow");
    check(!reader.is_remaining(SIZE_MAX), "reader addition overflow");
    check(!reader.try_align_up_position(0), "invalid alignment");
    check(!reader.try_offset(PTRDIFF_MIN), "negative offset overflow");
    check(reader.try_set_position(sizeof(data)), "reader EOF position");
    uint8_t bytes[40] = {};
    uint32_t sig = 0x424A5342, version = 4;
    std::memcpy(bytes, &sig, 4); std::memcpy(bytes+12, &version, 4);
    bytes[22] = 1; bytes[32] = 'A';
    metadata::CliImage image{};
    image.set_image_data(bytes, 33); image.set_metadata_offset_length(0, 33);
    auto unterminated = image.load_streams();
    check(unterminated.is_err() && unterminated.unwrap_err() == core::RtErr::BadImageFormat, "unterminated stream name");
    bytes[32] = '#'; bytes[33] = '~';
    uint32_t offset = 0xfffffff0, length = 32;
    std::memcpy(bytes+24, &offset, 4); std::memcpy(bytes+28, &length, 4);
    image.set_image_data(bytes, sizeof(bytes)); image.set_metadata_offset_length(0, sizeof(bytes));
    auto wrapped = image.load_streams();
    check(wrapped.is_err(), "stream range wraparound");
    image.set_metadata_offset_length(1, UINT32_MAX);
    auto oversized = image.load_streams();
    check(oversized.is_err(), "metadata outside image");
    metadata::CliSection section{1,100,32,8};
    image.set_sections(&section, 1);
    check(image.rva_to_image_offset(1).value() == 32 && !image.rva_to_image_offset(9).has_value(), "RVA must refer to backed raw data");
#ifdef _WIN32
    SYSTEM_INFO info; GetSystemInfo(&info);
    auto* block = static_cast<uint8_t*>(VirtualAlloc(nullptr, info.dwPageSize * 2, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    check(block != nullptr, "guard allocation");
    DWORD previous;
    check(VirtualProtect(block + info.dwPageSize, info.dwPageSize, PAGE_NOACCESS, &previous) != 0, "guard protection");
    auto* end = block + info.dwPageSize - 33;
    std::memcpy(end, bytes, 33);
    image.set_image_data(end, 33); image.set_metadata_offset_length(0, 33);
    auto guarded = image.load_streams();
    check(guarded.is_err(), "stream name must not read beyond the image");
    VirtualFree(block, 0, MEM_RELEASE);
#endif
}
static void exported_types()
{
    auto* mod = vm::Assembly::get_corlib()->mod;
    utils::Vector<metadata::RtClass*> expected;
    auto result = mod->get_types(true, expected);
    check(result.is_ok(), "get exported types");
    for (size_t capacity : {size_t(0), size_t(2), size_t(mod->get_class_count())})
    {
        auto** buffer = new LeanclrClass*[capacity + 1];
        buffer[capacity] = reinterpret_cast<LeanclrClass*>(uintptr_t(1));
        size_t written = capacity;
        LeanclrException* error = reinterpret_cast<LeanclrException*>(uintptr_t(1));
        leanclr_get_classes(reinterpret_cast<LeanclrModuleDef*>(mod), true, buffer, written, &error);
        check(error == nullptr && written == std::min(capacity, expected.size()), "exported count must use filtered results");
        for (size_t i = 0; i < written; ++i) check(buffer[i] == reinterpret_cast<LeanclrClass*>(expected[i]), "exported type mismatch");
        check(buffer[capacity] == reinterpret_cast<LeanclrClass*>(uintptr_t(1)), "exported buffer overrun");
        delete[] buffer;
    }
}
static void failed_initializer()
{
    auto assembly = vm::Assembly::load_by_name("RegressionFixtures");
    check(assembly.is_ok(), "load initializer fixture");
    auto type = assembly.unwrap()->mod->get_class_by_name("FailedInitializer", false, false);
    check(type.is_ok() && type.unwrap(), "initializer fixture type");
    vm::RtException* first_exception = nullptr;
    for (int i = 0; i < 3; ++i)
    {
        auto result = vm::Runtime::run_class_static_constructor(type.unwrap());
        check(result.is_err() && result.unwrap_err() == core::RtErr::ManagedException, "failed cctor must stay failed");
        auto* exception = vm::Exception::get_and_clear_current_exception();
        check(exception && exception->klass == vm::Class::get_corlib_types().cls_type_initialization_exception, "wrap type initialization error");
        check(exception->inner_exception != nullptr, "preserve initializer cause");
        if (i == 0) first_exception = exception;
        else check(first_exception == exception, "reuse cached initialization error");
        vm::GC::enable(); vm::GC::collect(0); vm::GC::disable();
    }
    auto recursive = assembly.unwrap()->mod->get_class_by_name("RecursiveInitializer", false, false);
    check(recursive.is_ok() && recursive.unwrap(), "recursive initializer type");
    auto initialized = vm::Runtime::run_class_static_constructor(recursive.unwrap());
    check(initialized.is_ok(), "recursive cctor must not deadlock or fail");
}
static alloc::MemoryCallbacks saved_callbacks;
static std::unordered_set<void*> callback_allocations;
static size_t callback_allocation_count;
static void* tracked_malloc(size_t size)
{
    void* result = saved_callbacks.malloc(size);
    if (result) { callback_allocations.insert(result); ++callback_allocation_count; }
    return result;
}
static void* tracked_calloc(size_t count, size_t size)
{
    void* result = saved_callbacks.calloc(count, size);
    if (result) { callback_allocations.insert(result); ++callback_allocation_count; }
    return result;
}
static void tracked_free(void* ptr)
{
    check(callback_allocations.erase(ptr) == 1, "invocation freed memory from another allocator");
    saved_callbacks.free(ptr);
}
static void invocation_buffers()
{
    auto assembly = vm::Assembly::load_by_name("RegressionFixtures");
    check(assembly.is_ok(), "load invocation fixture");
    auto type = assembly.unwrap()->mod->get_class_by_name("InvocationFixture", false, false);
    check(type.is_ok() && type.unwrap(), "invocation fixture type");
    auto initialized = vm::Class::initialize_all(type.unwrap());
    check(initialized.is_ok(), "initialize invocation fixture");
    auto* method = vm::Method::find_matched_method_in_class_by_name(type.unwrap(), "ManyArguments");
    check(method != nullptr, "invocation fixture method");
    int32_t values[20]; const void* args[20];
    for (int i = 0; i < 20; ++i) { values[i] = i+1; args[i] = &values[i]; }
    auto warm = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
    check(warm.is_ok(), "warm invocation");
    saved_callbacks = alloc::GeneralAllocation::get_memory_callbacks();
    auto tracked = saved_callbacks;
    tracked.malloc = tracked_malloc; tracked.calloc = tracked_calloc; tracked.free = tracked_free;
    alloc::GeneralAllocation::set_memory_callbacks(tracked);
    auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
    alloc::GeneralAllocation::set_memory_callbacks(saved_callbacks);
    check(result.is_ok() && *reinterpret_cast<int64_t*>(result.unwrap()+1) == 210, "large argument and return buffers");
    check(callback_allocation_count >= 2 && callback_allocations.empty(), "invocation buffers must pair callback allocation and free");
}
static void weak_finalizers()
{
    auto assembly = vm::Assembly::load_by_name("RegressionFixtures");
    check(assembly.is_ok(), "load finalizer fixture");
    auto type = assembly.unwrap()->mod->get_class_by_name("FinalizerFixture", false, false);
    check(type.is_ok() && type.unwrap(), "finalizer fixture type");
    auto initialized = vm::Class::initialize_all(type.unwrap());
    check(initialized.is_ok(), "initialize finalizer fixture");
    auto* revive_field = vm::Class::get_field_for_name(type.unwrap(), "Revive", false);
    auto* root_field = vm::Class::get_field_for_name(type.unwrap(), "Resurrected", false);
    for (int mode = 0; mode < 4; ++mode)
    {
        bool revive = mode == 1;
        auto set = vm::Field::set_static_value(revive_field, &revive);
        check(set.is_ok(), "set resurrection mode");
        auto allocated = LEANCLR_NEWOBJ_INTERNAL(type.unwrap(), "weak-finalizers");
        check(allocated.is_ok(), "allocate finalizer fixture");
        auto* object = allocated.unwrap();
        if (mode >= 2) vm::GC::suppress_finalize(object);
        if (mode == 3) vm::GC::reregister_for_finalize(object);
        void* short_handle = vm::GCHandle::new_weakref_handle(object, false);
        void* long_handle = vm::GCHandle::new_weakref_handle(object, true);
        vm::GC::enable(); vm::GC::collect(0);
        check(vm::GCHandle::get_target(short_handle) == nullptr, "short weak target must clear before finalization");
        check((vm::GCHandle::get_target(long_handle) != nullptr) == (mode != 2), "long weak promotion or suppression");
        vm::GC::wait_for_pending_finalizers();
        vm::GC::collect(0);
        check((vm::GCHandle::get_target(long_handle) != nullptr) == revive, "long weak resurrection semantics");
        check(vm::GCHandle::get_target(short_handle) == nullptr, "short weak handle must stay cleared after resurrection");
        vm::RtObject* null_object = nullptr;
        auto clear = vm::Field::set_static_value(root_field, &null_object);
        check(clear.is_ok(), "clear resurrection root");
        vm::GC::collect(0); vm::GC::disable();
        check(vm::GCHandle::get_target(long_handle) == nullptr, "long weak target must eventually clear");
        vm::GCHandle::free_handle(short_handle); vm::GCHandle::free_handle(long_handle);
    }
}
int main(int argc, char** argv)
{
    check(argc >= 3, "usage: runtime-regressions CASE DLL_DIRECTORY");
    dll_dir = argv[2];
    vm::Settings::set_file_loader(load_file);
    const char* args[] = {"runtime-regressions"};
    vm::Settings::set_command_line_arguments(1, args);
    vm::Settings::set_gc_mode(gc::GCMode::DISABLED);
    auto initialized = vm::Runtime::initialize();
    check(initialized.is_ok(), "runtime initialization failed");
    if (std::strcmp(argv[1], "zero-arrays") == 0) zero_arrays();
    else if (std::strcmp(argv[1], "clone-arrays") == 0) clone_arrays();
    else if (std::strcmp(argv[1], "metadata-bounds") == 0) metadata_bounds();
    else if (std::strcmp(argv[1], "exported-types") == 0) exported_types();
    else if (std::strcmp(argv[1], "failed-initializer") == 0) failed_initializer();
    else if (std::strcmp(argv[1], "invocation-buffers") == 0) invocation_buffers();
    else if (std::strcmp(argv[1], "weak-finalizers") == 0) weak_finalizers();
    else if (std::strcmp(argv[1], "failed-module") == 0)
    {
        auto module = vm::Assembly::load_by_name("FailedModule");
        check(module.is_ok(), "load module fixture");
        for (int i = 0; i < 2; ++i)
        {
            auto result = vm::Runtime::run_module_static_constructor(module.unwrap()->mod);
            check(result.is_err(), "module initializer must remain failed");
            check(vm::Exception::get_and_clear_current_exception() != nullptr, "cached module exception");
        }
    }
    else check(false, "unknown regression case");
    return 0;
}

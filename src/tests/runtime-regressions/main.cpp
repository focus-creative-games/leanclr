#include "fake-host-services.h"
#include "interp/machine_state.h"
#include "bcl/mono/managed_layouts.h"
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
#include "metadata/pe_image_reader.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/method.h"
#include "vm/rt_exception.h"
#include "vm/object.h"
#include "vm/field.h"
#include "vm/rt_string.h"
#include "icalls/system_string.h"
#include "icalls/system_runtime_interopservices_gchandle.h"
#include "gc/mark_sweep/small_heap_arena.h"
#include "public/leanclr.h"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

using namespace leanclr;
#if LEANCLR_REGRESSION_AOT
extern metadata::RtAotModulesData g_aot_modules_data;
#endif
static std::string dll_dir;
static int fixture_dll_loads;
static bool reject_fixture_load;

static void check(bool condition, const char* message)
{
    if (!condition)
    {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

static bool load_file(const char* name, const char* ext, vm::FileData& output)
{
    if (std::strcmp(name, "RegressionFixtures") == 0 && std::strcmp(ext, "dll") == 0)
    {
        ++fixture_dll_loads;
        if (reject_fixture_load)
            return false;
    }
    for (const char* sub : {"/", "/mono-4.5/"})
    {
        std::string path = dll_dir + sub + name + "." + ext;
#ifdef LEANCLR_REGRESSION_FIXTURE_DIR
        if (std::strcmp(name, "RegressionFixtures") == 0 || std::strcmp(name, "FailedModule") == 0 || std::strcmp(name, "InvalidLayouts") == 0 ||
            std::strcmp(name, "PortableMemory") == 0)
            path = std::string(LEANCLR_REGRESSION_FIXTURE_DIR) + "/" + name + "." + ext;
#endif
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input)
            continue;
        auto length = input.tellg();
        if (length < 0)
            return false;
        auto* data = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(static_cast<size_t>(length)));
        if (!data)
            return false;
        input.seekg(0);
        if (!input.read(reinterpret_cast<char*>(data), length))
        {
            alloc::GeneralAllocation::free(data);
            return false;
        }
        output = {data, static_cast<size_t>(length), false};
        return true;
    }
    return false;
}

static void zero_arrays()
{
    auto* element = vm::Class::get_corlib_types().cls_int32;
    const int32_t dimensions[][3] = {{0, 1, 2}, {1, 0, 2}, {1, 2, 0}, {0, 0, 0}, {INT32_MAX, INT32_MAX, 0}};
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
    const int32_t invalid[] = {0, -1};
    auto negative = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(element, 2, invalid, nullptr, gc::GcAllocSite::none());
    check(negative.is_err() && negative.unwrap_err() == core::RtErr::Overflow, "zero must not hide negative dimension");
    const int32_t huge[] = {INT32_MAX, 2};
    auto overflow = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(element, 2, huge, nullptr, gc::GcAllocSite::none());
    check(overflow.is_err() && overflow.unwrap_err() == core::RtErr::Overflow, "array product overflow");
}

static void clone_arrays()
{
    auto* element = vm::Class::get_corlib_types().cls_int32;
    const int32_t shapes[][2] = {{2, 3}, {100, 30}, {0, 1}};
    const int32_t lower[] = {-2, 4};
    for (auto& dims : shapes)
    {
        auto source = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(element, 2, dims, lower, gc::GcAllocSite::none());
        check(source.is_ok(), "source array allocation");
        if (source.unwrap()->length)
            vm::Array::get_array_data_start_as<int32_t>(source.unwrap())[0] = 123;
        auto clone = LEANCLR_ARRAY_CLONE(source.unwrap(), gc::GcAllocSite::none());
        check(clone.is_ok(), "array clone");
        check(source.unwrap()->bounds != clone.unwrap()->bounds, "clone must own its bounds");
        void* keep = vm::GCHandle::new_handle(clone.unwrap(), false);
        void* weak_source = vm::GCHandle::new_weakref_handle(source.unwrap(), false);
        vm::GC::enable();
        vm::GC::collect(0);
        vm::GC::disable();
        check(vm::GCHandle::get_target(weak_source) == nullptr, "source must be reclaimed");
        auto* array = reinterpret_cast<vm::RtArray*>(vm::GCHandle::get_target(keep));
        for (int i = 0; i < 2; ++i)
            check(array->bounds[i].length == dims[i] && array->bounds[i].lower_bound == lower[i], "clone bounds after source collection");
        if (array->length)
            check(vm::Array::get_array_data_start_as<int32_t>(array)[0] == 123, "clone data");
        vm::GCHandle::free_handle(keep);
        vm::GCHandle::free_handle(weak_source);
    }
}

static void metadata_bounds()
{
    utils::BinaryReader empty(nullptr, 0);
    const char* text;
    size_t text_length;
    check(!empty.try_peek_cstring(16, text, text_length), "empty reader has no string");
    const uint8_t data[] = {1, 2, 3, 4, 5, 6};
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
    std::memcpy(bytes, &sig, 4);
    std::memcpy(bytes + 12, &version, 4);
    bytes[22] = 1;
    bytes[32] = 'A';
    metadata::CliImage image{};
    image.set_image_data(bytes, 33);
    image.set_metadata_offset_length(0, 33);
    auto unterminated = image.load_streams();
    check(unterminated.is_err() && unterminated.unwrap_err() == core::RtErr::BadImageFormat, "unterminated stream name");
    bytes[32] = '#';
    bytes[33] = '~';
    uint32_t offset = 0xfffffff0, length = 32;
    std::memcpy(bytes + 24, &offset, 4);
    std::memcpy(bytes + 28, &length, 4);
    image.set_image_data(bytes, sizeof(bytes));
    image.set_metadata_offset_length(0, sizeof(bytes));
    auto wrapped = image.load_streams();
    check(wrapped.is_err(), "stream range wraparound");
    image.set_metadata_offset_length(1, UINT32_MAX);
    auto oversized = image.load_streams();
    check(oversized.is_err(), "metadata outside image");
    metadata::CliSection section{1, 100, 32, 8};
    image.set_sections(&section, 1);
    check(image.rva_to_image_offset(1).value() == 32 && !image.rva_to_image_offset(9).has_value(), "RVA must refer to backed raw data");
#ifdef _WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    auto* block = static_cast<uint8_t*>(VirtualAlloc(nullptr, info.dwPageSize * 2, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    check(block != nullptr, "guard allocation");
    DWORD previous;
    check(VirtualProtect(block + info.dwPageSize, info.dwPageSize, PAGE_NOACCESS, &previous) != 0, "guard protection");
    auto* end = block + info.dwPageSize - 33;
    std::memcpy(end, bytes, 33);
    image.set_image_data(end, 33);
    image.set_metadata_offset_length(0, 33);
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
        for (size_t i = 0; i < written; ++i)
            check(buffer[i] == reinterpret_cast<LeanclrClass*>(expected[i]), "exported type mismatch");
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
        if (i == 0)
            first_exception = exception;
        else
            check(first_exception == exception, "reuse cached initialization error");
        vm::GC::enable();
        vm::GC::collect(0);
        vm::GC::disable();
    }
    auto recursive = assembly.unwrap()->mod->get_class_by_name("RecursiveInitializer", false, false);
    check(recursive.is_ok() && recursive.unwrap(), "recursive initializer type");
    auto initialized = vm::Runtime::run_class_static_constructor(recursive.unwrap());
    check(initialized.is_ok(), "recursive cctor must not deadlock or fail");
}

static alloc::MemoryCallbacks saved_callbacks;
static std::unordered_set<void*> callback_allocations;
static size_t callback_allocation_count;
static bool track_callback_allocations;
static int allocations_before_failure = -1;

static void* tracked_malloc(size_t size)
{
    if (allocations_before_failure == 0)
        return nullptr;
    if (allocations_before_failure > 0)
        --allocations_before_failure;
    void* result = saved_callbacks.malloc(size);
    if (result && track_callback_allocations)
    {
        callback_allocations.insert(result);
        ++callback_allocation_count;
    }
    return result;
}

static void* tracked_calloc(size_t count, size_t size)
{
    if (allocations_before_failure == 0)
        return nullptr;
    if (allocations_before_failure > 0)
        --allocations_before_failure;
    void* result = saved_callbacks.calloc(count, size);
    if (result && track_callback_allocations)
    {
        callback_allocations.insert(result);
        ++callback_allocation_count;
    }
    return result;
}

static void tracked_free(void* ptr)
{
    if (track_callback_allocations)
        check(callback_allocations.erase(ptr) == 1, "temporary buffer must be freed by its installed allocator");
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
    int32_t values[20];
    const void* args[20];
    for (int i = 0; i < 20; ++i)
    {
        values[i] = i + 1;
        args[i] = &values[i];
    }
    auto warm = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
    check(warm.is_ok(), "warm invocation");
    track_callback_allocations = true;
    callback_allocation_count = 0;
    auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
    allocations_before_failure = -1;
    track_callback_allocations = false;
    check(result.is_ok() && *reinterpret_cast<int64_t*>(result.unwrap() + 1) == 210, "large argument and return buffers");
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
        if (mode >= 2)
            vm::GC::suppress_finalize(object);
        if (mode == 3)
            vm::GC::reregister_for_finalize(object);
        void* short_handle = vm::GCHandle::new_weakref_handle(object, false);
        void* long_handle = vm::GCHandle::new_weakref_handle(object, true);
        vm::GC::enable();
        vm::GC::collect(0);
        check(vm::GCHandle::get_target(short_handle) == nullptr, "short weak target must clear before finalization");
        check((vm::GCHandle::get_target(long_handle) != nullptr) == (mode != 2), "long weak promotion or suppression");
        vm::GC::wait_for_pending_finalizers();
        vm::GC::collect(0);
        check((vm::GCHandle::get_target(long_handle) != nullptr) == revive, "long weak resurrection semantics");
        check(vm::GCHandle::get_target(short_handle) == nullptr, "short weak handle must stay cleared after resurrection");
        vm::RtObject* null_object = nullptr;
        auto clear = vm::Field::set_static_value(root_field, &null_object);
        check(clear.is_ok(), "clear resurrection root");
        vm::GC::collect(0);
        vm::GC::disable();
        check(vm::GCHandle::get_target(long_handle) == nullptr, "long weak target must eventually clear");
        vm::GCHandle::free_handle(short_handle);
        vm::GCHandle::free_handle(long_handle);
    }
}

static void allocation_failures()
{
    invocation_buffers(); // Resolve and warm all fixture metadata before injecting failure.
    auto assembly = vm::Assembly::load_by_name("RegressionFixtures");
    check(assembly.is_ok(), "load OOM fixture");
    auto type = assembly.unwrap()->mod->get_class_by_name("InvocationFixture", false, false);
    check(type.is_ok(), "OOM fixture type");
    auto* method = vm::Method::find_matched_method_in_class_by_name(type.unwrap(), "ManyArguments");
    int32_t value = 1;
    const void* args[20];
    for (auto& arg : args)
        arg = &value;
    for (int allowed = 0; allowed <= 1; ++allowed)
    {
        allocations_before_failure = allowed;
        track_callback_allocations = true;
        auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
        allocations_before_failure = -1;
        track_callback_allocations = false;
        check(result.is_err() && result.unwrap_err() == core::RtErr::OutOfMemory, "argument/return buffer OOM must propagate");
        check(callback_allocations.empty(), "partial invocation allocation must be freed");
    }
    const uint8_t invalid_image[64] = {};
    for (int allowed = 0; allowed < 5; ++allowed)
    {
        allocations_before_failure = allowed;
        track_callback_allocations = true;
        auto loaded = vm::Assembly::load_from_data({invalid_image, sizeof(invalid_image), true}, nullptr);
        allocations_before_failure = -1;
        track_callback_allocations = false;
        check(loaded.is_err() && (loaded.unwrap_err() == RtErr::OutOfMemory || loaded.unwrap_err() == RtErr::BadImageFormat),
              "assembly allocation and parse failures must return an error");
        check(callback_allocations.empty(), "assembly rollback must free partially constructed pools");
    }
    const int32_t dims[] = {100, 100};
    auto warm = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(vm::Class::get_corlib_types().cls_int32, 2, dims, nullptr, gc::GcAllocSite::none());
    check(warm.is_ok(), "warm multidimensional array class");
    utils::Vector<int> vector;
    for (int i = 0; i < 8; ++i)
        vector.push_back(i);
    allocations_before_failure = 0;
    track_callback_allocations = true;
    auto string = icalls::SystemString::fast_allocate_string(10000);
    auto array = LEANCLR_NEW_MDARRAY_FROM_ARRAY_KLASS(warm.unwrap()->klass, dims, nullptr, gc::GcAllocSite::none());
    auto handle = icalls::SystemRuntimeInteropServicesGCHandle::get_target_handle(nullptr, 0, 2);
    bool grew = vector.try_reserve(16);
    bool appended = vector.try_push_back(9);
    gc::SizeClassPool pool(16384, 256, 8);
    void* block = pool.allocate_block();
    allocations_before_failure = -1;
    track_callback_allocations = false;
    check(string.is_err() && string.unwrap_err() == core::RtErr::OutOfMemory, "string OOM");
    check(array.is_err() && array.unwrap_err() == core::RtErr::OutOfMemory, "array OOM");
    check(handle.is_err() && handle.unwrap_err() == core::RtErr::OutOfMemory, "handle OOM");
    check(!grew && !appended && vector.size() == 8 && vector[7] == 7, "vector failure must preserve contents");
    check(vector.try_push_back(vector[7]) && vector.size() == 9 && vector[8] == 7, "fallible growth must preserve an aliased element");
    check(block == nullptr, "small heap arena registry OOM");
    check(alloc::GeneralAllocation::calloc(SIZE_MAX, 2) == nullptr, "allocation multiplication overflow");
    check(vm::String::fast_allocate_string(-1) == nullptr && vm::String::fast_allocate_string(INT32_MAX) == nullptr, "invalid string allocation size");
    auto recovered = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
    check(recovered.is_ok(), "runtime must recover after failed allocation");
}

static void loader_once()
{
    reject_fixture_load = true;
    auto failed = vm::Assembly::load_by_name("RegressionFixtures");
    check(failed.is_err() && fixture_dll_loads == 1, "failed load must invoke the DLL callback once");
    reject_fixture_load = false;
    auto loaded = vm::Assembly::load_by_name("RegressionFixtures");
    check(loaded.is_ok() && fixture_dll_loads == 2, "successful load must invoke the DLL callback once");
    auto cached = vm::Assembly::load_by_name("RegressionFixtures");
    check(cached.is_ok() && cached.unwrap() == loaded.unwrap() && fixture_dll_loads == 2, "cached load must not invoke the callback");
}

static void gc_accounting()
{
    const int32_t dims[] = {100, 30};
    auto array = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(vm::Class::get_corlib_types().cls_int32, 2, dims, nullptr, gc::GcAllocSite::none());
    check(array.is_ok(), "warm accounting array class");
    vm::GC::enable();
    vm::GC::collect(0);
    vm::GC::collect(0);
    const int64_t baseline = vm::GC::get_used_size();
    for (int repeat = 0; repeat < 5; ++repeat)
    {
        auto before = vm::GC::get_allocated_bytes_for_current_thread();
        for (int length : {120, 250, 600, 1200, 2500, 5000})
            check(vm::String::fast_allocate_string(length) != nullptr, "accounting string allocation");
        auto large = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(vm::Class::get_corlib_types().cls_int32, 2, dims, nullptr, gc::GcAllocSite::none());
        check(large.is_ok(), "accounting array allocation");
        auto after_alloc = vm::GC::get_allocated_bytes_for_current_thread();
        check(after_alloc > before && vm::GC::get_used_size() > baseline, "account allocation bytes");
        vm::GC::collect(0);
        check(vm::GC::get_used_size() == baseline, "small blocks and multidimensional arrays must not drift after GC");
        check(vm::GC::get_heap_size() == baseline, "heap statistics must match heap implementation");
        check(vm::GC::get_allocated_bytes_for_current_thread() >= after_alloc, "lifetime allocation counter must not reset on GC");
    }
    gc::GcPressure::on_collect();
    gc::GcPressure::set_config({100, 0});
    check(!gc::GcPressure::should_collect(false), "configured threshold before pressure");
    gc::GcPressure::record_external(100);
    check(gc::GcPressure::should_collect(false), "external pressure threshold");
    gc::GcPressure::on_collect();
    gc::GcPressure::set_config({0, baseline});
    check(gc::GcPressure::should_collect(false), "soft heap limit");
    gc::GcPressure::set_config({0, 0});
    check(!gc::GcPressure::should_collect(false) && gc::GcPressure::should_collect(true), "disabled thresholds and force");
}

static void compressed_signed()
{
    struct Case
    {
        uint8_t bytes[4];
        size_t width;
        int32_t expected;
    };

    const Case cases[] = {{{0x06}, 1, 3},
                          {{0x7b}, 1, -3},
                          {{0x01}, 1, -64},
                          {{0x7e}, 1, 63},
                          {{0x80, 0x80}, 2, 64},
                          {{0x80, 0x01}, 2, -8192},
                          {{0xbf, 0xfe}, 2, 8191},
                          {{0xc0, 0x00, 0x40, 0x00}, 4, 8192},
                          {{0xc0, 0, 0, 1}, 4, -268435456},
                          {{0xdf, 0xff, 0xff, 0xfe}, 4, 268435455},
                          {{0xdf, 0xff, 0xff, 0xff}, 4, -1}};
    for (auto& c : cases)
    {
        utils::BinaryReader reader(c.bytes, c.width);
        int32_t result;
        check(reader.try_read_compressed_int32(result) && result == c.expected && reader.remaining() == 0, "compressed signed value and width");
        for (size_t truncated = 0; truncated < c.width; ++truncated)
        {
            utils::BinaryReader short_reader(c.bytes, truncated);
            check(!short_reader.try_read_compressed_int32(result) && short_reader.get_position() == 0, "compressed signed truncation must not consume input");
        }
    }
    const uint8_t illegal[] = {0xe0, 0, 0, 0};
    utils::BinaryReader invalid(illegal, sizeof(illegal));
    int32_t result;
    check(!invalid.try_read_compressed_int32(result), "reserved compressed integer prefix");
}

template <typename T>
static void integer_call(metadata::RtClass* klass, const char* name, T a, T b, T expected, bool shift = false)
{
    const auto* method = vm::Method::find_matched_method_in_class_by_name(klass, name);
    check(method != nullptr, "integer fixture method");
#if LEANCLR_REGRESSION_AOT
    check(method->invoker_type == metadata::RtInvokerType::Aot, "integer fixture must execute generated AOT code");
#endif
    int32_t amount = static_cast<int32_t>(b);
    const void* args[] = {&a, shift ? static_cast<const void*>(&amount) : &b};
    auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
    check(result.is_ok() && result.unwrap() != nullptr, "integer method invocation");
    check(*reinterpret_cast<T*>(result.unwrap() + 1) == expected, name);
}

static void integer_arithmetic()
{
    auto assembly = vm::Assembly::load_by_name("RegressionFixtures");
    check(assembly.is_ok(), "load integer fixture");
    auto type = assembly.unwrap()->mod->get_class_by_name("IntegerFixture", false, false);
    check(type.is_ok() && type.unwrap(), "integer fixture type");
    auto initialized = vm::Class::initialize_all(type.unwrap());
    check(initialized.is_ok(), "initialize integer fixture");
    auto* klass = type.unwrap();
    integer_call<int32_t>(klass, "Add32", INT32_MAX, 1, INT32_MIN);
    integer_call<int32_t>(klass, "Sub32", INT32_MIN, 1, INT32_MAX);
    integer_call<int32_t>(klass, "Mul32", INT32_MIN, -1, INT32_MIN);
    integer_call<int32_t>(klass, "Neg32", INT32_MIN, 0, INT32_MIN);
    integer_call<int32_t>(klass, "Shl32", -1, 31, INT32_MIN, true);
    integer_call<int32_t>(klass, "Shr32", INT32_MIN, 31, -1, true);
    integer_call<int32_t>(klass, "ShrUn32", -1, 1, INT32_MAX, true);
    integer_call<int64_t>(klass, "Add64", INT64_MAX, 1, INT64_MIN);
    integer_call<int64_t>(klass, "Sub64", INT64_MIN, 1, INT64_MAX);
    integer_call<int64_t>(klass, "Mul64", INT64_MIN, -1, INT64_MIN);
    integer_call<int64_t>(klass, "Neg64", INT64_MIN, 0, INT64_MIN);
    integer_call<int64_t>(klass, "Shl64", -1, 63, INT64_MIN, true);
    integer_call<int64_t>(klass, "Shr64", INT64_MIN, 63, -1, true);
    integer_call<int64_t>(klass, "ShrUn64", INT64_MIN, 63, 1, true);
}

static void unaligned_image()
{
    vm::FileData file;
    check(load_file("mscorlib", "dll", file), "load image bytes");
    auto* bytes = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(file.length + 8));
    check(bytes != nullptr, "allocate shifted image");
    for (size_t shift = 0; shift < 8; ++shift)
    {
        std::memcpy(bytes + shift, file.data, file.length);
        alloc::MemPool pool;
        metadata::PeImageReader reader(bytes + shift, file.length);
        auto image = reader.ReadCliImage(pool);
        check(image.is_ok(), "parse shifted PE image");
        check(image.unwrap()->load_streams().is_ok(), "parse shifted streams");
        check(image.unwrap()->load_tables(pool).is_ok(), "parse shifted tables");
        auto name = image.unwrap()->read_assembly_name_no_ext();
        check(name.is_ok() && std::strcmp(name.unwrap(), "mscorlib") == 0, "decode shifted assembly table");
    }
    alloc::GeneralAllocation::free(bytes);
    alloc::GeneralAllocation::free(const_cast<uint8_t*>(file.data));
}

static void host_integration_log(host::LogLevel level, const char* message)
{
    FakeHost::log(level, message);
    if (level == host::LogLevel::Fatal)
    {
        auto frames = interp::MachineState::get_global_machine_state().get_active_frames();
        for (size_t i = 0; i < frames.size(); ++i)
            std::fprintf(stderr, "managed frame: %s::%s\n", frames[i].method->parent->name, frames[i].method->name);
    }
}

int main(int argc, char** argv)
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    check(argc >= 3, "usage: runtime-regressions CASE DLL_DIRECTORY");
    dll_dir = argv[2];
    if (std::strcmp(argv[1], "invocation-buffers") == 0 || std::strcmp(argv[1], "allocation-failures") == 0)
    {
        saved_callbacks = alloc::GeneralAllocation::get_memory_callbacks();
        auto tracked = saved_callbacks;
        tracked.malloc = tracked_malloc;
        tracked.calloc = tracked_calloc;
        tracked.free = tracked_free;
        check(alloc::GeneralAllocation::set_memory_callbacks(tracked), "install allocator before runtime allocations");
    }
    if (std::strcmp(argv[1], "managed-host-services") == 0)
    {
        auto callbacks = FakeHost::callbacks();
        callbacks.log = host_integration_log;
        check(host::Services::configure(callbacks), "install managed integration host");
    }
    vm::Settings::set_file_loader(load_file);
    const char* args[] = {"runtime-regressions"};
    vm::Settings::set_command_line_arguments(1, args);
    vm::Settings::set_gc_mode(gc::GCMode::DISABLED);
#if LEANCLR_REGRESSION_AOT
    vm::Settings::set_aot_modules_data(&g_aot_modules_data);
#endif
    auto initialized = vm::Runtime::initialize();
    check(initialized.is_ok(), "runtime initialization failed");
    if (std::strcmp(argv[1], "zero-arrays") == 0)
        zero_arrays();
    else if (std::strcmp(argv[1], "clone-arrays") == 0)
        clone_arrays();
    else if (std::strcmp(argv[1], "metadata-bounds") == 0)
        metadata_bounds();
    else if (std::strcmp(argv[1], "exported-types") == 0)
        exported_types();
    else if (std::strcmp(argv[1], "failed-initializer") == 0)
        failed_initializer();
    else if (std::strcmp(argv[1], "invocation-buffers") == 0)
        invocation_buffers();
    else if (std::strcmp(argv[1], "weak-finalizers") == 0)
        weak_finalizers();
    else if (std::strcmp(argv[1], "allocation-failures") == 0)
        allocation_failures();
    else if (std::strcmp(argv[1], "loader-once") == 0)
        loader_once();
    else if (std::strcmp(argv[1], "gc-accounting") == 0)
        gc_accounting();
    else if (std::strcmp(argv[1], "compressed-signed") == 0)
        compressed_signed();
    else if (std::strcmp(argv[1], "integer-arithmetic") == 0)
        integer_arithmetic();
    else if (std::strcmp(argv[1], "unaligned-image") == 0)
        unaligned_image();
    else if (std::strcmp(argv[1], "type-system") == 0 || std::strcmp(argv[1], "packed-memory") == 0)
    {
        auto assembly = vm::Assembly::load_by_name("RegressionFixtures");
        check(assembly.is_ok(), "load type fixture");
        auto klass =
            assembly.unwrap()->mod->get_class_by_name(std::strcmp(argv[1], "packed-memory") == 0 ? "PackedMemoryFixture" : "TypeSystemFixture", false, false);
        check(klass.is_ok() && klass.unwrap(), "type fixture");
        auto initialized_type = vm::Class::initialize_all(klass.unwrap());
        check(initialized_type.is_ok(), "initialize type fixture");
        auto* method = vm::Method::find_matched_method_in_class_by_name(klass.unwrap(), "Run");
        auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, nullptr);
        check(result.is_ok() && result.unwrap(), "type fixture invocation");
        int32_t failures = *reinterpret_cast<int32_t*>(result.unwrap() + 1);
        std::printf("type system failure mask: %d\n", failures);
        check(failures == 0, "type constraint/variance/layout semantics");
    }
    else if (std::strcmp(argv[1], "binary-il") == 0)
    {
        auto assembly = vm::Assembly::load_by_name("PortableMemory");
        check(assembly.is_ok(), "load binary IL fixture");
        auto klass = assembly.unwrap()->mod->get_class_by_name("PortableILFixture", false, false);
        check(klass.is_ok() && klass.unwrap(), "resolve binary IL fixture");
        check(vm::Class::initialize_all(klass.unwrap()).is_ok(), "initialize binary IL fixture");
        auto* method = vm::Method::find_matched_method_in_class_by_name(klass.unwrap(), "Run");
        auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, nullptr);
        check(result.is_ok() && result.unwrap(), "execute backward switch operands");
        check(*reinterpret_cast<int32_t*>(result.unwrap() + 1) == 43, "signed little-endian switch offsets");
        method = vm::Method::find_matched_method_in_class_by_name(klass.unwrap(), "Constant");
        auto integer_result = vm::Runtime::invoke_with_run_cctor(method, nullptr, nullptr);
        check(integer_result.is_ok() && integer_result.unwrap(), "execute 64-bit constant");
        check(*reinterpret_cast<int64_t*>(integer_result.unwrap() + 1) == INT64_C(0x0123456789ABCDEF), "zero-extend constant low word");
        method = vm::Method::find_matched_method_in_class_by_name(klass.unwrap(), "DoubleConstant");
        auto double_result = vm::Runtime::invoke_with_run_cctor(method, nullptr, nullptr);
        check(double_result.is_ok() && double_result.unwrap(), "execute double constant");
        check(*reinterpret_cast<double*>(double_result.unwrap() + 1) == 12345.678, "preserve constant double bits");
        for (const char* name : {"BranchToNext", "ConditionalBranchToNext", "ComparisonBranchToNext", "SwitchToNext"})
        {
            method = vm::Method::find_matched_method_in_class_by_name(klass.unwrap(), name);
            auto branch_result = vm::Runtime::invoke_with_run_cctor(method, nullptr, nullptr);
            check(branch_result.is_ok() && branch_result.unwrap(), "execute branch to next instruction");
            check(*reinterpret_cast<int32_t*>(branch_result.unwrap() + 1) == 42, "preserve stack across zero-offset branch");
        }
    }
    else if (std::strcmp(argv[1], "managed-host-services") == 0)
    {
        auto assembly = vm::Assembly::load_by_name("RegressionFixtures");
        check(assembly.is_ok(), "load host fixture");
        auto klass = assembly.unwrap()->mod->get_class_by_name("HostServicesFixture", false, false);
        check(klass.is_ok() && klass.unwrap(), "host fixture type");
        check(vm::Class::initialize_all(klass.unwrap()).is_ok(), "initialize host fixture");
        auto* method = vm::Method::find_matched_method_in_class_by_name(klass.unwrap(), "Run");
        const uint16_t path[] = {'m', 'e', 'm', 0};
        int32_t error = -1;
        intptr_t handle = host::Services::files().open(path, 3, 3, 0, 0, &error);
        check(handle == 42 && error == 0, "open memory-backed host stream");
        const void* args[] = {&handle};
        auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
        check(result.is_ok() && result.unwrap(), "managed host fixture invocation");
        auto code = *reinterpret_cast<int32_t*>(result.unwrap() + 1);
        std::printf("managed host fixture failure: %d\n", code);
        check(code == 0 && FakeHost::state().opens > 0 && FakeHost::state().reads > 0 && FakeHost::state().writes > 0 && FakeHost::state().closes > 0,
              "managed file and clock calls must reach injected host");
        check(FakeHost::state().random_calls > 0, "managed Guid entropy must reach host");
        FakeHost::state().fail_random = true;
        method = vm::Method::find_matched_method_in_class_by_name(klass.unwrap(), "EntropyFailure");
        auto failed_entropy = vm::Runtime::invoke_with_run_cctor(method, nullptr, nullptr);
        check(failed_entropy.is_ok() && failed_entropy.unwrap() && *reinterpret_cast<int32_t*>(failed_entropy.unwrap() + 1) == 0,
              "managed entropy failure must map to BCL exception");
    }
    else if (std::strcmp(argv[1], "invalid-layouts") == 0)
    {
        auto assembly = vm::Assembly::load_by_name("InvalidLayouts");
        check(assembly.is_ok(), "load invalid layout metadata");
        for (auto name : {"BadAlignedRef", "BadRefOverlap", "BadNestedOverlap", "BadOffsetOverflow"})
        {
            auto klass = assembly.unwrap()->mod->get_class_by_name(name, false, false);
            check(klass.is_ok() && klass.unwrap(), "resolve invalid layout type");
            for (int i = 0; i < 2; ++i)
            {
                auto initialized_layout = vm::Class::initialize_fields(klass.unwrap());
                check(initialized_layout.is_err() && initialized_layout.unwrap_err() == RtErr::TypeLoad, name);
            }
        }
    }
    else if (std::strcmp(argv[1], "nested-reference-layout") == 0)
    {
        auto assembly = vm::Assembly::load_by_name("RegressionFixtures");
        check(assembly.is_ok(), "layout fixture assembly");
        auto klass = assembly.unwrap()->mod->get_class_by_name("LayoutHolder", false, false);
        check(klass.is_ok(), "layout holder type");
        auto holder = LEANCLR_NEWOBJ_INTERNAL(klass.unwrap(), "layout-test");
        auto target = LEANCLR_NEWOBJ_INTERNAL(vm::Class::get_corlib_types().cls_object, "layout-target");
        check(holder.is_ok() && target.is_ok(), "allocate layout graph");
        const auto* field = vm::Class::get_field_for_name(klass.unwrap(), "Value", false);
        auto* slot = reinterpret_cast<uint8_t*>(holder.unwrap() + 1) + field->offset;
        auto* target_object = target.unwrap();
        std::memcpy(slot, &target_object, sizeof(target_object));
        void* keep = vm::GCHandle::new_handle(holder.unwrap(), false);
        void* weak = vm::GCHandle::new_weakref_handle(target_object, false);
        vm::GC::enable();
        vm::GC::collect(0);
        check(vm::GCHandle::get_target(weak) == target_object, "GC must scan nested explicit reference slots");
        vm::GCHandle::free_handle(keep);
        vm::GC::collect(0);
        vm::GC::disable();
        check(vm::GCHandle::get_target(weak) == nullptr, "unrooted nested graph must collect");
        vm::GCHandle::free_handle(weak);
    }
    else if (std::strcmp(argv[1], "lifecycle") == 0)
    {
        check(vm::Runtime::get_state() == vm::Runtime::State::Running, "runtime running");
        auto again = vm::Runtime::initialize();
        check(again.is_err() && again.unwrap_err() == RtErr::InvalidOperation, "reject double initialization");
        vm::Runtime::shutdown();
        vm::Runtime::shutdown();
        check(vm::Runtime::get_state() == vm::Runtime::State::Stopped, "terminal shutdown");
        auto invoke = vm::Runtime::invoke_with_run_cctor(nullptr, nullptr, nullptr);
        check(invoke.is_err() && invoke.unwrap_err() == RtErr::InvalidOperation, "reject execution after shutdown");
    }
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
    else
        check(false, "unknown regression case");
    return 0;
}

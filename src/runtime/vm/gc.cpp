#include "gc.h"
#include "appdomain.h"
#include "settings.h"
#include "gc/garbage_collector.h"
#include "gc/gc_finalizer.h"

namespace leanclr
{
namespace metadata
{
void register_modules_gc_roots();
}

namespace vm
{

static gc::GCMode s_mode = gc::GCMode::ENABLED;
static int64_t s_max_time_slice_ns = 0;
static int64_t s_used_size = 0;
static int64_t s_heap_size = 0;
static void* s_heap_start = nullptr;
static void* s_heap_end = nullptr;
static void* s_heap_top = nullptr;
static void* s_heap_bottom = nullptr;

void register_exception_gc_roots();
void register_string_gc_roots();
void register_appdomain_gc_roots();
void register_environment_gc_roots();
void register_reflection_gc_roots();
void register_threading_gc_roots();

void register_all_gc_roots()
{
    register_exception_gc_roots();
    register_string_gc_roots();
    register_appdomain_gc_roots();
    register_environment_gc_roots();
    register_reflection_gc_roots();
    register_threading_gc_roots();
    metadata::register_modules_gc_roots();
}

void GC::initialize()
{
    s_mode = Settings::get_gc_mode();
    auto config = gc::GarbageCollector::GcHeapImpl::Config{s_mode, {gc::GC_DEFAULT_BYTE_THRESHOLD, gc::GC_DEFAULT_SOFT_HEAP_LIMIT}};
    gc::GarbageCollector::initialize(config);
    register_all_gc_roots();
}

vm::RtObject* GC::get_ephemeron_tombstone()
{
    return AppDomain::get_ephemeron_tombstone();
}

void GC::register_ephemeron_array(vm::RtObject* arr)
{
    (void)arr;
}

int32_t GC::get_collection_count(int32_t generation)
{
    (void)generation;
    return gc::GarbageCollector::get_collection_count();
}

int32_t GC::get_max_generation()
{
    return 0;
}

void GC::collect(int32_t generation)
{
    gc::GarbageCollector::collect();
}

int32_t GC::collect_a_little()
{
    return gc::GarbageCollector::maybe_collect() ? 1 : 0;
}

void GC::internal_collect(int32_t generation)
{
    collect(generation);
}

void GC::record_pressure(int64_t bytes)
{
    gc::GcPressure::record_external(bytes);
}

int64_t GC::get_allocated_bytes_for_current_thread()
{
    return gc::GcPressure::get_bytes_allocated_since_last_gc();
}

int32_t GC::get_generation(vm::RtObject* obj)
{
    (void)obj;
    return 0;
}

void GC::wait_for_pending_finalizers()
{
    gc::GcFinalizer::wait_for_pending_finalizers();
}

void GC::suppress_finalize(vm::RtObject* obj)
{
    gc::GcFinalizer::suppress_finalize(obj);
}

void GC::reregister_for_finalize(vm::RtObject* obj)
{
    gc::GcFinalizer::reregister_for_finalize(obj);
}

int64_t GC::get_total_memory(bool force_full_collection)
{
    // don't support collection when managed code is running
    // if (force_full_collection)
    // {
    //     gc::GarbageCollector::collect();
    // }
    return gc::GarbageCollector::get_heap_size();
}

void GC::start_incremental_collection()
{
}

void GC::enable()
{
    // printf("GC::enable\n");
    set_mode(gc::GCMode::ENABLED);
}

void GC::disable()
{
    // printf("GC::disable\n");
    set_mode(gc::GCMode::DISABLED);
}

bool GC::is_disabled()
{
    return s_mode == gc::GCMode::DISABLED;
}

void GC::set_mode(gc::GCMode mode)
{
    // printf("GC::set_mode: %d\n", mode);
    s_mode = mode;
    gc::GarbageCollector::set_gc_mode(mode);
}

bool GC::is_incremental()
{
    return false;
}

void GC::set_max_time_slice_ns(int64_t maxTimeSlice)
{
    s_max_time_slice_ns = maxTimeSlice;
}

int64_t GC::get_max_time_slice_ns()
{
    return s_max_time_slice_ns;
}

int64_t GC::get_used_size()
{
    return s_used_size;
}

int64_t GC::get_heap_size()
{
    return s_heap_size;
}

void GC::foreach_heap(void (*func)(void* data, void* context), void* userData)
{
    (void)func;
    (void)userData;
}

void GC::start_gc_world()
{
}

void GC::stop_gc_world()
{
}

void GC::write_barrier(RtObject** obj_ref_location, RtObject* new_obj)
{
    gc::GarbageCollector::write_barrier(obj_ref_location, new_obj);
}

bool GC::has_strict_wbarriers()
{
    return false;
}

void GC::set_external_allocation_tracker(void (*func)(void*, size_t, int))
{
    (void)func;
}

void GC::set_external_wbarrier_tracker(void (*func)(void**))
{
    (void)func;
}
} // namespace vm
} // namespace leanclr

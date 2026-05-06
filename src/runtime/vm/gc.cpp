#include "gc.h"
#include "appdomain.h"
#include "gc/garbage_collector.h"

namespace leanclr
{
namespace vm
{

static GCMode s_mode = GCMode::ENABLED;
static bool s_is_incremental = false;
static int64_t s_max_time_slice_ns = 0;
static int64_t s_used_size = 0;
static int64_t s_heap_size = 0;
static void* s_heap_start = nullptr;
static void* s_heap_end = nullptr;
static void* s_heap_top = nullptr;
static void* s_heap_bottom = nullptr;

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
    return 0;
}

int32_t GC::get_max_generation()
{
    return 0;
}

void GC::collect(int32_t generation)
{
    (void)generation;
}

int32_t GC::collect_a_little()
{
    return 0;
}

void GC::internal_collect(int32_t generation)
{
    collect(generation);
}

void GC::record_pressure(int64_t bytes)
{
    (void)bytes;
}

int64_t GC::get_allocated_bytes_for_current_thread()
{
    return 0;
}

int32_t GC::get_generation(vm::RtObject* obj)
{
    (void)obj;
    return 0;
}

void GC::wait_for_pending_finalizers()
{
}

void GC::suppress_finalize(vm::RtObject* obj)
{
    (void)obj;
}

void GC::reregister_for_finalize(vm::RtObject* obj)
{
    (void)obj;
}

int64_t GC::get_total_memory(bool force_full_collection)
{
    (void)force_full_collection;
    return 0;
}

void GC::start_incremental_collection()
{
}

void GC::enable()
{
    s_mode = GCMode::ENABLED;
}

void GC::disable()
{
    s_mode = GCMode::DISABLED;
}

bool GC::is_disabled()
{
    return s_mode == GCMode::DISABLED;
}

void GC::set_mode(GCMode mode)
{
    s_mode = mode;
}

bool GC::is_incremental()
{
    return s_is_incremental;
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

void* GC::alloc_fixed(size_t size)
{
    return gc::GarbageCollector::allocate_fixed(size);
}

void GC::free_fixed(void* address)
{
    gc::GarbageCollector::free_fixed(address);
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

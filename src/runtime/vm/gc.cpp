#include "gc.h"
#include "appdomain.h"

namespace leanclr
{
namespace vm
{

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

void GC::internal_collect(int32_t generation)
{
    (void)generation;
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

} // namespace vm
} // namespace leanclr

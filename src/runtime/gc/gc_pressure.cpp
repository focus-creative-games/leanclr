#include "gc_pressure.h"
#include "gc_config.h"

namespace leanclr
{
namespace gc
{

static GcPressureConfig s_config = {};
static GcPressureState s_state = {};
static int64_t s_used_size = 0;

void GcPressure::initialize(const GcPressureConfig& config)
{
    s_config = config;
    s_state = {};
}

void GcPressure::set_config(const GcPressureConfig& config)
{
    s_config = config;
}

void GcPressure::on_alloc(size_t bytes)
{
    s_state.bytes_allocated_since_last_gc += bytes;
    s_state.objects_allocated_since_last_gc++;
}

void GcPressure::record_external(int64_t bytes)
{
    if (bytes > 0)
    {
        s_state.external_pressure += bytes;
    }
}

void GcPressure::on_collect()
{
    s_state.bytes_allocated_since_last_gc = 0;
    s_state.external_pressure = 0;
    s_state.objects_allocated_since_last_gc = 0;
}

void GcPressure::set_used_size(int64_t used)
{
    s_used_size = used;
}

int64_t GcPressure::get_effective_pressure()
{
    return s_state.bytes_allocated_since_last_gc + s_state.external_pressure;
}

int64_t GcPressure::get_bytes_allocated_since_last_gc()
{
    return s_state.bytes_allocated_since_last_gc;
}

bool GcPressure::should_collect(bool force)
{
    if (force)
    {
        return true;
    }
    return s_state.bytes_allocated_since_last_gc >= 1 * 1024 * 1024;
    // if (get_effective_pressure() >= s_config.byte_threshold)
    // {
    //     return true;
    // }
    // if (s_config.soft_heap_limit > 0 && static_cast<uint64_t>(s_used_size) >= s_config.soft_heap_limit)
    // {
    //     return true;
    // }
    // return false;
}

} // namespace gc
} // namespace leanclr

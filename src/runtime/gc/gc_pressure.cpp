#include "gc_pressure.h"
#include "gc_config.h"

namespace leanclr
{
namespace gc
{

static GcPressureConfig s_config = {};
static GcPressureState s_state = {};
static int64_t s_used_size = 0;
static int64_t s_total_allocated_bytes = 0;

static int64_t add_saturated(int64_t current, uint64_t bytes)
{
    return bytes > static_cast<uint64_t>(INT64_MAX - current) ? INT64_MAX : current + static_cast<int64_t>(bytes);
}

void GcPressure::initialize(const GcPressureConfig& config)
{
    s_config = config;
    s_state = {};
    s_used_size = 0;
    s_total_allocated_bytes = 0;
}

void GcPressure::set_config(const GcPressureConfig& config)
{
    s_config = config;
}

void GcPressure::on_alloc(size_t bytes)
{
    s_state.bytes_allocated_since_last_gc = add_saturated(s_state.bytes_allocated_since_last_gc, bytes);
    s_total_allocated_bytes = add_saturated(s_total_allocated_bytes, bytes);
    if (s_state.objects_allocated_since_last_gc < INT32_MAX) s_state.objects_allocated_since_last_gc++;
}

void GcPressure::record_external(int64_t bytes)
{
    if (bytes > 0)
    {
        s_state.external_pressure = add_saturated(s_state.external_pressure, static_cast<uint64_t>(bytes));
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
    return add_saturated(s_state.bytes_allocated_since_last_gc, static_cast<uint64_t>(s_state.external_pressure));
}

int64_t GcPressure::get_bytes_allocated_since_last_gc()
{
    return s_state.bytes_allocated_since_last_gc;
}

int64_t GcPressure::get_total_allocated_bytes()
{
    return s_total_allocated_bytes;
}

bool GcPressure::should_collect(bool force)
{
    if (force)
    {
        return true;
    }
    return (s_config.byte_threshold > 0 && get_effective_pressure() >= s_config.byte_threshold) ||
           (s_config.soft_heap_limit > 0 && s_used_size >= s_config.soft_heap_limit);
}

} // namespace gc
} // namespace leanclr

#include "garbage_collector.h"

namespace leanclr
{
namespace gc
{

void GarbageCollector::record_pressure(int64_t bytes)
{
    GcPressure::record_external(bytes);
}

uint64_t GarbageCollector::get_effective_pressure()
{
    return GcPressure::get_effective_pressure();
}

uint64_t GarbageCollector::get_bytes_allocated_since_last_gc()
{
    return GcPressure::get_bytes_allocated_since_last_gc();
}

} // namespace gc
} // namespace leanclr

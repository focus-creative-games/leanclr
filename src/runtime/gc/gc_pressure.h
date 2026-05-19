#pragma once

#include "core/rt_base.h"

namespace leanclr
{
namespace gc
{

struct GcPressureConfig
{
    uint64_t byte_threshold;
    uint64_t soft_heap_limit;
};

struct GcPressureState
{
    uint64_t bytes_allocated_since_last_gc;
    uint64_t external_pressure;
    uint32_t objects_allocated_since_last_gc;
};

class GcPressure
{
  public:
    static void initialize(const GcPressureConfig& config);
    static void set_config(const GcPressureConfig& config);

    static void on_alloc(size_t bytes);
    static void record_external(int64_t bytes);
    static void on_collect();

    static uint64_t get_effective_pressure();
    static uint64_t get_bytes_allocated_since_last_gc();
    static bool should_collect(bool force);
};

void gc_pressure_set_used_size(int64_t used);

} // namespace gc
} // namespace leanclr

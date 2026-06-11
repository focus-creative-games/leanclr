#pragma once

#include "core/rt_base.h"

namespace leanclr
{
namespace gc
{

struct GcPressureConfig
{
    int64_t byte_threshold;
    int64_t soft_heap_limit;
};

struct GcPressureState
{
    int64_t bytes_allocated_since_last_gc;
    int64_t external_pressure;
    int32_t objects_allocated_since_last_gc;
};

class GcPressure
{
  public:
    static void initialize(const GcPressureConfig& config);
    static void set_config(const GcPressureConfig& config);

    static void on_alloc(size_t bytes);
    static void record_external(int64_t bytes);
    static void on_collect();

    static int64_t get_effective_pressure();
    static int64_t get_bytes_allocated_since_last_gc();
    static void set_used_size(int64_t used);
    static bool should_collect(bool force);
};

} // namespace gc
} // namespace leanclr

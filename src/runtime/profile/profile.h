#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "core/rt_base.h"
#include "metadata/rt_metadata.h"

namespace leanclr
{
namespace profile
{

struct ProfileSlot
{
    uint64_t period_calls;
    uint64_t period_cost;
    uint64_t global_calls;
    uint64_t global_cost;
};

struct ProfileEntry
{
    const metadata::RtMethodInfo* method;
    uint64_t calls;
    uint64_t cost;
};

struct ExportOptions
{
    uint64_t min_calls = 1;
    uint64_t min_cost = 0;
    size_t top_n = 0;
};

class Profile
{
  public:
    static bool is_enabled() noexcept;
    static void set_enabled(bool enabled) noexcept;

    static uint32_t ensure_slot(metadata::RtMethodInfo* method) noexcept;

    static void inc_call_count(const metadata::RtMethodInfo* method, uint64_t delta = 1) noexcept;
    static void add_cost(const metadata::RtMethodInfo* method, uint64_t cost) noexcept;

    static void end_period() noexcept;
    static void reset_period() noexcept;
    static void reset_global() noexcept;

    static size_t get_period_entries(ProfileEntry* out, size_t capacity) noexcept;
    static size_t get_global_entries(ProfileEntry* out, size_t capacity) noexcept;

    static RtResult<std::string> get_period_stats_json(const ExportOptions& options = ExportOptions{}) noexcept;
    static RtResult<std::string> get_global_stats_json(const ExportOptions& options = ExportOptions{}) noexcept;

    static RtResultVoid export_period_stats_json(const char* path, const ExportOptions& options = ExportOptions{}) noexcept;
    static RtResultVoid export_global_stats_json(const char* path, const ExportOptions& options = ExportOptions{}) noexcept;
};

} // namespace profile
} // namespace leanclr

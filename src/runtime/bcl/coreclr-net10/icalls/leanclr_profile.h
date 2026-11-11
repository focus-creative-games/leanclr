#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class LeanCLRProfile
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResultVoid reset_period_stats() noexcept;
    static RtResultVoid end_period_stats() noexcept;
    static RtResultVoid reset_global_stats() noexcept;

    static RtResult<vm::RtString*> get_period_stats() noexcept;
    static RtResult<vm::RtString*> get_global_stats() noexcept;
    static RtResult<vm::RtString*> get_period_stats_json() noexcept;
    static RtResult<vm::RtString*> get_global_stats_json() noexcept;

    static RtResultVoid export_period_stats_json(vm::RtString* path) noexcept;
    static RtResultVoid export_global_stats_json(vm::RtString* path) noexcept;
};

} // namespace icalls
} // namespace leanclr

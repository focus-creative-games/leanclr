#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemGC
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<vm::RtObject*> get_ephemeron_tombstone() noexcept;
    static RtResultVoid register_ephemeron_array(vm::RtObject* arr) noexcept;
    static RtResult<int32_t> get_collection_count(int32_t generation) noexcept;
    static RtResult<int32_t> get_max_generation() noexcept;
    static RtResultVoid internal_collect(int32_t generation) noexcept;
    static RtResultVoid record_pressure(int64_t bytes) noexcept;
    static RtResult<int64_t> get_allocated_bytes_for_current_thread() noexcept;
    static RtResult<int32_t> get_generation(vm::RtObject* obj) noexcept;
    static RtResultVoid wait_for_pending_finalizers() noexcept;
    static RtResultVoid suppress_finalize(vm::RtObject* obj) noexcept;
    static RtResultVoid reregister_for_finalize(vm::RtObject* obj) noexcept;
    static RtResult<int64_t> get_total_memory(bool force_full_collection) noexcept;
};

} // namespace icalls
} // namespace leanclr

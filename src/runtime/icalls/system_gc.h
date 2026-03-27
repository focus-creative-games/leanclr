#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemGC
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    static RtResult<vm::RtObject*> get_ephemeron_tombstone();
    static RtResultVoid register_ephemeron_array(vm::RtObject* arr);
    static RtResult<int32_t> get_collection_count(int32_t generation);
    static RtResult<int32_t> get_max_generation();
    static RtResultVoid internal_collect(int32_t generation);
    static RtResultVoid record_pressure(int64_t bytes);
    static RtResult<int64_t> get_allocated_bytes_for_current_thread();
    static RtResult<int32_t> get_generation(vm::RtObject* obj);
    static RtResultVoid wait_for_pending_finalizers();
    static RtResultVoid suppress_finalize(vm::RtObject* obj);
    static RtResultVoid reregister_for_finalize(vm::RtObject* obj);
    static RtResult<int64_t> get_total_memory(bool force_full_collection);
};

} // namespace icalls
} // namespace leanclr

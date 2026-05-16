#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemThreadingThreadPool
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    /// Placeholders for single-threaded CLR; values are static defaults / last-set echoes only.
    static RtResult<bool> set_min_threads_native(int32_t worker_threads, int32_t completion_port_threads) noexcept;
    static RtResult<bool> set_max_threads_native(int32_t worker_threads, int32_t completion_port_threads) noexcept;
    static RtResultVoid get_min_threads_native(int32_t* worker_threads, int32_t* completion_port_threads) noexcept;
    static RtResultVoid get_max_threads_native(int32_t* worker_threads, int32_t* completion_port_threads) noexcept;
    static RtResultVoid get_available_threads_native(int32_t* worker_threads, int32_t* completion_port_threads) noexcept;
};

} // namespace icalls
} // namespace leanclr

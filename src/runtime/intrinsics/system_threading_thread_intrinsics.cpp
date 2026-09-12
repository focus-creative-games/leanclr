#include "system_threading_thread_intrinsics.h"

namespace leanclr
{
namespace intrinsics
{

RtResultVoid SystemThreadingThreadIntrinsics::fast_poll_gc() noexcept
{
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Thread::FastPollGC
static RtResultVoid fast_poll_gc_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    (void)ret;
    return SystemThreadingThreadIntrinsics::fast_poll_gc();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_threading_thread[] = {
    {"System.Threading.Thread::FastPollGC()", (vm::IntrinsicFunction)&SystemThreadingThreadIntrinsics::fast_poll_gc, fast_poll_gc_invoker},
    {"System.Threading.Thread::FastPollGC", (vm::IntrinsicFunction)&SystemThreadingThreadIntrinsics::fast_poll_gc, fast_poll_gc_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemThreadingThreadIntrinsics::get_intrinsic_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_intrinsic_entries_system_threading_thread) / sizeof(s_intrinsic_entries_system_threading_thread[0]);
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_threading_thread, entry_count);
}

} // namespace intrinsics
} // namespace leanclr

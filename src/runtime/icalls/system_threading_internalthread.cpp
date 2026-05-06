#include "system_threading_internalthread.h"

#include "alloc/general_allocation.h"
#include "vm/rt_managed_types.h"
#include "vm/rt_thread.h"

namespace leanclr
{
namespace icalls
{

// Thread cleanup
RtResultVoid SystemThreadingInternalThread::thread_free_internal(vm::RtInternalThread* this_thread) noexcept
{
    return vm::Thread::free_internal_thread(this_thread);
}

// Invoker function for thread_free_internal
static RtResultVoid thread_free_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto this_thread = EvalStackOp::get_param<vm::RtInternalThread*>(params, 0);
    return SystemThreadingInternalThread::thread_free_internal(this_thread);
}

// Internal call entries
static vm::InternalCallEntry s_internal_call_entries_system_threading_internalthread[] = {
    {"System.Threading.InternalThread::Thread_free_internal", (vm::InternalCallFunction)&SystemThreadingInternalThread::thread_free_internal,
     thread_free_internal_invoker},
};

utils::Span<vm::InternalCallEntry> SystemThreadingInternalThread::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_threading_internalthread,
                                              sizeof(s_internal_call_entries_system_threading_internalthread) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

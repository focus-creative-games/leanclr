#include "system_threading_threadpool.h"

namespace leanclr
{
namespace icalls
{
namespace
{
// Logical thread-pool sizing for a single-threaded runtime: holds echo of last Set* only.
int32_t g_min_worker = 1;
int32_t g_min_completion_port = 1;
int32_t g_max_worker = 32767;
int32_t g_max_completion_port = 1000;
} // namespace

RtResult<bool> SystemThreadingThreadPool::set_min_threads_native(int32_t worker_threads, int32_t completion_port_threads) noexcept
{
    g_min_worker = worker_threads;
    g_min_completion_port = completion_port_threads;
    RET_OK(true);
}

/// @icall: System.Threading.ThreadPool::SetMinThreadsNative(System.Int32,System.Int32)
static RtResultVoid set_min_threads_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* ret) noexcept
{
    int32_t w = EvalStackOp::get_param<int32_t>(params, 0);
    int32_t c = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemThreadingThreadPool::set_min_threads_native(w, c));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

RtResult<bool> SystemThreadingThreadPool::set_max_threads_native(int32_t worker_threads, int32_t completion_port_threads) noexcept
{
    g_max_worker = worker_threads;
    g_max_completion_port = completion_port_threads;
    RET_OK(true);
}

/// @icall: System.Threading.ThreadPool::SetMaxThreadsNative(System.Int32,System.Int32)
static RtResultVoid set_max_threads_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* ret) noexcept
{
    int32_t w = EvalStackOp::get_param<int32_t>(params, 0);
    int32_t c = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemThreadingThreadPool::set_max_threads_native(w, c));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThreadPool::get_min_threads_native(int32_t* worker_threads, int32_t* completion_port_threads) noexcept
{
    if (worker_threads != nullptr)
        *worker_threads = g_min_worker;
    if (completion_port_threads != nullptr)
        *completion_port_threads = g_min_completion_port;
    RET_VOID_OK();
}

/// @icall: System.Threading.ThreadPool::GetMinThreadsNative(System.Int32&,System.Int32&)
static RtResultVoid get_min_threads_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* /*ret*/) noexcept
{
    int32_t* w = EvalStackOp::get_param<int32_t*>(params, 0);
    int32_t* c = EvalStackOp::get_param<int32_t*>(params, 1);
    return SystemThreadingThreadPool::get_min_threads_native(w, c);
}

RtResultVoid SystemThreadingThreadPool::get_max_threads_native(int32_t* worker_threads, int32_t* completion_port_threads) noexcept
{
    if (worker_threads != nullptr)
        *worker_threads = g_max_worker;
    if (completion_port_threads != nullptr)
        *completion_port_threads = g_max_completion_port;
    RET_VOID_OK();
}

/// @icall: System.Threading.ThreadPool::GetMaxThreadsNative(System.Int32&,System.Int32&)
static RtResultVoid get_max_threads_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* /*ret*/) noexcept
{
    int32_t* w = EvalStackOp::get_param<int32_t*>(params, 0);
    int32_t* c = EvalStackOp::get_param<int32_t*>(params, 1);
    return SystemThreadingThreadPool::get_max_threads_native(w, c);
}

RtResultVoid SystemThreadingThreadPool::get_available_threads_native(int32_t* worker_threads, int32_t* completion_port_threads) noexcept
{
    // Pretend the pool is idle: available == configured max (no actual parallel work).
    if (worker_threads != nullptr)
        *worker_threads = g_max_worker;
    if (completion_port_threads != nullptr)
        *completion_port_threads = g_max_completion_port;
    RET_VOID_OK();
}

/// @icall: System.Threading.ThreadPool::GetAvailableThreadsNative(System.Int32&,System.Int32&)
static RtResultVoid get_available_threads_native_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                         const interp::RtStackObject* params, interp::RtStackObject* /*ret*/) noexcept
{
    int32_t* w = EvalStackOp::get_param<int32_t*>(params, 0);
    int32_t* c = EvalStackOp::get_param<int32_t*>(params, 1);
    return SystemThreadingThreadPool::get_available_threads_native(w, c);
}

static vm::InternalCallEntry s_internal_call_entries_system_threading_threadpool[] = {
    {"System.Threading.ThreadPool::SetMinThreadsNative(System.Int32,System.Int32)",
     (vm::InternalCallFunction)&SystemThreadingThreadPool::set_min_threads_native, set_min_threads_native_invoker},
    {"System.Threading.ThreadPool::SetMaxThreadsNative(System.Int32,System.Int32)",
     (vm::InternalCallFunction)&SystemThreadingThreadPool::set_max_threads_native, set_max_threads_native_invoker},
    {"System.Threading.ThreadPool::GetMinThreadsNative(System.Int32&,System.Int32&)",
     (vm::InternalCallFunction)&SystemThreadingThreadPool::get_min_threads_native, get_min_threads_native_invoker},
    {"System.Threading.ThreadPool::GetMaxThreadsNative(System.Int32&,System.Int32&)",
     (vm::InternalCallFunction)&SystemThreadingThreadPool::get_max_threads_native, get_max_threads_native_invoker},
    {"System.Threading.ThreadPool::GetAvailableThreadsNative(System.Int32&,System.Int32&)",
     (vm::InternalCallFunction)&SystemThreadingThreadPool::get_available_threads_native, get_available_threads_native_invoker},
};

utils::Span<vm::InternalCallEntry> SystemThreadingThreadPool::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_threading_threadpool,
                                              sizeof(s_internal_call_entries_system_threading_threadpool) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

#include "system_gc.h"

#include "icall_base.h"
#include "vm/gc.h"

namespace leanclr
{
namespace icalls
{

RtResult<vm::RtObject*> SystemGC::get_ephemeron_tombstone() noexcept
{
    RET_OK(vm::GC::get_ephemeron_tombstone());
}

/// @icall: System.GC::get_ephemeron_tombstone
static RtResultVoid get_ephemeron_tombstone_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                    const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, tombstone, SystemGC::get_ephemeron_tombstone());
    EvalStackOp::set_return(ret, tombstone);
    RET_VOID_OK();
}

RtResultVoid SystemGC::register_ephemeron_array(vm::RtObject* arr) noexcept
{
    vm::GC::register_ephemeron_array(arr);
    RET_VOID_OK();
}

/// @icall: System.GC::register_ephemeron_array
static RtResultVoid register_ephemeron_array_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto arr = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    RET_ERR_ON_FAIL(SystemGC::register_ephemeron_array(arr));
    RET_VOID_OK();
}

RtResult<int32_t> SystemGC::get_collection_count(int32_t generation) noexcept
{
    RET_OK(vm::GC::get_collection_count(generation));
}

/// @icall: System.GC::GetCollectionCount(System.Int32)
static RtResultVoid get_collection_count_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto generation = EvalStackOp::get_param<int32_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, count, SystemGC::get_collection_count(generation));
    EvalStackOp::set_return(ret, count);
    RET_VOID_OK();
}

RtResult<int32_t> SystemGC::get_max_generation() noexcept
{
    RET_OK(vm::GC::get_max_generation());
}

/// @icall: System.GC::GetMaxGeneration()
static RtResultVoid get_max_generation_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, generation, SystemGC::get_max_generation());
    EvalStackOp::set_return(ret, generation);
    RET_VOID_OK();
}

RtResultVoid SystemGC::internal_collect(int32_t generation) noexcept
{
    vm::GC::internal_collect(generation);
    RET_VOID_OK();
}

/// @icall: System.GC::InternalCollect(System.Int32)
static RtResultVoid internal_collect_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto generation = EvalStackOp::get_param<int32_t>(params, 0);
    RET_ERR_ON_FAIL(SystemGC::internal_collect(generation));
    RET_VOID_OK();
}

RtResultVoid SystemGC::record_pressure(int64_t bytes) noexcept
{
    vm::GC::record_pressure(bytes);
    RET_VOID_OK();
}

/// @icall: System.GC::RecordPressure(System.Int64)
static RtResultVoid record_pressure_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto bytes = EvalStackOp::get_param<int64_t>(params, 0);
    RET_ERR_ON_FAIL(SystemGC::record_pressure(bytes));
    RET_VOID_OK();
}

RtResult<int64_t> SystemGC::get_allocated_bytes_for_current_thread() noexcept
{
    RET_OK(vm::GC::get_allocated_bytes_for_current_thread());
}

/// @icall: System.GC::GetAllocatedBytesForCurrentThread()
static RtResultVoid get_allocated_bytes_for_current_thread_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, bytes, SystemGC::get_allocated_bytes_for_current_thread());
    EvalStackOp::set_return(ret, bytes);
    RET_VOID_OK();
}

RtResult<int32_t> SystemGC::get_generation(vm::RtObject* obj) noexcept
{
    RET_OK(vm::GC::get_generation(obj));
}

/// @icall: System.GC::GetGeneration(System.Object)
static RtResultVoid get_generation_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, generation, SystemGC::get_generation(obj));
    EvalStackOp::set_return(ret, generation);
    RET_VOID_OK();
}

RtResultVoid SystemGC::wait_for_pending_finalizers() noexcept
{
    vm::GC::wait_for_pending_finalizers();
    RET_VOID_OK();
}

/// @icall: System.GC::WaitForPendingFinalizers()
static RtResultVoid wait_for_pending_finalizers_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    (void)ret;
    RET_ERR_ON_FAIL(SystemGC::wait_for_pending_finalizers());
    RET_VOID_OK();
}

RtResultVoid SystemGC::suppress_finalize(vm::RtObject* obj) noexcept
{
    vm::GC::suppress_finalize(obj);
    RET_VOID_OK();
}

/// @icall: System.GC::_SuppressFinalize(System.Object)
static RtResultVoid suppress_finalize_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    RET_ERR_ON_FAIL(SystemGC::suppress_finalize(obj));
    RET_VOID_OK();
}

RtResultVoid SystemGC::reregister_for_finalize(vm::RtObject* obj) noexcept
{
    vm::GC::reregister_for_finalize(obj);
    RET_VOID_OK();
}

/// @icall: System.GC::_ReRegisterForFinalize(System.Object)
static RtResultVoid reregister_for_finalize_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                    const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    RET_ERR_ON_FAIL(SystemGC::reregister_for_finalize(obj));
    RET_VOID_OK();
}

RtResult<int64_t> SystemGC::get_total_memory(bool force_full_collection) noexcept
{
    RET_OK(vm::GC::get_total_memory(force_full_collection));
}

/// @icall: System.GC::GetTotalMemory(System.Boolean)
static RtResultVoid get_total_memory_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto force = EvalStackOp::get_param<bool>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, memory, SystemGC::get_total_memory(force));
    EvalStackOp::set_return(ret, memory);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemGC::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.GC::get_ephemeron_tombstone", (vm::InternalCallFunction)&SystemGC::get_ephemeron_tombstone, get_ephemeron_tombstone_invoker},
        {"System.GC::register_ephemeron_array", (vm::InternalCallFunction)&SystemGC::register_ephemeron_array, register_ephemeron_array_invoker},
        {"System.GC::GetCollectionCount(System.Int32)", (vm::InternalCallFunction)&SystemGC::get_collection_count, get_collection_count_invoker},
        {"System.GC::GetMaxGeneration()", (vm::InternalCallFunction)&SystemGC::get_max_generation, get_max_generation_invoker},
        {"System.GC::InternalCollect(System.Int32)", (vm::InternalCallFunction)&SystemGC::internal_collect, internal_collect_invoker},
        {"System.GC::RecordPressure(System.Int64)", (vm::InternalCallFunction)&SystemGC::record_pressure, record_pressure_invoker},
        {"System.GC::GetAllocatedBytesForCurrentThread()", (vm::InternalCallFunction)&SystemGC::get_allocated_bytes_for_current_thread,
         get_allocated_bytes_for_current_thread_invoker},
        {"System.GC::GetGeneration(System.Object)", (vm::InternalCallFunction)&SystemGC::get_generation, get_generation_invoker},
        {"System.GC::WaitForPendingFinalizers()", (vm::InternalCallFunction)&SystemGC::wait_for_pending_finalizers, wait_for_pending_finalizers_invoker},
        {"System.GC::_SuppressFinalize(System.Object)", (vm::InternalCallFunction)&SystemGC::suppress_finalize, suppress_finalize_invoker},
        {"System.GC::_ReRegisterForFinalize(System.Object)", (vm::InternalCallFunction)&SystemGC::reregister_for_finalize, reregister_for_finalize_invoker},
        {"System.GC::GetTotalMemory(System.Boolean)", (vm::InternalCallFunction)&SystemGC::get_total_memory, get_total_memory_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

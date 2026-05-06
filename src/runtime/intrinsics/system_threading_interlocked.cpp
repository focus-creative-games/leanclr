#include "system_threading_interlocked.h"

#include "core/rt_err.h"
#include "interp/eval_stack_op.h"
#include "vm/object.h"

namespace leanclr
{
namespace intrinsics
{
using namespace leanclr::metadata;
using namespace leanclr::interp;

// ============================================
// Exchange Object Implementation
// ============================================

RtResult<vm::RtObject*> SystemThreadingInterlocked::exchange_object(vm::RtObject** location, vm::RtObject* value) noexcept
{
    if (location == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    vm::RtObject* old = *location;
    *location = value;
    RET_OK(old);
}

/// @CUSTOM
/// @intrinsic: System.Threading.Interlocked::Exchange(System.Object&,System.Object)
static RtResultVoid exchange_object_invoker(RtManagedMethodPointer _method_pointer, const RtMethodInfo* _method, const RtStackObject* params,
                                            RtStackObject* ret) noexcept
{
    vm::RtObject** location = EvalStackOp::get_param<vm::RtObject**>(params, 0);
    vm::RtObject* value = EvalStackOp::get_param<vm::RtObject*>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, old, SystemThreadingInterlocked::exchange_object(location, value));
    EvalStackOp::set_return(ret, old);
    RET_VOID_OK();
}

// ============================================
// Exchange Generic Implementation
// ============================================

RtResult<void*> SystemThreadingInterlocked::exchange(void** location, void* value) noexcept
{
    if (location == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    void* old = *location;
    *location = value;
    RET_OK(old);
}

/// @intrinsic: System.Threading.Interlocked::Exchange<>
static RtResultVoid exchange_invoker(RtManagedMethodPointer _method_pointer, const RtMethodInfo* _method, const RtStackObject* params,
                                     RtStackObject* ret) noexcept
{
    void** location = EvalStackOp::get_param<void**>(params, 0);
    void* value = EvalStackOp::get_param<void*>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, old, SystemThreadingInterlocked::exchange(location, value));
    EvalStackOp::set_return(ret, old);
    RET_VOID_OK();
}

// ============================================
// CompareExchange Implementation
// ============================================

RtResult<void*> SystemThreadingInterlocked::compare_exchange(void** location, void* value, void* comparand) noexcept
{
    if (location == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    void* old = *location;
    if (old == comparand)
    {
        *location = value;
    }
    RET_OK(old);
}

/// @intrinsic: System.Threading.Interlocked::CompareExchange<>
static RtResultVoid compare_exchange_invoker(RtManagedMethodPointer _method_pointer, const RtMethodInfo* _method, const RtStackObject* params,
                                             RtStackObject* ret) noexcept
{
    void** location = EvalStackOp::get_param<void**>(params, 0);
    void* value = EvalStackOp::get_param<void*>(params, 1);
    void* comparand = EvalStackOp::get_param<void*>(params, 2);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, old, SystemThreadingInterlocked::compare_exchange(location, value, comparand));
    EvalStackOp::set_return(ret, old);
    RET_VOID_OK();
}

// ============================================
// MemoryBarrier Implementation
// ============================================

RtResultVoid SystemThreadingInterlocked::memory_barrier() noexcept
{
    // Memory barrier implementation - typically a no-op or compiler fence
    // The actual memory ordering is handled by the compiler/runtime
    RET_VOID_OK();
}

/// @intrinsic: System.Threading.Interlocked::MemoryBarrier
static RtResultVoid memory_barrier_invoker(RtManagedMethodPointer _method_pointer, const RtMethodInfo* _method, const RtStackObject* _params,
                                           RtStackObject* _ret) noexcept
{
    return SystemThreadingInterlocked::memory_barrier();
}

// ============================================
// Intrinsic Registration
// ============================================

static vm::IntrinsicEntry s_intrinsic_entries_system_threading_interlocked[] = {
    {"System.Threading.Interlocked::Exchange(System.Object&,System.Object)", (vm::IntrinsicFunction)&SystemThreadingInterlocked::exchange_object,
     exchange_object_invoker},
    {"System.Threading.Interlocked::Exchange<>", (vm::IntrinsicFunction)&SystemThreadingInterlocked::exchange, exchange_invoker},
    {"System.Threading.Interlocked::CompareExchange<>", (vm::IntrinsicFunction)&SystemThreadingInterlocked::compare_exchange, compare_exchange_invoker},
    {"System.Threading.Interlocked::MemoryBarrier", (vm::IntrinsicFunction)&SystemThreadingInterlocked::memory_barrier, memory_barrier_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemThreadingInterlocked::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_threading_interlocked,
                                           sizeof(s_intrinsic_entries_system_threading_interlocked) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

#include "system_threading_interlocked.h"
#include "interp/eval_stack_op.h"
#include <cstdint>
#include <cstring>

using namespace leanclr::core;
using namespace leanclr::vm;
using namespace leanclr::metadata;
using namespace leanclr::interp;

namespace leanclr
{
namespace icalls
{

// ========== Implementation Functions ==========

// Add operations
RtResult<int32_t> SystemThreadingInterlocked::add_i32(int32_t* location, int32_t value) noexcept
{
    int32_t result = *location + value;
    *location = result;
    RET_OK(result);
}

RtResult<int64_t> SystemThreadingInterlocked::add_i64(int64_t* location, int64_t value) noexcept
{
    int64_t result = *location + value;
    *location = result;
    RET_OK(result);
}

// Increment operations
RtResult<int32_t> SystemThreadingInterlocked::increment_i32(int32_t* location) noexcept
{
    int32_t result = *location + 1;
    *location = result;
    RET_OK(result);
}

RtResult<int64_t> SystemThreadingInterlocked::increment_i64(int64_t* location) noexcept
{
    int64_t result = *location + 1;
    *location = result;
    RET_OK(result);
}

// Decrement operations
RtResult<int32_t> SystemThreadingInterlocked::decrement_i32(int32_t* location) noexcept
{
    int32_t result = *location - 1;
    *location = result;
    RET_OK(result);
}

RtResult<int64_t> SystemThreadingInterlocked::decrement_i64(int64_t* location) noexcept
{
    int64_t result = *location - 1;
    *location = result;
    RET_OK(result);
}

// CompareExchange operations
RtResult<int32_t> SystemThreadingInterlocked::compare_exchange_i32(int32_t* location, int32_t value, int32_t comparand) noexcept
{
    int32_t old = *location;
    if (old == comparand)
    {
        *location = value;
    }
    RET_OK(old);
}

RtResult<int32_t> SystemThreadingInterlocked::compare_exchange2_i32(int32_t* location, int32_t value, int32_t comparand, bool* succ) noexcept
{
    int32_t old = *location;
    if (old == comparand)
    {
        *location = value;
        *succ = true;
    }
    else
    {
        *succ = false;
    }
    RET_OK(old);
}

RtResult<int64_t> SystemThreadingInterlocked::compare_exchange_i64(int64_t* location, int64_t value, int64_t comparand) noexcept
{
    int64_t old = *location;
    if (old == comparand)
    {
        *location = value;
    }
    RET_OK(old);
}

RtResult<intptr_t> SystemThreadingInterlocked::compare_exchange_intptr(intptr_t* location, intptr_t value, intptr_t comparand) noexcept
{
    intptr_t old = *location;
    if (old == comparand)
    {
        *location = value;
    }
    RET_OK(old);
}

RtResultVoid SystemThreadingInterlocked::compare_exchange_object2(RtObject** location, RtObject** value, RtObject** comparand, RtObject** result) noexcept
{
    RtObject* old = *location;
    if (old == *comparand)
    {
        *location = *value;
    }
    *result = old;
    RET_VOID_OK();
}

RtResult<RtObject*> SystemThreadingInterlocked::compare_exchange_object(RtObject** location, RtObject* value, RtObject* comparand) noexcept
{
    RtObject* old = *location;
    if (old == comparand)
    {
        *location = value;
    }
    RET_OK(old);
}

RtResult<float> SystemThreadingInterlocked::compare_exchange_f32(float* location, float value, float comparand) noexcept
{
    float old = *location;
    uint32_t old_bits;
    uint32_t comparand_bits;
    std::memcpy(&old_bits, &old, sizeof(uint32_t));
    std::memcpy(&comparand_bits, &comparand, sizeof(uint32_t));
    if (old_bits == comparand_bits)
    {
        *location = value;
    }
    RET_OK(old);
}

RtResult<double> SystemThreadingInterlocked::compare_exchange_f64(double* location, double value, double comparand) noexcept
{
    double old = *location;
    uint64_t old_bits;
    uint64_t comparand_bits;
    std::memcpy(&old_bits, &old, sizeof(uint64_t));
    std::memcpy(&comparand_bits, &comparand, sizeof(uint64_t));
    if (old_bits == comparand_bits)
    {
        *location = value;
    }
    RET_OK(old);
}

// Exchange operations
RtResult<int32_t> SystemThreadingInterlocked::exchange_i32(int32_t* location, int32_t value) noexcept
{
    int32_t old = *location;
    *location = value;
    RET_OK(old);
}

RtResult<int64_t> SystemThreadingInterlocked::exchange_i64(int64_t* location, int64_t value) noexcept
{
    int64_t old = *location;
    *location = value;
    RET_OK(old);
}

RtResult<intptr_t> SystemThreadingInterlocked::exchange_intptr(intptr_t* location, intptr_t value) noexcept
{
    intptr_t old = *location;
    *location = value;
    RET_OK(old);
}

RtResult<float> SystemThreadingInterlocked::exchange_f32(float* location, float value) noexcept
{
    float old = *location;
    *location = value;
    RET_OK(old);
}

RtResult<double> SystemThreadingInterlocked::exchange_f64(double* location, double value) noexcept
{
    double old = *location;
    *location = value;
    RET_OK(old);
}

RtResultVoid SystemThreadingInterlocked::exchange_object(RtObject** location, RtObject** value, RtObject** result) noexcept
{
    RtObject* old = *location;
    *location = *value;
    *result = old;
    RET_VOID_OK();
}

// Memory barrier
RtResultVoid SystemThreadingInterlocked::memory_barrier_process_wide() noexcept
{
    RET_VOID_OK();
}

// Read
RtResult<int64_t> SystemThreadingInterlocked::read(int64_t* location) noexcept
{
    RET_OK(*location);
}

// ========== Invoker Functions ==========

/// @icall: System.Threading.Interlocked::Add(System.Int32&,System.Int32)
static RtResultVoid add_i32_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    int32_t* location = EvalStackOp::get_param<int32_t*>(params, 0);
    int32_t value = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemThreadingInterlocked::add_i32(location, value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::Add(System.Int64&,System.Int64)
static RtResultVoid add_i64_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    int64_t* location = EvalStackOp::get_param<int64_t*>(params, 0);
    int64_t value = EvalStackOp::get_param<int64_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, result, SystemThreadingInterlocked::add_i64(location, value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::Increment(System.Int32&)
static RtResultVoid increment_i32_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    int32_t* location = EvalStackOp::get_param<int32_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemThreadingInterlocked::increment_i32(location));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::Increment(System.Int64&)
static RtResultVoid increment_i64_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    int64_t* location = EvalStackOp::get_param<int64_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, result, SystemThreadingInterlocked::increment_i64(location));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::Decrement(System.Int32&)
static RtResultVoid decrement_i32_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    int32_t* location = EvalStackOp::get_param<int32_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemThreadingInterlocked::decrement_i32(location));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::Decrement(System.Int64&)
static RtResultVoid decrement_i64_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    int64_t* location = EvalStackOp::get_param<int64_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, result, SystemThreadingInterlocked::decrement_i64(location));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::CompareExchange(System.Int32&,System.Int32,System.Int32)
static RtResultVoid compare_exchange_i32_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    int32_t* location = EvalStackOp::get_param<int32_t*>(params, 0);
    int32_t value = EvalStackOp::get_param<int32_t>(params, 1);
    int32_t comparand = EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemThreadingInterlocked::compare_exchange_i32(location, value, comparand));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::CompareExchange(System.Int32&,System.Int32,System.Int32,System.Boolean&)
static RtResultVoid compare_exchange2_i32_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    int32_t* location = EvalStackOp::get_param<int32_t*>(params, 0);
    int32_t value = EvalStackOp::get_param<int32_t>(params, 1);
    int32_t comparand = EvalStackOp::get_param<int32_t>(params, 2);
    bool* succ_ptr = EvalStackOp::get_param<bool*>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemThreadingInterlocked::compare_exchange2_i32(location, value, comparand, succ_ptr));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::CompareExchange(System.Int64&,System.Int64,System.Int64)
static RtResultVoid compare_exchange_i64_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    int64_t* location = EvalStackOp::get_param<int64_t*>(params, 0);
    int64_t value = EvalStackOp::get_param<int64_t>(params, 1);
    int64_t comparand = EvalStackOp::get_param<int64_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, result, SystemThreadingInterlocked::compare_exchange_i64(location, value, comparand));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::CompareExchange(System.IntPtr&,System.IntPtr,System.IntPtr)
static RtResultVoid compare_exchange_intptr_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    intptr_t* location = EvalStackOp::get_param<intptr_t*>(params, 0);
    intptr_t value = EvalStackOp::get_param<intptr_t>(params, 1);
    intptr_t comparand = EvalStackOp::get_param<intptr_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, result, SystemThreadingInterlocked::compare_exchange_intptr(location, value, comparand));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::CompareExchange(System.Object&,System.Object&,System.Object&,System.Object&)
static RtResultVoid compare_exchange_object2_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    (void)ret;
    RtObject** location_ptr = EvalStackOp::get_param<RtObject**>(params, 0);
    RtObject** value_ptr = EvalStackOp::get_param<RtObject**>(params, 1);
    RtObject** comparand_ptr = EvalStackOp::get_param<RtObject**>(params, 2);
    RtObject** result_ptr = EvalStackOp::get_param<RtObject**>(params, 3);
    return SystemThreadingInterlocked::compare_exchange_object2(location_ptr, value_ptr, comparand_ptr, result_ptr);
}

/// @icall: System.Threading.Interlocked::CompareExchange(System.Object&,System.Object,System.Object)
static RtResultVoid compare_exchange_object_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    RtObject** location = EvalStackOp::get_param<RtObject**>(params, 0);
    RtObject* value = EvalStackOp::get_param<RtObject*>(params, 1);
    RtObject* comparand = EvalStackOp::get_param<RtObject*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, result, SystemThreadingInterlocked::compare_exchange_object(location, value, comparand));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::CompareExchange(System.Single&,System.Single,System.Single)
static RtResultVoid compare_exchange_f32_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    float* location = EvalStackOp::get_param<float*>(params, 0);
    float value = EvalStackOp::get_param<float>(params, 1);
    float comparand = EvalStackOp::get_param<float>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemThreadingInterlocked::compare_exchange_f32(location, value, comparand));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::CompareExchange(System.Double&,System.Double,System.Double)
static RtResultVoid compare_exchange_f64_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    double* location = EvalStackOp::get_param<double*>(params, 0);
    double value = EvalStackOp::get_param<double>(params, 1);
    double comparand = EvalStackOp::get_param<double>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemThreadingInterlocked::compare_exchange_f64(location, value, comparand));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::Exchange(System.Int32&,System.Int32)
static RtResultVoid exchange_i32_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    int32_t* location = EvalStackOp::get_param<int32_t*>(params, 0);
    int32_t value = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemThreadingInterlocked::exchange_i32(location, value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::Exchange(System.Int64&,System.Int64)
static RtResultVoid exchange_i64_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    int64_t* location = EvalStackOp::get_param<int64_t*>(params, 0);
    int64_t value = EvalStackOp::get_param<int64_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, result, SystemThreadingInterlocked::exchange_i64(location, value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::Exchange(System.IntPtr&,System.IntPtr)
static RtResultVoid exchange_intptr_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    intptr_t* location = EvalStackOp::get_param<intptr_t*>(params, 0);
    intptr_t value = EvalStackOp::get_param<intptr_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, result, SystemThreadingInterlocked::exchange_intptr(location, value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::Exchange(System.Single&,System.Single)
static RtResultVoid exchange_f32_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    float* location = EvalStackOp::get_param<float*>(params, 0);
    float value = EvalStackOp::get_param<float>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, result, SystemThreadingInterlocked::exchange_f32(location, value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::Exchange(System.Double&,System.Double)
static RtResultVoid exchange_f64_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    double* location = EvalStackOp::get_param<double*>(params, 0);
    double value = EvalStackOp::get_param<double>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, result, SystemThreadingInterlocked::exchange_f64(location, value));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Interlocked::Exchange(System.Object&,System.Object&,System.Object&)
static RtResultVoid exchange_object_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    (void)ret;
    RtObject** location_ptr = EvalStackOp::get_param<RtObject**>(params, 0);
    RtObject** value_ptr = EvalStackOp::get_param<RtObject**>(params, 1);
    RtObject** result_ptr = EvalStackOp::get_param<RtObject**>(params, 2);
    return SystemThreadingInterlocked::exchange_object(location_ptr, value_ptr, result_ptr);
}

/// @icall: System.Threading.Interlocked::MemoryBarrierProcessWide()
static RtResultVoid memory_barrier_process_wide_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    (void)params;
    (void)ret;
    return SystemThreadingInterlocked::memory_barrier_process_wide();
}

/// @icall: System.Threading.Interlocked::Read(System.Int64&)
static RtResultVoid interlocked_read_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    int64_t* location = EvalStackOp::get_param<int64_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, result, SystemThreadingInterlocked::read(location));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

// ========== Registration ==========

static InternalCallEntry s_internal_call_entries_system_threading_interlocked[] = {
    {"System.Threading.Interlocked::Add(System.Int32&,System.Int32)", (InternalCallFunction)&SystemThreadingInterlocked::add_i32, add_i32_invoker},
    {"System.Threading.Interlocked::Add(System.Int64&,System.Int64)", (InternalCallFunction)&SystemThreadingInterlocked::add_i64, add_i64_invoker},
    {"System.Threading.Interlocked::Increment(System.Int32&)", (InternalCallFunction)&SystemThreadingInterlocked::increment_i32, increment_i32_invoker},
    {"System.Threading.Interlocked::Increment(System.Int64&)", (InternalCallFunction)&SystemThreadingInterlocked::increment_i64, increment_i64_invoker},
    {"System.Threading.Interlocked::Decrement(System.Int32&)", (InternalCallFunction)&SystemThreadingInterlocked::decrement_i32, decrement_i32_invoker},
    {"System.Threading.Interlocked::Decrement(System.Int64&)", (InternalCallFunction)&SystemThreadingInterlocked::decrement_i64, decrement_i64_invoker},
    {"System.Threading.Interlocked::CompareExchange(System.Int32&,System.Int32,System.Int32)",
     (InternalCallFunction)&SystemThreadingInterlocked::compare_exchange_i32, compare_exchange_i32_invoker},
    {"System.Threading.Interlocked::CompareExchange(System.Int32&,System.Int32,System.Int32,System.Boolean&)",
     (InternalCallFunction)&SystemThreadingInterlocked::compare_exchange2_i32, compare_exchange2_i32_invoker},
    {"System.Threading.Interlocked::CompareExchange(System.Int64&,System.Int64,System.Int64)",
     (InternalCallFunction)&SystemThreadingInterlocked::compare_exchange_i64, compare_exchange_i64_invoker},
    {"System.Threading.Interlocked::CompareExchange(System.IntPtr&,System.IntPtr,System.IntPtr)",
     (InternalCallFunction)&SystemThreadingInterlocked::compare_exchange_intptr, compare_exchange_intptr_invoker},
    {"System.Threading.Interlocked::CompareExchange(System.Object&,System.Object&,System.Object&,System.Object&)",
     (InternalCallFunction)&SystemThreadingInterlocked::compare_exchange_object2, compare_exchange_object2_invoker},
    {"System.Threading.Interlocked::CompareExchange(System.Object&,System.Object,System.Object)",
     (InternalCallFunction)&SystemThreadingInterlocked::compare_exchange_object, compare_exchange_object_invoker},
    {"System.Threading.Interlocked::CompareExchange(System.Single&,System.Single,System.Single)",
     (InternalCallFunction)&SystemThreadingInterlocked::compare_exchange_f32, compare_exchange_f32_invoker},
    {"System.Threading.Interlocked::CompareExchange(System.Double&,System.Double,System.Double)",
     (InternalCallFunction)&SystemThreadingInterlocked::compare_exchange_f64, compare_exchange_f64_invoker},
    {"System.Threading.Interlocked::Exchange(System.Int32&,System.Int32)", (InternalCallFunction)&SystemThreadingInterlocked::exchange_i32,
     exchange_i32_invoker},
    {"System.Threading.Interlocked::Exchange(System.Int64&,System.Int64)", (InternalCallFunction)&SystemThreadingInterlocked::exchange_i64,
     exchange_i64_invoker},
    {"System.Threading.Interlocked::Exchange(System.IntPtr&,System.IntPtr)", (InternalCallFunction)&SystemThreadingInterlocked::exchange_intptr,
     exchange_intptr_invoker},
    {"System.Threading.Interlocked::Exchange(System.Single&,System.Single)", (InternalCallFunction)&SystemThreadingInterlocked::exchange_f32,
     exchange_f32_invoker},
    {"System.Threading.Interlocked::Exchange(System.Double&,System.Double)", (InternalCallFunction)&SystemThreadingInterlocked::exchange_f64,
     exchange_f64_invoker},
    {"System.Threading.Interlocked::Exchange(System.Object&,System.Object&,System.Object&)", (InternalCallFunction)&SystemThreadingInterlocked::exchange_object,
     exchange_object_invoker},
    {"System.Threading.Interlocked::MemoryBarrierProcessWide()", (InternalCallFunction)&SystemThreadingInterlocked::memory_barrier_process_wide,
     memory_barrier_process_wide_invoker},
    {"System.Threading.Interlocked::Read(System.Int64&)", (InternalCallFunction)&SystemThreadingInterlocked::read, interlocked_read_invoker},
};

utils::Span<InternalCallEntry> SystemThreadingInterlocked::get_internal_call_entries() noexcept
{
    constexpr size_t entry_count =
        sizeof(s_internal_call_entries_system_threading_interlocked) / sizeof(s_internal_call_entries_system_threading_interlocked[0]);
    return utils::Span<InternalCallEntry>(s_internal_call_entries_system_threading_interlocked, entry_count);
}

} // namespace icalls
} // namespace leanclr

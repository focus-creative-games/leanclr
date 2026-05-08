#include "system_threading_thread.h"

#include "icall_base.h"
#include "vm/rt_thread.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"
#include "vm/appdomain.h"
#include "utils/string_util.h"

#include <cstring>

namespace leanclr
{
namespace icalls
{

// Basic thread operations
RtResultVoid SystemThreadingThread::get_current_thread_icall(vm::RtThread** thread) noexcept
{
    *thread = vm::Thread::get_current_thread();
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::reset_abort_native(vm::RtThread* this_thread) noexcept
{
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::suspend_internal(vm::RtThread* this_thread) noexcept
{
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::resume_internal(vm::RtThread* this_thread) noexcept
{
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::interrupt_internal(vm::RtThread* this_thread) noexcept
{
    RET_VOID_OK();
}

RtResult<int32_t> SystemThreadingThread::get_priority_native(vm::RtThread* this_thread) noexcept
{
    RET_OK(vm::Thread::get_priority_native(this_thread));
}

RtResultVoid SystemThreadingThread::set_priority_native(vm::RtThread* this_thread, int32_t priority) noexcept
{
    vm::Thread::set_priority_native(this_thread, priority);
    RET_VOID_OK();
}

RtResult<bool> SystemThreadingThread::join_internal(vm::RtThread* this_thread, int32_t milliseconds) noexcept
{
    RET_OK(true);
}

RtResultVoid SystemThreadingThread::sleep_internal(int32_t milliseconds) noexcept
{
    vm::Thread::sleep(milliseconds);
    RET_VOID_OK();
}

RtResult<bool> SystemThreadingThread::yield_internal() noexcept
{
    RET_OK(vm::Thread::yield_internal());
}

RtResultVoid SystemThreadingThread::memory_barrier() noexcept
{
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::construct_internal_thread(vm::RtThread* this_thread) noexcept
{
    vm::Thread::construct_internal_thread(this_thread);
    RET_VOID_OK();
}

// Array domain conversions
RtResult<vm::RtArray*> SystemThreadingThread::byte_array_to_root_domain(vm::RtArray* arr) noexcept
{
    RET_OK(arr);
}

RtResult<vm::RtArray*> SystemThreadingThread::byte_array_to_current_domain(vm::RtArray* arr) noexcept
{
    RET_OK(arr);
}

// AppDomain access
RtResult<int32_t> SystemThreadingThread::get_domain_id() noexcept
{
    RET_OK(vm::AppDomain::get_appdomain_id());
}

// Thread initialization
RtResult<bool> SystemThreadingThread::thread_internal(vm::RtThread* this_thread, vm::RtMulticastDelegate* start) noexcept
{
    RET_OK(vm::Thread::start_thread(this_thread, start));
}

// Thread name operations
RtResult<vm::RtString*> SystemThreadingThread::get_name_internal(vm::RtInternalThread* internal_thread) noexcept
{
    const Utf16Char* name = internal_thread->name_chars;
    int32_t name_length = internal_thread->name_length;
    vm::RtString* name_str = vm::String::create_string_from_utf16chars(name, name_length);
    RET_OK(name_str);
}

RtResultVoid SystemThreadingThread::set_name_icall(vm::RtInternalThread* internal_thread, const Utf16Char* name, int32_t len) noexcept
{
    if (internal_thread->name_chars != nullptr)
    {
        RET_ERR(RtErr::InvalidOperation);
    }
    if (len < 0)
    {
        RET_ERR(RtErr::ArgumentOutOfRange);
    }
    // Duplicate the UTF-16 string without null terminator
    internal_thread->name_chars = utils::StringUtil::strdup_utf16_without_null_terminator(name, static_cast<size_t>(len));
    internal_thread->name_length = len;
    RET_VOID_OK();
}

// Thread abort
RtResultVoid SystemThreadingThread::abort_internal(vm::RtObject* internal_thread, vm::RtObject* state) noexcept
{
    vm::Thread::abort(static_cast<vm::RtThread*>(internal_thread));
    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemThreadingThread::get_abort_exception_state() noexcept
{
    return vm::Thread::get_abort_exception_state();
}

// Spin wait
RtResultVoid SystemThreadingThread::spin_wait_nop() noexcept
{
    RET_VOID_OK();
}

// Thread state management
RtResultVoid SystemThreadingThread::set_state(vm::RtInternalThread* internal_thread, int32_t state) noexcept
{
    vm::Thread::set_state(internal_thread, static_cast<vm::RtThreadState>(state));
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::clr_state(vm::RtInternalThread* internal_thread, int32_t state) noexcept
{
    vm::Thread::clear_state(internal_thread, static_cast<vm::RtThreadState>(state));
    RET_VOID_OK();
}

RtResult<int32_t> SystemThreadingThread::get_state(vm::RtInternalThread* internal_thread) noexcept
{
    RET_OK(static_cast<int32_t>(vm::Thread::get_state(internal_thread)));
}

// Volatile read operations
RtResult<uint8_t> SystemThreadingThread::volatile_read_u8(uint8_t* loc) noexcept
{
    RET_OK(*loc);
}

RtResult<int8_t> SystemThreadingThread::volatile_read_i8(int8_t* loc) noexcept
{
    RET_OK(*loc);
}

RtResult<uint16_t> SystemThreadingThread::volatile_read_u16(uint16_t* loc) noexcept
{
    RET_OK(*loc);
}

RtResult<int16_t> SystemThreadingThread::volatile_read_i16(int16_t* loc) noexcept
{
    RET_OK(*loc);
}

RtResult<uint32_t> SystemThreadingThread::volatile_read_u32(uint32_t* loc) noexcept
{
    RET_OK(*loc);
}

RtResult<int32_t> SystemThreadingThread::volatile_read_i32(int32_t* loc) noexcept
{
    RET_OK(*loc);
}

RtResult<uint64_t> SystemThreadingThread::volatile_read_u64(uint64_t* loc) noexcept
{
    RET_OK(*loc);
}

RtResult<int64_t> SystemThreadingThread::volatile_read_i64(int64_t* loc) noexcept
{
    RET_OK(*loc);
}

RtResult<float> SystemThreadingThread::volatile_read_f32(float* loc) noexcept
{
    RET_OK(*loc);
}

RtResult<double> SystemThreadingThread::volatile_read_f64(double* loc) noexcept
{
    RET_OK(*loc);
}

RtResult<intptr_t> SystemThreadingThread::volatile_read_intptr(intptr_t* loc) noexcept
{
    RET_OK(*loc);
}

RtResult<uintptr_t> SystemThreadingThread::volatile_read_uintptr(uintptr_t* loc) noexcept
{
    RET_OK(*loc);
}

RtResult<vm::RtObject*> SystemThreadingThread::volatile_read_object(vm::RtObject** loc) noexcept
{
    RET_OK(*loc);
}

// Volatile write operations
RtResultVoid SystemThreadingThread::volatile_write_u8(uint8_t* loc, uint8_t val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::volatile_write_i8(int8_t* loc, int8_t val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::volatile_write_u16(uint16_t* loc, uint16_t val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::volatile_write_i16(int16_t* loc, int16_t val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::volatile_write_u32(uint32_t* loc, uint32_t val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::volatile_write_i32(int32_t* loc, int32_t val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::volatile_write_u64(uint64_t* loc, uint64_t val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::volatile_write_i64(int64_t* loc, int64_t val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::volatile_write_f32(float* loc, float val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::volatile_write_f64(double* loc, double val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::volatile_write_intptr(intptr_t* loc, intptr_t val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::volatile_write_uintptr(uintptr_t* loc, uintptr_t val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

RtResultVoid SystemThreadingThread::volatile_write_object(vm::RtObject** loc, vm::RtObject* val) noexcept
{
    *loc = val;
    RET_VOID_OK();
}

// Stack size
RtResult<int32_t> SystemThreadingThread::system_max_stack_stize() noexcept
{
    RET_OK(vm::Thread::get_system_max_stack_size());
}

// Stack traces
RtResultVoid SystemThreadingThread::get_stack_traces(vm::RtArray** threads, vm::RtArray** stack_frames) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

// Invoker functions
/// @icall: System.Threading.Thread::GetCurrentThread_icall
static RtResultVoid get_current_thread_icall_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto thread_ref_ptr = EvalStackOp::get_param<vm::RtThread**>(params, 0);
    RET_ERR_ON_FAIL(SystemThreadingThread::get_current_thread_icall(thread_ref_ptr));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::ResetAbortNative
static RtResultVoid reset_abort_native_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto this_thread = EvalStackOp::get_param<vm::RtThread*>(params, 0);
    RET_ERR_ON_FAIL(SystemThreadingThread::reset_abort_native(this_thread));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::SuspendInternal
static RtResultVoid suspend_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto this_thread = EvalStackOp::get_param<vm::RtThread*>(params, 0);
    RET_ERR_ON_FAIL(SystemThreadingThread::suspend_internal(this_thread));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::ResumeInternal
static RtResultVoid resume_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto this_thread = EvalStackOp::get_param<vm::RtThread*>(params, 0);
    RET_ERR_ON_FAIL(SystemThreadingThread::resume_internal(this_thread));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::InterruptInternal
static RtResultVoid interrupt_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto this_thread = EvalStackOp::get_param<vm::RtThread*>(params, 0);
    RET_ERR_ON_FAIL(SystemThreadingThread::interrupt_internal(this_thread));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::GetPriorityNative
static RtResultVoid get_priority_native_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto this_thread = EvalStackOp::get_param<vm::RtThread*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, priority, SystemThreadingThread::get_priority_native(this_thread));
    EvalStackOp::set_return(ret, priority);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::SetPriorityNative(System.Int32)
static RtResultVoid set_priority_native_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto this_thread = EvalStackOp::get_param<vm::RtThread*>(params, 0);
    auto priority = EvalStackOp::get_param<int32_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::set_priority_native(this_thread, priority));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::JoinInternal(System.Int32)
static RtResultVoid join_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    auto this_thread = EvalStackOp::get_param<vm::RtThread*>(params, 0);
    auto ms = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemThreadingThread::join_internal(this_thread, ms));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::SleepInternal(System.Int32)
static RtResultVoid sleep_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto ms = EvalStackOp::get_param<int32_t>(params, 0);
    RET_ERR_ON_FAIL(SystemThreadingThread::sleep_internal(ms));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::YieldInternal
static RtResultVoid yield_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemThreadingThread::yield_internal());
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::MemoryBarrier
static RtResultVoid memory_barrier_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    RET_ERR_ON_FAIL(SystemThreadingThread::memory_barrier());
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::ConstructInternalThread
static RtResultVoid construct_internal_thread_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                      const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto this_thread = EvalStackOp::get_param<vm::RtThread*>(params, 0);
    RET_ERR_ON_FAIL(SystemThreadingThread::construct_internal_thread(this_thread));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::ByteArrayToRootDomain(System.Byte[])
static RtResultVoid byte_array_to_root_domain_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                      const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result, SystemThreadingThread::byte_array_to_root_domain(arr));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::ByteArrayToCurrentDomain(System.Byte[])
static RtResultVoid byte_array_to_current_domain_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result, SystemThreadingThread::byte_array_to_current_domain(arr));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::GetDomainID
static RtResultVoid get_domain_id_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, id, SystemThreadingThread::get_domain_id());
    EvalStackOp::set_return(ret, id);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::Thread_internal(System.MulticastDelegate)
static RtResultVoid thread_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto this_thread = EvalStackOp::get_param<vm::RtThread*>(params, 0);
    auto start = EvalStackOp::get_param<vm::RtMulticastDelegate*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemThreadingThread::thread_internal(this_thread, start));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::GetName_internal(System.Threading.InternalThread)
static RtResultVoid get_name_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto internal_thread = EvalStackOp::get_param<vm::RtInternalThread*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, name, SystemThreadingThread::get_name_internal(internal_thread));
    EvalStackOp::set_return(ret, name);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::SetName_icall(System.Threading.InternalThread,System.Char*,System.Int32)
static RtResultVoid set_name_icall_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto internal_thread = EvalStackOp::get_param<vm::RtInternalThread*>(params, 0);
    auto name = EvalStackOp::get_param<const uint16_t*>(params, 1);
    auto len = EvalStackOp::get_param<int32_t>(params, 2);
    RET_ERR_ON_FAIL(SystemThreadingThread::set_name_icall(internal_thread, name, len));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::Abort_internal(System.Threading.InternalThread,System.Object)
static RtResultVoid abort_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto internal_thread = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    auto state = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::abort_internal(internal_thread, state));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::GetAbortExceptionState
static RtResultVoid get_abort_exception_state_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                      const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemThreadingThread::get_abort_exception_state());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::SpinWait_nop
static RtResultVoid spin_wait_nop_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    RET_ERR_ON_FAIL(SystemThreadingThread::spin_wait_nop());
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::SetState(System.Threading.InternalThread,System.Threading.ThreadState)
static RtResultVoid set_state_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto internal_thread = EvalStackOp::get_param<vm::RtInternalThread*>(params, 0);
    auto state = EvalStackOp::get_param<int32_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::set_state(internal_thread, state));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::ClrState(System.Threading.InternalThread,System.Threading.ThreadState)
static RtResultVoid clr_state_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto internal_thread = EvalStackOp::get_param<vm::RtInternalThread*>(params, 0);
    auto state = EvalStackOp::get_param<int32_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::clr_state(internal_thread, state));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::GetState(System.Threading.InternalThread)
static RtResultVoid get_state_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto internal_thread = EvalStackOp::get_param<vm::RtInternalThread*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, state, SystemThreadingThread::get_state(internal_thread));
    EvalStackOp::set_return(ret, state);
    RET_VOID_OK();
}

// Volatile read invokers
/// @icall: System.Threading.Thread::VolatileRead(System.Byte&)
static RtResultVoid volatile_read_u8_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<uint8_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint8_t, v, SystemThreadingThread::volatile_read_u8(p));
    EvalStackOp::set_return(ret, static_cast<int32_t>(v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileRead(System.SByte&)
static RtResultVoid volatile_read_i8_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<int8_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int8_t, v, SystemThreadingThread::volatile_read_i8(p));
    EvalStackOp::set_return(ret, static_cast<int32_t>(v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileRead(System.UInt16&)
static RtResultVoid volatile_read_u16_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<uint16_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint16_t, v, SystemThreadingThread::volatile_read_u16(p));
    EvalStackOp::set_return(ret, static_cast<int32_t>(v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileRead(System.Int16&)
static RtResultVoid volatile_read_i16_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<int16_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int16_t, v, SystemThreadingThread::volatile_read_i16(p));
    EvalStackOp::set_return(ret, static_cast<int32_t>(v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileRead(System.UInt32&)
static RtResultVoid volatile_read_u32_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<uint32_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, v, SystemThreadingThread::volatile_read_u32(p));
    EvalStackOp::set_return(ret, static_cast<int32_t>(v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileRead(System.Int32&)
static RtResultVoid volatile_read_i32_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<int32_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, v, SystemThreadingThread::volatile_read_i32(p));
    EvalStackOp::set_return(ret, v);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileRead(System.UInt64&)
static RtResultVoid volatile_read_u64_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<uint64_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint64_t, v, SystemThreadingThread::volatile_read_u64(p));
    EvalStackOp::set_return(ret, v);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileRead(System.Int64&)
static RtResultVoid volatile_read_i64_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<int64_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int64_t, v, SystemThreadingThread::volatile_read_i64(p));
    EvalStackOp::set_return(ret, v);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileRead(System.Single&)
static RtResultVoid volatile_read_f32_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<float*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(float, v, SystemThreadingThread::volatile_read_f32(p));
    EvalStackOp::set_return(ret, v);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileRead(System.Double&)
static RtResultVoid volatile_read_f64_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<double*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(double, v, SystemThreadingThread::volatile_read_f64(p));
    EvalStackOp::set_return(ret, v);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileRead(System.IntPtr&)
static RtResultVoid volatile_read_intptr_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<intptr_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, v, SystemThreadingThread::volatile_read_intptr(p));
    EvalStackOp::set_return(ret, v);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileRead(System.UIntPtr&)
static RtResultVoid volatile_read_uintptr_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<uintptr_t*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uintptr_t, v, SystemThreadingThread::volatile_read_uintptr(p));
    EvalStackOp::set_return(ret, v);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileRead(System.Object&)
static RtResultVoid volatile_read_object_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<vm::RtObject**>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, v, SystemThreadingThread::volatile_read_object(p));
    EvalStackOp::set_return(ret, v);
    RET_VOID_OK();
}

// Volatile write invokers
/// @icall: System.Threading.Thread::VolatileWrite(System.Byte&,System.Byte)
static RtResultVoid volatile_write_u8_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<uint8_t*>(params, 0);
    auto v = EvalStackOp::get_param<uint8_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_u8(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileWrite(System.SByte&,System.SByte)
static RtResultVoid volatile_write_i8_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<int8_t*>(params, 0);
    auto v = EvalStackOp::get_param<int8_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_i8(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileWrite(System.UInt16&,System.UInt16)
static RtResultVoid volatile_write_u16_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<uint16_t*>(params, 0);
    auto v = EvalStackOp::get_param<uint16_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_u16(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileWrite(System.Int16&,System.Int16)
static RtResultVoid volatile_write_i16_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<int16_t*>(params, 0);
    auto v = EvalStackOp::get_param<int16_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_i16(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileWrite(System.UInt32&,System.UInt32)
static RtResultVoid volatile_write_u32_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<uint32_t*>(params, 0);
    auto v = EvalStackOp::get_param<uint32_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_u32(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileWrite(System.Int32&,System.Int32)
static RtResultVoid volatile_write_i32_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<int32_t*>(params, 0);
    auto v = EvalStackOp::get_param<int32_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_i32(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileWrite(System.UInt64&,System.UInt64)
static RtResultVoid volatile_write_u64_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<uint64_t*>(params, 0);
    auto v = EvalStackOp::get_param<uint64_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_u64(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileWrite(System.Int64&,System.Int64)
static RtResultVoid volatile_write_i64_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<int64_t*>(params, 0);
    auto v = EvalStackOp::get_param<int64_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_i64(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileWrite(System.Single&,System.Single)
static RtResultVoid volatile_write_f32_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<float*>(params, 0);
    auto v = EvalStackOp::get_param<float>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_f32(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileWrite(System.Double&,System.Double)
static RtResultVoid volatile_write_f64_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<double*>(params, 0);
    auto v = EvalStackOp::get_param<double>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_f64(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileWrite(System.IntPtr&,System.IntPtr)
static RtResultVoid volatile_write_intptr_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<intptr_t*>(params, 0);
    auto v = EvalStackOp::get_param<intptr_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_intptr(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileWrite(System.UIntPtr&,System.UIntPtr)
static RtResultVoid volatile_write_uintptr_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<uintptr_t*>(params, 0);
    auto v = EvalStackOp::get_param<uintptr_t>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_uintptr(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::VolatileWrite(System.Object&,System.Object)
static RtResultVoid volatile_write_object_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto p = EvalStackOp::get_param<vm::RtObject**>(params, 0);
    auto v = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::volatile_write_object(p, v));
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::SystemMaxStackStize
static RtResultVoid system_max_stack_stize_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, size, SystemThreadingThread::system_max_stack_stize());
    EvalStackOp::set_return(ret, size);
    RET_VOID_OK();
}

/// @icall: System.Threading.Thread::GetStackTraces(System.Threading.Thread[]&,System.Object[]&)
static RtResultVoid get_stack_traces_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto threads = EvalStackOp::get_param<vm::RtArray**>(params, 0);
    auto stack_frames = EvalStackOp::get_param<vm::RtArray**>(params, 1);
    RET_ERR_ON_FAIL(SystemThreadingThread::get_stack_traces(threads, stack_frames));
    RET_VOID_OK();
}

// Internal call registry
static vm::InternalCallEntry s_internal_call_entries_system_threading_thread[] = {
    {"System.Threading.Thread::GetCurrentThread_icall", (vm::InternalCallFunction)&SystemThreadingThread::get_current_thread_icall,
     get_current_thread_icall_invoker},
    {"System.Threading.Thread::ResetAbortNative", (vm::InternalCallFunction)&SystemThreadingThread::reset_abort_native, reset_abort_native_invoker},
    {"System.Threading.Thread::SuspendInternal", (vm::InternalCallFunction)&SystemThreadingThread::suspend_internal, suspend_internal_invoker},
    {"System.Threading.Thread::ResumeInternal", (vm::InternalCallFunction)&SystemThreadingThread::resume_internal, resume_internal_invoker},
    {"System.Threading.Thread::InterruptInternal", (vm::InternalCallFunction)&SystemThreadingThread::interrupt_internal, interrupt_internal_invoker},
    {"System.Threading.Thread::GetPriorityNative", (vm::InternalCallFunction)&SystemThreadingThread::get_priority_native, get_priority_native_invoker},
    {"System.Threading.Thread::SetPriorityNative(System.Int32)", (vm::InternalCallFunction)&SystemThreadingThread::set_priority_native,
     set_priority_native_invoker},
    {"System.Threading.Thread::JoinInternal(System.Int32)", (vm::InternalCallFunction)&SystemThreadingThread::join_internal, join_internal_invoker},
    {"System.Threading.Thread::SleepInternal(System.Int32)", (vm::InternalCallFunction)&SystemThreadingThread::sleep_internal, sleep_internal_invoker},
    {"System.Threading.Thread::YieldInternal", (vm::InternalCallFunction)&SystemThreadingThread::yield_internal, yield_internal_invoker},
    {"System.Threading.Thread::MemoryBarrier", (vm::InternalCallFunction)&SystemThreadingThread::memory_barrier, memory_barrier_invoker},
    {"System.Threading.Thread::ConstructInternalThread", (vm::InternalCallFunction)&SystemThreadingThread::construct_internal_thread,
     construct_internal_thread_invoker},
    {"System.Threading.Thread::ByteArrayToRootDomain(System.Byte[])", (vm::InternalCallFunction)&SystemThreadingThread::byte_array_to_root_domain,
     byte_array_to_root_domain_invoker},
    {"System.Threading.Thread::ByteArrayToCurrentDomain(System.Byte[])", (vm::InternalCallFunction)&SystemThreadingThread::byte_array_to_current_domain,
     byte_array_to_current_domain_invoker},
    {"System.Threading.Thread::GetDomainID", (vm::InternalCallFunction)&SystemThreadingThread::get_domain_id, get_domain_id_invoker},
    {"System.Threading.Thread::Thread_internal(System.MulticastDelegate)", (vm::InternalCallFunction)&SystemThreadingThread::thread_internal,
     thread_internal_invoker},
    {"System.Threading.Thread::GetName_internal(System.Threading.InternalThread)", (vm::InternalCallFunction)&SystemThreadingThread::get_name_internal,
     get_name_internal_invoker},
    {"System.Threading.Thread::SetName_icall(System.Threading.InternalThread,System.Char*,System.Int32)",
     (vm::InternalCallFunction)&SystemThreadingThread::set_name_icall, set_name_icall_invoker},
    {"System.Threading.Thread::Abort_internal(System.Threading.InternalThread,System.Object)", (vm::InternalCallFunction)&SystemThreadingThread::abort_internal,
     abort_internal_invoker},
    {"System.Threading.Thread::GetAbortExceptionState", (vm::InternalCallFunction)&SystemThreadingThread::get_abort_exception_state,
     get_abort_exception_state_invoker},
    {"System.Threading.Thread::SpinWait_nop", (vm::InternalCallFunction)&SystemThreadingThread::spin_wait_nop, spin_wait_nop_invoker},
    {"System.Threading.Thread::SetState(System.Threading.InternalThread,System.Threading.ThreadState)",
     (vm::InternalCallFunction)&SystemThreadingThread::set_state, set_state_invoker},
    {"System.Threading.Thread::ClrState(System.Threading.InternalThread,System.Threading.ThreadState)",
     (vm::InternalCallFunction)&SystemThreadingThread::clr_state, clr_state_invoker},
    {"System.Threading.Thread::GetState(System.Threading.InternalThread)", (vm::InternalCallFunction)&SystemThreadingThread::get_state, get_state_invoker},
    {"System.Threading.Thread::VolatileRead(System.Byte&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_u8, volatile_read_u8_invoker},
    {"System.Threading.Thread::VolatileRead(System.SByte&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_i8, volatile_read_i8_invoker},
    {"System.Threading.Thread::VolatileRead(System.UInt16&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_u16, volatile_read_u16_invoker},
    {"System.Threading.Thread::VolatileRead(System.Int16&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_i16, volatile_read_i16_invoker},
    {"System.Threading.Thread::VolatileRead(System.UInt32&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_u32, volatile_read_u32_invoker},
    {"System.Threading.Thread::VolatileRead(System.Int32&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_i32, volatile_read_i32_invoker},
    {"System.Threading.Thread::VolatileRead(System.UInt64&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_u64, volatile_read_u64_invoker},
    {"System.Threading.Thread::VolatileRead(System.Int64&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_i64, volatile_read_i64_invoker},
    {"System.Threading.Thread::VolatileRead(System.Single&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_f32, volatile_read_f32_invoker},
    {"System.Threading.Thread::VolatileRead(System.Double&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_f64, volatile_read_f64_invoker},
    {"System.Threading.Thread::VolatileRead(System.IntPtr&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_intptr,
     volatile_read_intptr_invoker},
    {"System.Threading.Thread::VolatileRead(System.UIntPtr&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_uintptr,
     volatile_read_uintptr_invoker},
    {"System.Threading.Thread::VolatileRead(System.Object&)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_read_object,
     volatile_read_object_invoker},
    {"System.Threading.Thread::VolatileWrite(System.Byte&,System.Byte)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_u8,
     volatile_write_u8_invoker},
    {"System.Threading.Thread::VolatileWrite(System.SByte&,System.SByte)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_i8,
     volatile_write_i8_invoker},
    {"System.Threading.Thread::VolatileWrite(System.UInt16&,System.UInt16)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_u16,
     volatile_write_u16_invoker},
    {"System.Threading.Thread::VolatileWrite(System.Int16&,System.Int16)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_i16,
     volatile_write_i16_invoker},
    {"System.Threading.Thread::VolatileWrite(System.UInt32&,System.UInt32)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_u32,
     volatile_write_u32_invoker},
    {"System.Threading.Thread::VolatileWrite(System.Int32&,System.Int32)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_i32,
     volatile_write_i32_invoker},
    {"System.Threading.Thread::VolatileWrite(System.UInt64&,System.UInt64)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_u64,
     volatile_write_u64_invoker},
    {"System.Threading.Thread::VolatileWrite(System.Int64&,System.Int64)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_i64,
     volatile_write_i64_invoker},
    {"System.Threading.Thread::VolatileWrite(System.Single&,System.Single)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_f32,
     volatile_write_f32_invoker},
    {"System.Threading.Thread::VolatileWrite(System.Double&,System.Double)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_f64,
     volatile_write_f64_invoker},
    {"System.Threading.Thread::VolatileWrite(System.IntPtr&,System.IntPtr)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_intptr,
     volatile_write_intptr_invoker},
    {"System.Threading.Thread::VolatileWrite(System.UIntPtr&,System.UIntPtr)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_uintptr,
     volatile_write_uintptr_invoker},
    {"System.Threading.Thread::VolatileWrite(System.Object&,System.Object)", (vm::InternalCallFunction)&SystemThreadingThread::volatile_write_object,
     volatile_write_object_invoker},
    {"System.Threading.Thread::SystemMaxStackStize", (vm::InternalCallFunction)&SystemThreadingThread::system_max_stack_stize, system_max_stack_stize_invoker},
    {"System.Threading.Thread::GetStackTraces(System.Threading.Thread[]&,System.Object[]&)", (vm::InternalCallFunction)&SystemThreadingThread::get_stack_traces,
     get_stack_traces_invoker},
};

utils::Span<vm::InternalCallEntry> SystemThreadingThread::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_threading_thread,
                                              sizeof(s_internal_call_entries_system_threading_thread) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

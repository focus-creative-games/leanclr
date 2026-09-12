#include "system_exception.h"

#include "icall_base.h"
#include "vm/rt_exception.h"

namespace leanclr
{
namespace icalls
{

RtResultVoid SystemException::report_unhandled_exception(vm::RtException* exception) noexcept
{
    return vm::Exception::report_unhandled_exception(exception);
}

bool SystemException::is_immutable_agile_exception(vm::RtException* exception) noexcept
{
    (void)exception;
    return false;
}

void SystemException::prepare_for_foreign_exception_raise() noexcept
{
}

vm::RtObject* SystemException::get_frozen_stack_trace(vm::RtException* exception) noexcept
{
    (void)exception;
    return nullptr;
}

uint32_t SystemException::get_exception_count() noexcept
{
    return 0;
}

/// @icall: System.Exception::ReportUnhandledException(System.Exception)
static RtResultVoid report_unhandled_exception_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto exception = EvalStackOp::get_param<vm::RtException*>(params, 0);
    return SystemException::report_unhandled_exception(exception);
}

/// @icall: System.Exception::IsImmutableAgileException(System.Exception)
static RtResultVoid is_immutable_agile_exception_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto exception = EvalStackOp::get_param<vm::RtException*>(params, 0);
    EvalStackOp::set_return(ret, SystemException::is_immutable_agile_exception(exception));
    RET_VOID_OK();
}

/// @icall: System.Exception::PrepareForForeignExceptionRaise()
static RtResultVoid prepare_for_foreign_exception_raise_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    (void)ret;
    SystemException::prepare_for_foreign_exception_raise();
    RET_VOID_OK();
}

/// @icall: System.Exception::GetFrozenStackTrace(System.Exception)
static RtResultVoid get_frozen_stack_trace_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto exception = EvalStackOp::get_param<vm::RtException*>(params, 0);
    EvalStackOp::set_return(ret, SystemException::get_frozen_stack_trace(exception));
    RET_VOID_OK();
}

/// @icall: System.Exception::GetExceptionCount()
static RtResultVoid get_exception_count_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    EvalStackOp::set_return(ret, SystemException::get_exception_count());
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemException::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Exception::ReportUnhandledException(System.Exception)", (vm::InternalCallFunction)&SystemException::report_unhandled_exception,
         report_unhandled_exception_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

utils::Span<vm::InternalCallEntry> SystemException::get_net10_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Exception::IsImmutableAgileException(System.Exception)",
         (vm::InternalCallFunction)&SystemException::is_immutable_agile_exception,
         is_immutable_agile_exception_invoker},
        {"System.Exception::PrepareForForeignExceptionRaise()",
         (vm::InternalCallFunction)&SystemException::prepare_for_foreign_exception_raise,
         prepare_for_foreign_exception_raise_invoker},
        {"System.Exception::GetFrozenStackTrace(System.Exception)",
         (vm::InternalCallFunction)&SystemException::get_frozen_stack_trace,
         get_frozen_stack_trace_invoker},
        {"System.Exception::GetExceptionCount()",
         (vm::InternalCallFunction)&SystemException::get_exception_count,
         get_exception_count_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

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

/// @icall: System.Exception::ReportUnhandledException(System.Exception)
static RtResultVoid report_unhandled_exception_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto exception = EvalStackOp::get_param<vm::RtException*>(params, 0);
    return SystemException::report_unhandled_exception(exception);
}

utils::Span<vm::InternalCallEntry> SystemException::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Exception::ReportUnhandledException(System.Exception)", (vm::InternalCallFunction)&SystemException::report_unhandled_exception,
         report_unhandled_exception_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

#include "rt_exception.h"
#include "gc/gc_roots.h"
#include "stacktrace.h"
#include "settings.h"
#include <cstdlib>

namespace leanclr
{
namespace vm
{
// Execution state is core-owned; the adapter only creates/inspects managed
// exception fields and maps CLR error categories to its BCL exception classes.
static RtException* s_ref_exception = nullptr;

RtException* Exception::get_current_exception()
{
    return s_ref_exception;
}

RtResultVoid Exception::initialize()
{
    RET_VOID_OK();
}

void register_exception_gc_roots()
{
    gc::GcRoots::register_slot(reinterpret_cast<RtObject**>(&s_ref_exception));
}

void Exception::set_current_exception(RtException* ex)
{
    s_ref_exception = ex;
}

RtException* Exception::get_and_clear_current_exception()
{
    RtException* ex = s_ref_exception;
    s_ref_exception = nullptr;
    return ex;
}

RtException* Exception::raise_aot_error_as_exception(RtErr err, const metadata::RtMethodInfo* methodInfo, int32_t ip)
{
    return raise_error_as_exception(err, nullptr, reinterpret_cast<const void*>(static_cast<intptr_t>(ip)));
}

RtException* Exception::raise_aot_exception(RtException* ex, const metadata::RtMethodInfo* methodInfo, int32_t ip)
{
    return raise_exception(ex, nullptr, reinterpret_cast<const void*>(static_cast<intptr_t>(ip)));
}

static void prepare_exception_info(RtException* ex, interp::InterpFrame* frame, const void* ip)
{
    auto ret = StackTrace::setup_trace_ips(ex);
    if (ret.is_err())
    {
        // Failed to setup stack trace, ignore for now
    }
}

RtException* Exception::raise_exception(RtException* ex, interp::InterpFrame* frame, const void* ip)
{
    prepare_exception_info(ex, frame, ip);
    set_current_exception(ex);
    return ex;
}

void Exception::raise_as_cpp_exception(RtException* ex)
{
    RtException* raised = raise_aot_exception(ex, nullptr, -1);
#if (defined(__cpp_exceptions) && (__cpp_exceptions >= 199711L)) || (defined(_CPPUNWIND) && _CPPUNWIND)
    throw AotExceptionWrapper{raised};
#else
    (void)raised;
    (void)report_unhandled_exception(ex);
    std::abort();
#endif
}

RtResultVoid Exception::report_unhandled_exception(RtException* exception)
{
    auto handler = vm::Settings::get_report_unhandled_exception_function();
    if (handler != nullptr)
    {
        handler(exception);
    }
    RET_VOID_OK();
}
} // namespace vm
} // namespace leanclr

#include "rt_exception.h"
#include <cstdlib>
#include "gc/garbage_collector.h"
#include "assembly.h"
#include "class.h"
#include "object.h"
#include "method.h"
#include "rt_string.h"
#include "stacktrace.h"
#include "interp/machine_state.h"
#include "settings.h"

namespace leanclr
{
namespace vm
{

static RtException** s_ref_exceptions = nullptr;

RtResultVoid Exception::initialize()
{
    s_ref_exceptions = reinterpret_cast<RtException**>(gc::GarbageCollector::allocate_fixed_reference_array(1));
    RET_VOID_OK();
}

void Exception::set_current_exception(RtException* ex)
{
    s_ref_exceptions[0] = ex;
}

RtException* Exception::get_and_clear_current_exception()
{
    RtException* ex = s_ref_exceptions[0];
    s_ref_exceptions[0] = nullptr;
    return ex;
}

static RtException* internal_get_current_exception()
{
    return s_ref_exceptions[0];
}

static metadata::RtClass* get_exception_klass_of_runtime_error(RtErr err)
{
    const CorLibTypes& types = Class::get_corlib_types();

    switch (err)
    {
    case RtErr::NotImplemented:
        return types.cls_not_implemented_exception;
    case RtErr::StackOverflow:
        return types.cls_stack_overflow_exception;
    case RtErr::InvalidCast:
        return types.cls_invalid_cast_exception;
    case RtErr::NullReference:
        return types.cls_null_reference_exception;
    case RtErr::ArrayTypeMismatch:
        return types.cls_array_type_mismatch_exception;
    case RtErr::IndexOutOfRange:
        return types.cls_index_out_of_range_exception;
    case RtErr::OutOfMemory:
        return types.cls_out_of_memory_exception;
    case RtErr::Arithmetic:
        return types.cls_arithmetic_exception;
    case RtErr::FieldAccess:
        return types.cls_missing_field_exception;
    case RtErr::MethodAccess:
        return types.cls_missing_method_exception;
    case RtErr::ExecutionEngine:
        return types.cls_execution_engine_exception;
    case RtErr::Argument:
        return types.cls_argument_exception;
    case RtErr::ArgumentNull:
        return types.cls_argument_null_exception;
    case RtErr::ArgumentOutOfRange:
        return types.cls_argument_out_of_range_exception;
    case RtErr::DivideByZero:
        return types.cls_division_by_zero_exception;
    case RtErr::Overflow:
        return types.cls_overflow_exception;
    case RtErr::TypeLoad:
        return types.cls_type_load_exception;
    case RtErr::MissingField:
        return types.cls_missing_field_exception;
    case RtErr::MissingMethod:
        return types.cls_missing_method_exception;
    case RtErr::MissingMember:
        return types.cls_missing_member_exception;
    case RtErr::BadImageFormat:
        return types.cls_bad_image_format_exception;
    case RtErr::EntryPointNotFound:
        return types.cls_entry_point_not_found_exception;
    case RtErr::NotSupported:
        return types.cls_not_supported_exception;
    default:
        assert(false && "Unknown runtime error");
        return types.cls_execution_engine_exception;
    }
}

RtException* Exception::raise_error_as_exception(RtErr err, interp::InterpFrame* frame, const void* ip)
{
    if (err == RtErr::ManagedException)
    {
        return internal_get_current_exception();
    }
    metadata::RtClass* ex_class = get_exception_klass_of_runtime_error(err);
    auto ex_ret = vm::Object::new_object(ex_class);
    if (ex_ret.is_ok())
    {
        RtException* ex = reinterpret_cast<RtException*>(ex_ret.unwrap());
        return raise_exception(ex, frame, ip);
    }
    else
    {
        auto ex_ret2 = vm::Object::new_object(Class::get_corlib_types().cls_execution_engine_exception);
        if (ex_ret2.is_ok())
        {
            return raise_exception(reinterpret_cast<RtException*>(ex_ret2.unwrap()), frame, ip);
        }
        // Failed to create exception object, return nullptr
        return nullptr;
    }
}

RtException* Exception::raise_aot_error_as_exception(RtErr err, const metadata::RtMethodInfo* methodInfo, int32_t ip)
{
    return raise_error_as_exception(err, nullptr, reinterpret_cast<const void*>(static_cast<intptr_t>(ip)));
}

RtException* Exception::raise_aot_exception(RtException* ex, const metadata::RtMethodInfo* methodInfo, int32_t ip)
{
    return raise_exception(ex, nullptr, reinterpret_cast<const void*>(static_cast<intptr_t>(ip)));
}

RtErr Exception::raise_internal_runtime_error_as_exception(RtErr err, const char* message)
{
    raise_error_as_exception(err, nullptr, nullptr);
    return RtErr::ManagedException;
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

RtException* Exception::raise_internal_runtime_exception(metadata::RtClass* ex_class, const char* message)
{
    const CorLibTypes& types = Class::get_corlib_types();
    assert(Class::is_exception_sub_class(ex_class));
    auto ex_ret = vm::Object::new_object(ex_class);
    if (ex_ret.is_ok())
    {
        RtException* ex = reinterpret_cast<RtException*>(ex_ret.unwrap());
        return raise_exception(ex, nullptr, nullptr);
    }
    else
    {
        auto ex_ret2 = vm::Object::new_object(Class::get_corlib_types().cls_execution_engine_exception);
        if (ex_ret2.is_ok())
        {
            return reinterpret_cast<RtException*>(ex_ret2.unwrap());
        }
        // Failed to create exception object, return nullptr
        return nullptr;
    }
    return nullptr;
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

void Exception::format_exception(RtException* ex, utils::StringBuilder& sb)
{
    const metadata::RtClass* klass = ex->klass;
    if (klass->namespaze && klass->namespaze[0] != 0)
    {
        sb.append_cstr(klass->namespaze);
        sb.append_char('.');
    }
    sb.append_cstr(klass->name);
    if (ex->message)
    {
        sb.append_cstr(": ");
        sb.append_utf16_str(vm::String::get_chars_ptr(ex->message), static_cast<size_t>(vm::String::get_length(ex->message)));
    }
    sb.sure_null_terminator_but_not_append();
}

} // namespace vm
} // namespace leanclr

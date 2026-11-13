#include "managed_layouts.h"
#include "vm/rt_exception.h"
#include "vm/class.h"
#include "vm/assembly.h"
#include "metadata/module_def.h"
#include "vm/object.h"
#include "vm/rt_string.h"
#include "vm/stacktrace.h"
#include "interp/machine_state.h"

namespace leanclr
{
namespace vm
{
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
    case RtErr::FileNotFound:
    {
        static metadata::RtClass* file_not_found = nullptr;
        if (!file_not_found)
        {
            auto klass = Assembly::get_corlib()->mod->get_class_by_name2("System.IO", "FileNotFoundException", false, true);
            if (klass.is_err())
                return types.cls_execution_engine_exception;
            auto initialized = Class::initialize_all(klass.unwrap());
            if (initialized.is_err())
                return types.cls_out_of_memory_exception;
            file_not_found = klass.unwrap();
        }
        return file_not_found;
    }
    case RtErr::BadImageFormat:
        return types.cls_bad_image_format_exception;
    case RtErr::EntryPointNotFound:
        return types.cls_entry_point_not_found_exception;
    case RtErr::NotSupported:
        return types.cls_not_supported_exception;
    case RtErr::ModuleAlreadyLoaded:
        return types.cls_execution_engine_exception;
    default:
        assert(false && "Unknown runtime error");
        return types.cls_execution_engine_exception;
    }
}

RtException* Exception::raise_error_as_exception(RtErr err, interp::InterpFrame* frame, const void* ip)
{
    if (std::getenv("LEANCLR_EXCEPTION_TRACE") != nullptr)
    {
        const metadata::RtMethodInfo* method = frame != nullptr ? frame->method : nullptr;
        const char* ns = method != nullptr && method->parent != nullptr && method->parent->namespaze != nullptr ? method->parent->namespaze : "";
        const char* type_name = method != nullptr && method->parent != nullptr && method->parent->name != nullptr ? method->parent->name : "<null>";
        const char* method_name = method != nullptr && method->name != nullptr ? method->name : "<null>";
        int64_t ir_offset = -1;
        if (method != nullptr && method->interp_data != nullptr && method->interp_data->codes != nullptr && ip != nullptr)
        {
            ir_offset = static_cast<int64_t>(static_cast<const uint8_t*>(ip) - method->interp_data->codes);
        }
        std::fprintf(stderr, "leanclr-exception: err=%d method=%s.%s::%s ir=%lld ip=%p\n", static_cast<int>(err), ns, type_name, method_name,
                     static_cast<long long>(ir_offset), ip);
    }
    if (err == RtErr::BadImageFormat && std::getenv("LEANCLR_REFLECTION_TRACE") != nullptr)
    {
        const metadata::RtMethodInfo* method = frame != nullptr ? frame->method : nullptr;
        const char* type_name = method != nullptr && method->parent != nullptr && method->parent->name != nullptr ? method->parent->name : "<null>";
        const char* method_name = method != nullptr && method->name != nullptr ? method->name : "<null>";
        std::fprintf(stderr, "leanclr-exception: BadImageFormat in %s::%s ip=%p\n", type_name, method_name, ip);
    }
    if (err == RtErr::ManagedException)
    {
        return get_current_exception();
    }
    metadata::RtClass* ex_class = get_exception_klass_of_runtime_error(err);
    auto ex_ret = LEANCLR_NEWOBJ_INTERNAL(ex_class, "Exception::raise_error_as_exception");
    if (ex_ret.is_ok())
    {
        RtException* ex = reinterpret_cast<RtException*>(ex_ret.unwrap());
        return raise_exception(ex, frame, ip);
    }
    else
    {
        auto ex_ret2 = LEANCLR_NEWOBJ_INTERNAL(Class::get_corlib_types().cls_execution_engine_exception, "Exception::raise_error_as_exception");
        if (ex_ret2.is_ok())
        {
            return raise_exception(reinterpret_cast<RtException*>(ex_ret2.unwrap()), frame, ip);
        }
        // Failed to create exception object, return nullptr
        return nullptr;
    }
}

RtErr Exception::raise_internal_runtime_error_as_exception(RtErr err, const char* message)
{
    RtException* ex = raise_error_as_exception(err, nullptr, nullptr);
    if (message)
    {
        ex->message = String::create_string_from_utf8cstr(message);
    }
    return RtErr::ManagedException;
}

RtException* Exception::raise_internal_runtime_exception(metadata::RtClass* ex_class, const char* message)
{
    const CorLibTypes& types = Class::get_corlib_types();
    assert(Class::is_exception_sub_class(ex_class));
    auto ex_ret = LEANCLR_NEWOBJ_INTERNAL(ex_class, "Exception::raise_internal_runtime_exception");
    if (ex_ret.is_ok())
    {
        RtException* ex = reinterpret_cast<RtException*>(ex_ret.unwrap());
        return raise_exception(ex, nullptr, nullptr);
    }
    else
    {
        auto ex_ret2 = LEANCLR_NEWOBJ_INTERNAL(Class::get_corlib_types().cls_execution_engine_exception, "Exception::raise_internal_runtime_exception");
        if (ex_ret2.is_ok())
        {
            return reinterpret_cast<RtException*>(ex_ret2.unwrap());
        }
        // Failed to create exception object, return nullptr
        return nullptr;
    }
    return nullptr;
}

void Exception::format_exception(RtException* ex, utils::Utf8StringBuilder& sb)
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

void Exception::set_inner_exception(RtException* exception, RtException* inner)
{
    exception->inner_exception = inner;
}

} // namespace vm
} // namespace leanclr

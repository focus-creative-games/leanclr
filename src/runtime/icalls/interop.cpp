#include "interop.h"

namespace leanclr
{
namespace icalls
{

RtResult<int32_t> Interop::double_to_string(double value, const char* format, char* buffer, int32_t buffer_size)
{
    return ::snprintf(buffer, buffer_size, format, value);
}

/// @icall: Interop/Sys::DoubleToString(System.Double,System.Byte*,System.Byte*,System.Int32)
RtResultVoid double_to_string_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto value = EvalStackOp::get_param<double>(params, 0);
    auto format = EvalStackOp::get_param<const char*>(params, 1);
    auto buffer = EvalStackOp::get_param<char*>(params, 2);
    auto buffer_size = EvalStackOp::get_param<int32_t>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, Interop::double_to_string(value, format, buffer, buffer_size));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_interop_internal_call_entries[] = {
    {"Interop/Sys::DoubleToString(System.Double,System.Byte*,System.Byte*,System.Int32)", (vm::InternalCallFunction)&Interop::double_to_string,
     double_to_string_invoker},
};

utils::Span<vm::InternalCallEntry> Interop::get_internal_call_entries()
{
    return utils::Span<vm::InternalCallEntry>(s_interop_internal_call_entries, sizeof(s_interop_internal_call_entries) / sizeof(vm::InternalCallEntry));
}
} // namespace icalls
} // namespace leanclr

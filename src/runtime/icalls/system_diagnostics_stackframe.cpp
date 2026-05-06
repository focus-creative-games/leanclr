#include "system_diagnostics_stackframe.h"

#include "icall_base.h"
#include "vm/stacktrace.h"

namespace leanclr
{
namespace icalls
{

RtResult<bool> SystemDiagnosticsStackFrame::get_frame_info(int32_t skip, bool need_file_info, vm::RtReflectionMethod** method, int32_t* il_offset,
                                                           int32_t* native_offset, vm::RtString** file_name, int32_t* line_number,
                                                           int32_t* column_number) noexcept
{
    return vm::StackTrace::get_frame_info(skip, need_file_info, method, il_offset, native_offset, file_name, line_number, column_number);
}

/// @icall:
/// System.Diagnostics.StackFrame::get_frame_info(System.Int32,System.Boolean,System.Reflection.MethodBase&,System.Int32&,System.Int32&,System.String&,System.Int32&,System.Int32&)
static RtResultVoid get_frame_info_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto skip = EvalStackOp::get_param<int32_t>(params, 0);
    auto need_file_info = EvalStackOp::get_param<bool>(params, 1);
    auto ref_method_ptr = EvalStackOp::get_param<vm::RtReflectionMethod**>(params, 2);
    auto il_offset = EvalStackOp::get_param<int32_t*>(params, 3);
    auto native_offset = EvalStackOp::get_param<int32_t*>(params, 4);
    auto file_name = EvalStackOp::get_param<vm::RtString**>(params, 5);
    auto line_number = EvalStackOp::get_param<int32_t*>(params, 6);
    auto column_number = EvalStackOp::get_param<int32_t*>(params, 7);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        bool, result,
        SystemDiagnosticsStackFrame::get_frame_info(skip, need_file_info, ref_method_ptr, il_offset, native_offset, file_name, line_number, column_number));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemDiagnosticsStackFrame::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Diagnostics.StackFrame::get_frame_info(System.Int32,System.Boolean,System.Reflection.MethodBase&,System.Int32&,System.Int32&,System.String&,"
         "System.Int32&,System.Int32&)",
         (vm::InternalCallFunction)&SystemDiagnosticsStackFrame::get_frame_info, get_frame_info_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

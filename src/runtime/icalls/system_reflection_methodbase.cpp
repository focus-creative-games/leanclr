#include "system_reflection_methodbase.h"
#include "icall_base.h"
#include "vm/reflection.h"
#include "interp/machine_state.h"

namespace leanclr
{
namespace icalls
{

// ========== Implementation Functions ==========

RtResult<vm::RtReflectionMethod*> SystemReflectionMethodBase::get_current_method() noexcept
{
    interp::InterpFrame* executing_frame = interp::MachineState::get_global_machine_state().get_executing_frame_stack();
    if (executing_frame == nullptr)
    {
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
    const metadata::RtMethodInfo* method = executing_frame->method;
    return vm::Reflection::get_method_reflection_object(method, method->parent);
}

// ========== Invoker Functions ==========

/// @icall: System.Reflection.MethodBase::GetCurrentMethod
static RtResultVoid get_current_method_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, result, SystemReflectionMethodBase::get_current_method());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

// ========== Internal Call Entries ==========

static vm::InternalCallEntry s_internal_call_entries_system_reflection_methodbase[] = {
    {"System.Reflection.MethodBase::GetCurrentMethod", (vm::InternalCallFunction)&SystemReflectionMethodBase::get_current_method, get_current_method_invoker},
};

utils::Span<vm::InternalCallEntry> SystemReflectionMethodBase::get_internal_call_entries() noexcept
{
    constexpr size_t entry_count =
        sizeof(s_internal_call_entries_system_reflection_methodbase) / sizeof(s_internal_call_entries_system_reflection_methodbase[0]);
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_reflection_methodbase, entry_count);
}

} // namespace icalls
} // namespace leanclr

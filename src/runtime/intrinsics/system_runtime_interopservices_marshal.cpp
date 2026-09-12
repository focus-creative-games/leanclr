#include "system_runtime_interopservices_marshal.h"

#include "interp/eval_stack_op.h"
#include "vm/gchandle.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<bool> SystemRuntimeInteropServicesMarshal::is_pinnable(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        RET_OK(true);
    }

    RET_OK(vm::GCHandle::is_type_pinned(obj->klass));
}

/// @intrinsic: System.Runtime.InteropServices.Marshal::IsPinnable(System.Object)
static RtResultVoid is_pinnable_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtObject* obj = interp::EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeInteropServicesMarshal::is_pinnable(obj));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_runtime_interopservices_marshal[] = {
    {"System.Runtime.InteropServices.Marshal::IsPinnable(System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeInteropServicesMarshal::is_pinnable, is_pinnable_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemRuntimeInteropServicesMarshal::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_runtime_interopservices_marshal,
                                           sizeof(s_intrinsic_entries_system_runtime_interopservices_marshal) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

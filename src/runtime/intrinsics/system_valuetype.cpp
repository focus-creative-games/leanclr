#include "system_valuetype.h"

#include "icalls/system_valuetype.h"
#include "interp/eval_stack_op.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<int32_t> SystemValueType::get_hash_code(vm::RtObject* obj) noexcept
{
    vm::RtArray* uncomputed_fields = nullptr;
    return icalls::SystemValueType::internal_get_hash_code(obj, &uncomputed_fields);
}

/// @intrinsic: System.ValueType::GetHashCode()
static RtResultVoid get_hash_code_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                          const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    vm::RtObject* obj = interp::EvalStackOp::get_param<vm::RtObject*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hash_code, SystemValueType::get_hash_code(obj));
    interp::EvalStackOp::set_return(ret, hash_code);
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_valuetype[] = {
    {"System.ValueType::GetHashCode()", (vm::IntrinsicFunction)&SystemValueType::get_hash_code, get_hash_code_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemValueType::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_valuetype,
                                           sizeof(s_intrinsic_entries_system_valuetype) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

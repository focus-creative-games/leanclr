#include "system_reflection_rtfieldinfo.h"

#include "interp/eval_stack_op.h"
#include "vm/field.h"
#include "vm/reflection.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<vm::RtObject*> SystemReflectionRtFieldInfo::get_value(vm::RtReflectionField* field, vm::RtObject* obj) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field_info, vm::Reflection::get_field_info_from_reflection_object(field));
    return vm::Field::get_value_object(field_info, obj);
}

/// @intrinsic: System.Reflection.RtFieldInfo::GetValue(System.Object)
static RtResultVoid get_value_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto field = interp::EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    auto obj = interp::EvalStackOp::get_param<vm::RtObject*>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, value, SystemReflectionRtFieldInfo::get_value(field, obj));
    interp::EvalStackOp::set_return(ret, value);
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_reflection_rtfieldinfo[] = {
    {"System.Reflection.RtFieldInfo::GetValue(System.Object)", (vm::IntrinsicFunction)&SystemReflectionRtFieldInfo::get_value, get_value_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemReflectionRtFieldInfo::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_reflection_rtfieldinfo,
                                           sizeof(s_intrinsic_entries_system_reflection_rtfieldinfo) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

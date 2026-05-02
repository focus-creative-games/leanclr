#include "system_monocustomattrs.h"
#include "vm/class.h"
#include "vm/customattribute.h"
#include "interp/eval_stack_op.h"

using namespace leanclr::core;
using namespace leanclr::vm;
using namespace leanclr::metadata;
using namespace leanclr::interp;

namespace leanclr
{
namespace icalls
{

// Implementation functions

RtResult<bool> SystemMonoCustomAttrs::is_defined_internal(RtObject* obj, RtReflectionType* attribute_type) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, attr_klass, Class::get_class_from_typesig(attribute_type->type_handle));
    return CustomAttribute::has_attribute(obj, attr_klass);
}

RtResult<RtArray*> SystemMonoCustomAttrs::get_custom_attributes_internal(RtObject* obj, RtReflectionType* attribute_type, bool pseudo_attrs) noexcept
{
    // TODO: Handle pseudo attributes
    (void)pseudo_attrs; // Suppress unused parameter warning
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, attr_klass, Class::get_class_from_typesig(attribute_type->type_handle));
    return CustomAttribute::get_customattributes_on_target_object(obj, attr_klass);
}

RtResult<RtArray*> SystemMonoCustomAttrs::get_custom_attributes_data_internal(RtObject* obj) noexcept
{
    return CustomAttribute::get_customattributes_data_on_target(obj);
}

// Invoker functions

/// @icall: System.MonoCustomAttrs::IsDefinedInternal
static RtResultVoid is_defined_internal_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    RtObject* obj = EvalStackOp::get_param<RtObject*>(params, 0);
    RtReflectionType* attr_type = EvalStackOp::get_param<RtReflectionType*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemMonoCustomAttrs::is_defined_internal(obj, attr_type));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.MonoCustomAttrs::GetCustomAttributesInternal
static RtResultVoid get_custom_attributes_internal_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params,
                                                           RtStackObject* ret) noexcept
{
    RtObject* obj = EvalStackOp::get_param<RtObject*>(params, 0);
    RtReflectionType* attr_type = EvalStackOp::get_param<RtReflectionType*>(params, 1);
    bool pseudo_attrs = EvalStackOp::get_param<bool>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, result, SystemMonoCustomAttrs::get_custom_attributes_internal(obj, attr_type, pseudo_attrs));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.MonoCustomAttrs::GetCustomAttributesDataInternal(System.Reflection.ICustomAttributeProvider)
static RtResultVoid get_custom_attributes_data_internal_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params,
                                                                RtStackObject* ret) noexcept
{
    RtObject* obj = EvalStackOp::get_param<RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, result, SystemMonoCustomAttrs::get_custom_attributes_data_internal(obj));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

// Internal call entries

static InternalCallEntry s_internal_call_entries_system_monocustomattrs[] = {
    {"System.MonoCustomAttrs::IsDefinedInternal", (InternalCallFunction)&SystemMonoCustomAttrs::is_defined_internal, is_defined_internal_invoker},
    {"System.MonoCustomAttrs::GetCustomAttributesInternal", (InternalCallFunction)&SystemMonoCustomAttrs::get_custom_attributes_internal,
     get_custom_attributes_internal_invoker},
    {"System.MonoCustomAttrs::GetCustomAttributesDataInternal(System.Reflection.ICustomAttributeProvider)",
     (InternalCallFunction)&SystemMonoCustomAttrs::get_custom_attributes_data_internal, get_custom_attributes_data_internal_invoker},
};

utils::Span<InternalCallEntry> SystemMonoCustomAttrs::get_internal_call_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_internal_call_entries_system_monocustomattrs) / sizeof(s_internal_call_entries_system_monocustomattrs[0]);
    return utils::Span<InternalCallEntry>(s_internal_call_entries_system_monocustomattrs, entry_count);
}

} // namespace icalls
} // namespace leanclr

#include "intrinsics/system_reflection_customattribute.h"

#include "interp/eval_stack_op.h"
#include "vm/class.h"
#include "vm/customattribute.h"
#include "vm/reflection.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<bool> SystemReflectionCustomAttribute::is_defined(vm::RtReflectionMethod* method, vm::RtReflectionRuntimeType* attribute_type,
                                                           bool inherit) noexcept
{
    (void)inherit;
    if (method == nullptr || attribute_type == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, attribute_type_sig,
                                            vm::Reflection::get_type_sig_from_reflection_type_object(&attribute_type->reflection_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, attr_klass,
                                            vm::Class::get_class_from_typesig(attribute_type_sig));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method_info, vm::Reflection::get_method_info_from_reflection_object(method));
    return vm::CustomAttribute::has_customattribute_on_method(method_info, attr_klass);
}

RtResult<bool> SystemReflectionCustomAttribute::is_defined_on_type(vm::RtReflectionRuntimeType* type, vm::RtReflectionRuntimeType* attribute_type,
                                                                   bool inherit) noexcept
{
    (void)inherit;
    if (type == nullptr || attribute_type == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig,
                                            vm::Reflection::get_type_sig_from_reflection_type_object(&type->reflection_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, attribute_type_sig,
                                            vm::Reflection::get_type_sig_from_reflection_type_object(&attribute_type->reflection_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, attr_klass,
                                            vm::Class::get_class_from_typesig(attribute_type_sig));
    return vm::CustomAttribute::has_customattribute_on_class(klass, attr_klass);
}

RtResult<vm::RtArray*> SystemReflectionCustomAttribute::get_custom_attributes(vm::RtReflectionMethod* method,
                                                                              vm::RtReflectionRuntimeType* attribute_type,
                                                                              bool inherit) noexcept
{
    (void)inherit;
    if (method == nullptr || attribute_type == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, attribute_type_sig,
                                            vm::Reflection::get_type_sig_from_reflection_type_object(&attribute_type->reflection_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, attr_klass,
                                            vm::Class::get_class_from_typesig(attribute_type_sig));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method_info, vm::Reflection::get_method_info_from_reflection_object(method));
    return vm::CustomAttribute::get_customattributes_on_target_token(method_info->parent->image, method_info->token, attr_klass);
}

RtResult<vm::RtArray*> SystemReflectionCustomAttribute::get_custom_attributes_data_internal(vm::RtReflectionMethod* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method_info, vm::Reflection::get_method_info_from_reflection_object(method));
    return vm::CustomAttribute::get_customattributes_data_on_target_token(method_info->parent->image, method_info->token);
}

RtResult<vm::RtArray*> SystemReflectionCustomAttribute::get_custom_attributes_data_internal_on_type(vm::RtReflectionRuntimeType* type) noexcept
{
    if (type == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig,
                                            vm::Reflection::get_type_sig_from_reflection_type_object(&type->reflection_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    return vm::CustomAttribute::get_customattributes_data_on_target_token(klass->image, klass->token);
}

RtResult<vm::RtArray*> SystemReflectionCustomAttribute::get_custom_attributes_data_internal_on_provider(vm::RtObject* provider) noexcept
{
    return vm::CustomAttribute::get_customattributes_data_on_target(provider);
}

/// @intrinsic: System.Reflection.CustomAttribute::IsDefined(System.Reflection.RuntimeMethodInfo,System.RuntimeType,System.Boolean)
static RtResultVoid is_defined_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    auto method = interp::EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    auto attribute_type = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 1);
    bool inherit = interp::EvalStackOp::get_param<bool>(params, 2);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemReflectionCustomAttribute::is_defined(method, attribute_type, inherit));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @intrinsic: System.Reflection.CustomAttribute::IsDefined(System.RuntimeType,System.RuntimeType,System.Boolean)
static RtResultVoid is_defined_on_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    auto type = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto attribute_type = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 1);
    bool inherit = interp::EvalStackOp::get_param<bool>(params, 2);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemReflectionCustomAttribute::is_defined_on_type(type, attribute_type, inherit));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @intrinsic: System.Reflection.CustomAttribute::GetCustomAttributes(System.Reflection.RuntimeMethodInfo,System.RuntimeType,System.Boolean)
static RtResultVoid get_custom_attributes_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto method = interp::EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    auto attribute_type = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 1);
    bool inherit = interp::EvalStackOp::get_param<bool>(params, 2);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result,
                                            SystemReflectionCustomAttribute::get_custom_attributes(method, attribute_type, inherit));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @intrinsic: System.Reflection.RuntimeCustomAttributeData::GetCustomAttributesInternal(System.Reflection.RuntimeMethodInfo)
static RtResultVoid get_custom_attributes_data_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto method = interp::EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result,
                                            SystemReflectionCustomAttribute::get_custom_attributes_data_internal(method));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @intrinsic: System.Reflection.RuntimeCustomAttributeData::GetCustomAttributesInternal(System.RuntimeType)
static RtResultVoid get_custom_attributes_data_internal_on_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto type = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result,
                                            SystemReflectionCustomAttribute::get_custom_attributes_data_internal_on_type(type));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static RtResultVoid get_custom_attributes_data_internal_on_provider_invoker(metadata::RtManagedMethodPointer,
                                                                            const metadata::RtMethodInfo*,
                                                                            const interp::RtStackObject* params,
                                                                            interp::RtStackObject* ret) noexcept
{
    auto provider = interp::EvalStackOp::get_param<vm::RtObject*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result,
                                            SystemReflectionCustomAttribute::get_custom_attributes_data_internal_on_provider(provider));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_reflection_customattribute[] = {
    {"System.Reflection.CustomAttribute::IsDefined(System.Reflection.RuntimeMethodInfo,System.RuntimeType,System.Boolean)",
     (vm::IntrinsicFunction)&SystemReflectionCustomAttribute::is_defined, is_defined_invoker},
    {"System.Reflection.CustomAttribute::IsDefined(System.RuntimeType,System.RuntimeType,System.Boolean)",
     (vm::IntrinsicFunction)&SystemReflectionCustomAttribute::is_defined_on_type, is_defined_on_type_invoker},
    {"System.Reflection.CustomAttribute::GetCustomAttributes(System.Reflection.RuntimeMethodInfo,System.RuntimeType,System.Boolean)",
     (vm::IntrinsicFunction)&SystemReflectionCustomAttribute::get_custom_attributes, get_custom_attributes_invoker},
    {"System.Reflection.RuntimeCustomAttributeData::GetCustomAttributesInternal(System.Reflection.RuntimeMethodInfo)",
     (vm::IntrinsicFunction)&SystemReflectionCustomAttribute::get_custom_attributes_data_internal, get_custom_attributes_data_internal_invoker},
    {"System.Reflection.RuntimeCustomAttributeData::GetCustomAttributesInternal(System.RuntimeType)",
     (vm::IntrinsicFunction)&SystemReflectionCustomAttribute::get_custom_attributes_data_internal_on_type,
     get_custom_attributes_data_internal_on_type_invoker},
    {"System.Reflection.RuntimeCustomAttributeData::GetCustomAttributesInternal(System.Reflection.RuntimeFieldInfo)",
     (vm::IntrinsicFunction)&SystemReflectionCustomAttribute::get_custom_attributes_data_internal_on_provider,
     get_custom_attributes_data_internal_on_provider_invoker},
    {"System.Reflection.RuntimeCustomAttributeData::GetCustomAttributesInternal(System.Reflection.RuntimeConstructorInfo)",
     (vm::IntrinsicFunction)&SystemReflectionCustomAttribute::get_custom_attributes_data_internal_on_provider,
     get_custom_attributes_data_internal_on_provider_invoker},
    {"System.Reflection.RuntimeCustomAttributeData::GetCustomAttributesInternal(System.Reflection.RuntimeEventInfo)",
     (vm::IntrinsicFunction)&SystemReflectionCustomAttribute::get_custom_attributes_data_internal_on_provider,
     get_custom_attributes_data_internal_on_provider_invoker},
    {"System.Reflection.RuntimeCustomAttributeData::GetCustomAttributesInternal(System.Reflection.RuntimePropertyInfo)",
     (vm::IntrinsicFunction)&SystemReflectionCustomAttribute::get_custom_attributes_data_internal_on_provider,
     get_custom_attributes_data_internal_on_provider_invoker},
    {"System.Reflection.RuntimeCustomAttributeData::GetCustomAttributesInternal(System.Reflection.RuntimeModule)",
     (vm::IntrinsicFunction)&SystemReflectionCustomAttribute::get_custom_attributes_data_internal_on_provider,
     get_custom_attributes_data_internal_on_provider_invoker},
    {"System.Reflection.RuntimeCustomAttributeData::GetCustomAttributesInternal(System.Reflection.RuntimeAssembly)",
     (vm::IntrinsicFunction)&SystemReflectionCustomAttribute::get_custom_attributes_data_internal_on_provider,
     get_custom_attributes_data_internal_on_provider_invoker},
    {"System.Reflection.RuntimeCustomAttributeData::GetCustomAttributesInternal(System.Reflection.RuntimeParameterInfo)",
     (vm::IntrinsicFunction)&SystemReflectionCustomAttribute::get_custom_attributes_data_internal_on_provider,
     get_custom_attributes_data_internal_on_provider_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemReflectionCustomAttribute::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_reflection_customattribute,
                                           sizeof(s_intrinsic_entries_system_reflection_customattribute) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

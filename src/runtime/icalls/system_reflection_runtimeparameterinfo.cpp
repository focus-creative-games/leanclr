#include "system_reflection_runtimeparameterinfo.h"

#include "vm/class.h"
#include "vm/method.h"
#include "vm/reflection.h"
#include "vm/rt_array.h"

namespace leanclr
{
namespace icalls
{

RtResult<int32_t> SystemReflectionRuntimeParameterInfo::get_metadata_token(const vm::RtReflectionParameter* param) noexcept
{
    vm::RtReflectionMethod* ref_method = reinterpret_cast<vm::RtReflectionMethod*>(param->member);
    const metadata::RtMethodInfo* method = ref_method->method;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<uint32_t>, token_opt, vm::Method::get_parameter_token(method, param->index));
    uint32_t token = token_opt.has_value() ? token_opt.value() : 0;
    RET_OK(static_cast<int32_t>(token));
}

RtResult<vm::RtArray*> SystemReflectionRuntimeParameterInfo::get_type_modifiers(vm::RtReflectionType* parameter_type, vm::RtObject* member, int32_t index,
                                                                                bool optional) noexcept
{
    (void)parameter_type;

    const metadata::RtMethodInfo* method = nullptr;
    const metadata::RtClass* member_klass = member->klass;
    const metadata::RtClass* reflection_method_class = vm::Class::get_corlib_types().cls_reflection_method;
    const metadata::RtClass* reflection_constructor_class = vm::Class::get_corlib_types().cls_reflection_constructor;

    if (member_klass == reflection_method_class)
    {
        vm::RtReflectionMethod* ref_method = reinterpret_cast<vm::RtReflectionMethod*>(member);
        method = ref_method->method;
    }
    else if (member_klass == reflection_constructor_class)
    {
        vm::RtReflectionMethod* ref_constructor = reinterpret_cast<vm::RtReflectionMethod*>(member);
        method = ref_constructor->method;
    }
    else
    {
        RET_ERR(RtErr::InvalidOperation);
    }

    utils::Vector<metadata::RtClass*> modifiers;
    RET_ERR_ON_FAIL(vm::Method::get_parameter_modifiers(method, index, optional, modifiers));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        vm::RtArray*, modifier_type_arr,
        vm::Array::new_szarray_from_ele_klass(vm::Class::get_corlib_types().cls_systemtype, static_cast<int32_t>(modifiers.size())));

    for (size_t i = 0; i < modifiers.size(); ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, type_obj, vm::Reflection::get_klass_reflection_object(modifiers[i]));
        vm::Array::set_array_data_at(modifier_type_arr, static_cast<int32_t>(i), type_obj);
    }

    RET_OK(modifier_type_arr);
}

/// @icall: System.Reflection.RuntimeParameterInfo::GetMetadataToken()
static RtResultVoid get_metadata_token_invoker_system_reflection_runtimeparameterinfo(metadata::RtManagedMethodPointer methodPtr,
                                                                                      const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                                                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto param = EvalStackOp::get_param<const vm::RtReflectionParameter*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, token, SystemReflectionRuntimeParameterInfo::get_metadata_token(param));
    EvalStackOp::set_return(ret, token);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeParameterInfo::GetTypeModifiers(System.Type,System.Reflection.MemberInfo,System.Int32,System.Boolean)
static RtResultVoid runtimeparameterinfo_get_type_modifiers_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                    const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto parameter_type = EvalStackOp::get_param<vm::RtReflectionType*>(params, 0);
    auto member = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    auto index = EvalStackOp::get_param<int32_t>(params, 2);
    auto optional = EvalStackOp::get_param<bool>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, modifiers,
                                            SystemReflectionRuntimeParameterInfo::get_type_modifiers(parameter_type, member, index, optional));
    EvalStackOp::set_return(ret, modifiers);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemReflectionRuntimeParameterInfo::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Reflection.RuntimeParameterInfo::GetMetadataToken()", (vm::InternalCallFunction)&SystemReflectionRuntimeParameterInfo::get_metadata_token,
         get_metadata_token_invoker_system_reflection_runtimeparameterinfo},
        {"System.Reflection.RuntimeParameterInfo::GetTypeModifiers(System.Type,System.Reflection.MemberInfo,System.Int32,System.Boolean)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeParameterInfo::get_type_modifiers, runtimeparameterinfo_get_type_modifiers_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

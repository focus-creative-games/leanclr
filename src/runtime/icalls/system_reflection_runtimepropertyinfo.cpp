#include "system_reflection_runtimepropertyinfo.h"
#include "icall_base.h"
#include "vm/class.h"
#include "vm/property.h"
#include "vm/reflection.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace icalls
{

// ========== PInfo enum ==========
enum class PInfo : int32_t
{
    Attributes = 0x1,
    GetMethod = 0x2,
    SetMethod = 0x4,
    ReflectedType = 0x8,
    DeclaringType = 0x10,
    Name = 0x20,
};

// ========== Implementation Functions ==========

RtResult<vm::RtReflectionProperty*> SystemReflectionRuntimePropertyInfo::internal_from_handle_type(metadata::RtPropertyInfo* property,
                                                                                                   const metadata::RtTypeSig* type_sig) noexcept
{
    const metadata::RtClass* property_parent = property->parent;
    if (type_sig == nullptr)
    {
        return vm::Reflection::get_property_reflection_object(property, property_parent);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtClass*, cur_klass, vm::Class::get_class_from_typesig(type_sig));
    while (cur_klass != nullptr)
    {
        if (cur_klass == property_parent)
        {
            return vm::Reflection::get_property_reflection_object(property, cur_klass);
        }
        cur_klass = cur_klass->parent;
    }
    RET_OK(nullptr);
}

RtResultVoid SystemReflectionRuntimePropertyInfo::get_property_info(vm::RtReflectionProperty* property, vm::RtMonoPropertyInfo* result_info,
                                                                    int32_t pinfo) noexcept
{
    if (pinfo & static_cast<int32_t>(PInfo::Attributes))
    {
        result_info->attrs = property->property->flags;
    }
    const metadata::RtClass* reflected_klass = property->klass;
    const metadata::RtPropertyInfo* prop = property->property;

    if (pinfo & static_cast<int32_t>(PInfo::GetMethod))
    {
        if (prop->get_method != nullptr)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, get_method,
                                                    vm::Reflection::get_method_reflection_object(prop->get_method, reflected_klass));
            result_info->get_method = get_method;
        }
        else
        {
            result_info->get_method = nullptr;
        }
    }
    if (pinfo & static_cast<int32_t>(PInfo::SetMethod))
    {
        if (prop->set_method != nullptr)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, set_method,
                                                    vm::Reflection::get_method_reflection_object(prop->set_method, reflected_klass));
            result_info->set_method = set_method;
        }
        else
        {
            result_info->set_method = nullptr;
        }
    }
    if (pinfo & static_cast<int32_t>(PInfo::ReflectedType))
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, parent, vm::Reflection::get_klass_reflection_object(reflected_klass));
        result_info->parent = parent;
    }
    if (pinfo & static_cast<int32_t>(PInfo::DeclaringType))
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, declaring_type, vm::Reflection::get_klass_reflection_object(prop->parent));
        result_info->declaring_type = declaring_type;
    }
    if (pinfo & static_cast<int32_t>(PInfo::Name))
    {
        vm::RtString* name = vm::String::create_string_from_utf8cstr(prop->name);
        result_info->name = name;
    }
    RET_VOID_OK();
}

RtResult<vm::RtArray*> SystemReflectionRuntimePropertyInfo::get_type_modifiers(vm::RtReflectionProperty* property, bool optional) noexcept
{
    const metadata::RtPropertyInfo* prop = property->property;
    metadata::RtModuleDef* mod = prop->parent->image;

    utils::Vector<metadata::RtClass*> modifiers;
    RET_ERR_ON_FAIL(vm::Property::get_property_modifiers(prop, optional, modifiers));

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

RtResult<vm::RtObject*> SystemReflectionRuntimePropertyInfo::get_default_value(vm::RtReflectionProperty* property) noexcept
{
    return vm::Property::get_const_object(property->property);
}

RtResult<int32_t> SystemReflectionRuntimePropertyInfo::get_metadata_token(vm::RtReflectionProperty* property) noexcept
{
    RET_OK(static_cast<int32_t>(property->property->token));
}

// ========== Invoker Functions ==========

/// @icall: System.Reflection.RuntimePropertyInfo::internal_from_handle_type(System.IntPtr,System.IntPtr)
static RtResultVoid internal_from_handle_type_invoker_runtimepropertyinfo(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                          const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    metadata::RtPropertyInfo* property = EvalStackOp::get_param<metadata::RtPropertyInfo*>(params, 0);
    const metadata::RtTypeSig* type_sig = EvalStackOp::get_param<const metadata::RtTypeSig*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionProperty*, result,
                                            SystemReflectionRuntimePropertyInfo::internal_from_handle_type(property, type_sig));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall:
/// System.Reflection.RuntimePropertyInfo::System.Reflection.RuntimePropertyInfo::get_property_info
static RtResultVoid get_property_info_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    (void)ret;
    vm::RtReflectionProperty* property = EvalStackOp::get_param<vm::RtReflectionProperty*>(params, 0);
    vm::RtMonoPropertyInfo* result_info_ptr = EvalStackOp::get_param<vm::RtMonoPropertyInfo*>(params, 1);
    int32_t pinfo = EvalStackOp::get_param<int32_t>(params, 2);
    return SystemReflectionRuntimePropertyInfo::get_property_info(property, result_info_ptr, pinfo);
}

/// @icall: System.Reflection.RuntimePropertyInfo::GetTypeModifiers(System.Reflection.RuntimePropertyInfo,System.Boolean)
static RtResultVoid runtimepropertyinfo_get_type_modifiers_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionProperty* property = EvalStackOp::get_param<vm::RtReflectionProperty*>(params, 0);
    bool optional = EvalStackOp::get_param<bool>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result, SystemReflectionRuntimePropertyInfo::get_type_modifiers(property, optional));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimePropertyInfo::get_default_value(System.Reflection.RuntimePropertyInfo)
static RtResultVoid get_default_value_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionProperty* property = EvalStackOp::get_param<vm::RtReflectionProperty*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemReflectionRuntimePropertyInfo::get_default_value(property));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimePropertyInfo::get_metadata_token(System.Reflection.RuntimePropertyInfo)
static RtResultVoid get_metadata_token_invoker_system_reflection_runtimepropertyinfo(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionProperty* property = EvalStackOp::get_param<vm::RtReflectionProperty*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemReflectionRuntimePropertyInfo::get_metadata_token(property));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

// ========== Registration ==========

static vm::InternalCallEntry s_internal_call_entries_system_reflection_runtimepropertyinfo[] = {
    {"System.Reflection.RuntimePropertyInfo::internal_from_handle_type(System.IntPtr,System.IntPtr)",
     (vm::InternalCallFunction)&SystemReflectionRuntimePropertyInfo::internal_from_handle_type, internal_from_handle_type_invoker_runtimepropertyinfo},
    {"System.Reflection.RuntimePropertyInfo::get_property_info", (vm::InternalCallFunction)&SystemReflectionRuntimePropertyInfo::get_property_info,
     get_property_info_invoker},
    {"System.Reflection.RuntimePropertyInfo::GetTypeModifiers(System.Reflection.RuntimePropertyInfo,System.Boolean)",
     (vm::InternalCallFunction)&SystemReflectionRuntimePropertyInfo::get_type_modifiers, runtimepropertyinfo_get_type_modifiers_invoker},
    {"System.Reflection.RuntimePropertyInfo::get_default_value(System.Reflection.RuntimePropertyInfo)",
     (vm::InternalCallFunction)&SystemReflectionRuntimePropertyInfo::get_default_value, get_default_value_invoker},
    {"System.Reflection.RuntimePropertyInfo::get_metadata_token(System.Reflection.RuntimePropertyInfo)",
     (vm::InternalCallFunction)&SystemReflectionRuntimePropertyInfo::get_metadata_token, get_metadata_token_invoker_system_reflection_runtimepropertyinfo},
};

utils::Span<vm::InternalCallEntry> SystemReflectionRuntimePropertyInfo::get_internal_call_entries() noexcept
{
    constexpr size_t entry_count =
        sizeof(s_internal_call_entries_system_reflection_runtimepropertyinfo) / sizeof(s_internal_call_entries_system_reflection_runtimepropertyinfo[0]);
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_reflection_runtimepropertyinfo, entry_count);
}

} // namespace icalls
} // namespace leanclr

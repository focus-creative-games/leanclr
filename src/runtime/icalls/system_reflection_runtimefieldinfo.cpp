#include "system_reflection_runtimefieldinfo.h"
#include "icall_base.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/reflection.h"
#include "vm/rt_array.h"

namespace leanclr
{
namespace icalls
{

// ========== Implementation Functions ==========

RtResult<uint32_t> SystemReflectionRuntimeFieldInfo::get_metadata_token(vm::RtReflectionField* field) noexcept
{
    if (field == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    const metadata::RtFieldInfo* field_info = field->field;
    RET_OK(field_info->token);
}

RtResult<int32_t> SystemReflectionRuntimeFieldInfo::get_field_offset(vm::RtReflectionField* field) noexcept
{
    if (field == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    const metadata::RtFieldInfo* field_info = field->field;
    RET_OK(static_cast<int32_t>(vm::Field::get_field_offset_excludes_object_header_for_all_type(field_info)));
}

RtResult<vm::RtObject*> SystemReflectionRuntimeFieldInfo::get_raw_const_value(vm::RtReflectionField* field) noexcept
{
    if (field == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    const metadata::RtFieldInfo* field_info = field->field;
    return vm::Field::get_field_const_object(field_info);
}

RtResult<vm::RtObject*> SystemReflectionRuntimeFieldInfo::get_value_internal(vm::RtReflectionField* field, vm::RtObject* obj) noexcept
{
    const metadata::RtFieldInfo* field_info = field->field;
    return vm::Field::get_value_object(field_info, obj);
}

RtResult<vm::RtObject*> SystemReflectionRuntimeFieldInfo::unsafe_get_value(vm::RtReflectionField* field, vm::RtObject* obj) noexcept
{
    const metadata::RtFieldInfo* field_info = field->field;
    return vm::Field::get_value_object(field_info, obj);
}

RtResultVoid SystemReflectionRuntimeFieldInfo::set_value_internal(vm::RtReflectionField* field, vm::RtObject* obj, vm::RtObject* value) noexcept
{
    const metadata::RtFieldInfo* field_info = field->field;
    return vm::Field::set_value_object(field_info, obj, value);
}

RtResult<vm::RtReflectionType*> SystemReflectionRuntimeFieldInfo::get_parent_type(vm::RtReflectionField* field, bool declaring) noexcept
{
    const metadata::RtClass* parent = declaring ? field->field->parent : field->klass;
    return vm::Reflection::get_klass_reflection_object(parent);
}

RtResult<vm::RtReflectionType*> SystemReflectionRuntimeFieldInfo::resolve_type(vm::RtReflectionField* field) noexcept
{
    const metadata::RtFieldInfo* field_info = field->field;
    const metadata::RtTypeSig* type_sig = field_info->type_sig;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    return vm::Reflection::get_klass_reflection_object(klass);
}

RtResult<vm::RtArray*> SystemReflectionRuntimeFieldInfo::get_type_modifiers(vm::RtReflectionField* field, bool optional) noexcept
{
    const metadata::RtFieldInfo* field_info = field->field;

    utils::Vector<metadata::RtClass*> modifiers;
    RET_ERR_ON_FAIL(vm::Field::get_field_modifiers(field_info, optional, modifiers));

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

// ========== Invoker Functions ==========

/// @icall: System.Reflection.RuntimeFieldInfo::get_metadata_token
static RtResultVoid get_metadata_token_invoker_system_reflection_runtimefieldinfo(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionField* field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, result, SystemReflectionRuntimeFieldInfo::get_metadata_token(field));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeFieldInfo::GetFieldOffset
static RtResultVoid get_field_offset_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionField* field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemReflectionRuntimeFieldInfo::get_field_offset(field));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeFieldInfo::GetRawConstantValue
static RtResultVoid get_raw_const_value_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionField* field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemReflectionRuntimeFieldInfo::get_raw_const_value(field));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeFieldInfo::GetValueInternal
static RtResultVoid get_value_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionField* field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemReflectionRuntimeFieldInfo::get_value_internal(field, obj));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeFieldInfo::UnsafeGetValue
static RtResultVoid unsafe_get_value_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionField* field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemReflectionRuntimeFieldInfo::unsafe_get_value(field, obj));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeFieldInfo::SetValueInternal(System.Reflection.FieldInfo,System.Object,System.Object)
static RtResultVoid set_value_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    (void)ret;
    vm::RtReflectionField* field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    vm::RtObject* value = EvalStackOp::get_param<vm::RtObject*>(params, 2);
    return SystemReflectionRuntimeFieldInfo::set_value_internal(field, obj, value);
}

/// @icall: System.Reflection.RuntimeFieldInfo::GetParentType
static RtResultVoid get_parent_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                            interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionField* field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    bool declaring = EvalStackOp::get_param<bool>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, result, SystemReflectionRuntimeFieldInfo::get_parent_type(field, declaring));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeFieldInfo::ResolveType
static RtResultVoid resolve_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                         interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionField* field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, result, SystemReflectionRuntimeFieldInfo::resolve_type(field));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeFieldInfo::GetTypeModifiers(System.Boolean)
static RtResultVoid runtimefieldinfo_get_type_modifiers_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionField* field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    bool optional = EvalStackOp::get_param<bool>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result, SystemReflectionRuntimeFieldInfo::get_type_modifiers(field, optional));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

// ========== Registration ==========

static vm::InternalCallEntry s_internal_call_entries_system_reflection_runtimefieldinfo[] = {
    {"System.Reflection.RuntimeFieldInfo::get_metadata_token", (vm::InternalCallFunction)&SystemReflectionRuntimeFieldInfo::get_metadata_token,
     get_metadata_token_invoker_system_reflection_runtimefieldinfo},
    {"System.Reflection.RuntimeFieldInfo::GetFieldOffset", (vm::InternalCallFunction)&SystemReflectionRuntimeFieldInfo::get_field_offset,
     get_field_offset_invoker},
    {"System.Reflection.RuntimeFieldInfo::GetRawConstantValue", (vm::InternalCallFunction)&SystemReflectionRuntimeFieldInfo::get_raw_const_value,
     get_raw_const_value_invoker},
    {"System.Reflection.RuntimeFieldInfo::GetValueInternal", (vm::InternalCallFunction)&SystemReflectionRuntimeFieldInfo::get_value_internal,
     get_value_internal_invoker},
    {"System.Reflection.RuntimeFieldInfo::UnsafeGetValue", (vm::InternalCallFunction)&SystemReflectionRuntimeFieldInfo::unsafe_get_value,
     unsafe_get_value_invoker},
    {"System.Reflection.RuntimeFieldInfo::SetValueInternal(System.Reflection.FieldInfo,System.Object,System.Object)",
     (vm::InternalCallFunction)&SystemReflectionRuntimeFieldInfo::set_value_internal, set_value_internal_invoker},
    {"System.Reflection.RuntimeFieldInfo::GetParentType", (vm::InternalCallFunction)&SystemReflectionRuntimeFieldInfo::get_parent_type,
     get_parent_type_invoker},
    {"System.Reflection.RuntimeFieldInfo::ResolveType", (vm::InternalCallFunction)&SystemReflectionRuntimeFieldInfo::resolve_type, resolve_type_invoker},
    {"System.Reflection.RuntimeFieldInfo::GetTypeModifiers(System.Boolean)", (vm::InternalCallFunction)&SystemReflectionRuntimeFieldInfo::get_type_modifiers,
     runtimefieldinfo_get_type_modifiers_invoker},
};

utils::Span<vm::InternalCallEntry> SystemReflectionRuntimeFieldInfo::get_internal_call_entries() noexcept
{
    constexpr size_t entry_count =
        sizeof(s_internal_call_entries_system_reflection_runtimefieldinfo) / sizeof(s_internal_call_entries_system_reflection_runtimefieldinfo[0]);
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_reflection_runtimefieldinfo, entry_count);
}

} // namespace icalls
} // namespace leanclr

#include "system_runtimefieldhandle.h"

#include <cstring>

#include "system_reflection_runtimefieldinfo.h"
#include "system_typedreference.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/object.h"
#include "vm/type.h"

namespace leanclr
{
namespace icalls
{

RtResult<vm::RtObject*> SystemRuntimeFieldHandle::get_value_direct(vm::RtReflectionField* field, vm::RtReflectionRuntimeType* field_type, vm::RtTypedReference* typed_ref,
                                                                  vm::RtReflectionRuntimeType* context_type) noexcept
{
    assert(field != nullptr);
    assert(typed_ref != nullptr);
    assert(context_type != nullptr);

    const metadata::RtFieldInfo* field_info = field->field;
    const metadata::RtClass* parent_klass = field_info->parent;
    //if (!vm::Class::is_value_type(parent_klass))
    //{
    //    RET_ERR(RtErr::NotSupported);
    //}
    // may be used in unsafe context which type is not the same as the field's parent type, so we don't check it
    // if (parent_klass != typed_ref->klass)
    // {
    //     RET_ERR(RtErr::Argument);
    // }
    if (vm::Class::is_reference_type(parent_klass))
    {
        return vm::Field::get_value_object(field_info, *(vm::RtObject**)typed_ref->value);
    }
    return vm::Field::get_value_direct(field_info, const_cast<void*>(typed_ref->value));
}

RtResultVoid SystemRuntimeFieldHandle::set_value_direct(vm::RtReflectionField* field, vm::RtReflectionRuntimeType* field_type, vm::RtTypedReference* typed_ref,
                                                        vm::RtObject* value, vm::RtReflectionRuntimeType* context_type) noexcept
{
    assert(field != nullptr);
    assert(typed_ref != nullptr);
    assert(context_type != nullptr);

    const metadata::RtFieldInfo* field_info = field->field;
    const metadata::RtClass* parent_klass = field_info->parent;
    //if (!vm::Class::is_value_type(parent_klass))
    //{
    //    RET_ERR(RtErr::NotSupported);
    //}
    // may be used in unsafe context which type is not the same as the field's parent type, so we don't check it
    // if (parent_klass != typed_ref->klass)
    // {
    //     RET_ERR(RtErr::Argument);
    // }
    if (vm::Class::is_reference_type(parent_klass))
    {
        return vm::Field::set_value_object(field_info, *(vm::RtObject**)typed_ref->value, value);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_reference_type, vm::Type::is_reference_type(field_info->type_sig));
    void* ptr_field_value = is_reference_type ? (void*)&value : (void*)(value ? value + 1 : nullptr);
    return vm::Field::set_value_direct(field_info, const_cast<void*>(typed_ref->value), ptr_field_value);
}

RtResultVoid SystemRuntimeFieldHandle::set_value_internal(vm::RtReflectionField* field, vm::RtObject* obj, vm::RtObject* value) noexcept
{
    return SystemReflectionRuntimeFieldInfo::set_value_internal(field, obj, value);
}

/// @icall: System.RuntimeFieldHandle::GetValueDirect(System.Reflection.RuntimeFieldInfo,System.RuntimeType,System.Void*,System.RuntimeType)
static RtResultVoid get_value_direct_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    auto field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    auto field_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 1);
    auto typed_ref = EvalStackOp::get_param<vm::RtTypedReference*>(params, 2);
    auto context_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result,
                                            SystemRuntimeFieldHandle::get_value_direct(field, field_type, typed_ref, context_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeFieldHandle::SetValueDirect(System.Reflection.RuntimeFieldInfo,System.RuntimeType,System.Void*,System.Object,System.RuntimeType)
static RtResultVoid set_value_direct_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    auto field_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 1);
    auto typed_ref = EvalStackOp::get_param<vm::RtTypedReference*>(params, 2);
    auto value = EvalStackOp::get_param<vm::RtObject*>(params, 3);
    auto context_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 4);
    return SystemRuntimeFieldHandle::set_value_direct(field, field_type, typed_ref, value, context_type);
}

/// @icall: System.RuntimeFieldHandle::SetValueInternal(System.Reflection.FieldInfo,System.Object,System.Object)
static RtResultVoid set_value_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    auto value = EvalStackOp::get_param<vm::RtObject*>(params, 2);
    return SystemRuntimeFieldHandle::set_value_internal(field, obj, value);
}

static vm::InternalCallEntry s_internal_call_entries_system_runtimefieldhandle[] = {
    {"System.RuntimeFieldHandle::GetValueDirect(System.Reflection.RuntimeFieldInfo,System.RuntimeType,System.Void*,System.RuntimeType)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_value_direct, get_value_direct_invoker},
    {"System.RuntimeFieldHandle::SetValueDirect(System.Reflection.RuntimeFieldInfo,System.RuntimeType,System.Void*,System.Object,System.RuntimeType)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::set_value_direct, set_value_direct_invoker},
    {"System.RuntimeFieldHandle::SetValueInternal(System.Reflection.FieldInfo,System.Object,System.Object)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::set_value_internal, set_value_internal_invoker},
};

utils::Span<vm::InternalCallEntry> SystemRuntimeFieldHandle::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_runtimefieldhandle,
                                              sizeof(s_internal_call_entries_system_runtimefieldhandle) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

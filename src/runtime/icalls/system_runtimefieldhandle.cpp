#include "system_runtimefieldhandle.h"

#include "system_reflection_runtimefieldinfo.h"
#include "system_typedreference.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/reflection.h"
#include "vm/type.h"

namespace leanclr
{
namespace icalls
{
namespace
{
static RtResult<const metadata::RtFieldInfo*> get_runtime_field_handle_internal_param(const interp::RtStackObject* params,
                                                                                     size_t index) noexcept
{
    uintptr_t raw_value = EvalStackOp::get_param<uintptr_t>(params, index);
    return vm::Reflection::get_field_info_from_handle_arg(reinterpret_cast<const void*>(raw_value));
}
} // namespace

RtResult<vm::RtObject*> SystemRuntimeFieldHandle::get_value_direct(vm::RtReflectionField* field, vm::RtReflectionRuntimeType* field_type, vm::RtTypedReference* typed_ref,
                                                                  vm::RtReflectionRuntimeType* context_type) noexcept
{
    assert(field != nullptr);
    assert(typed_ref != nullptr);
    assert(context_type != nullptr);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field_info, vm::Reflection::get_field_info_from_reflection_object(field));
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

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field_info, vm::Reflection::get_field_info_from_reflection_object(field));
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

RtResult<int32_t> SystemRuntimeFieldHandle::get_token(const metadata::RtFieldInfo* field) noexcept
{
    if (field == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(static_cast<int32_t>(field->token));
}

RtResult<uint32_t> SystemRuntimeFieldHandle::get_attributes(const metadata::RtFieldInfo* field) noexcept
{
    if (field == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(field->flags);
}

RtResult<const void*> SystemRuntimeFieldHandle::get_approx_declaring_method_table(const metadata::RtFieldInfo* field) noexcept
{
    if (field == nullptr || field->parent == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const void*, method_table,
                                            vm::Reflection::get_net10_method_table(vm::Class::get_by_val_type_sig(field->parent)));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, declaring_type,
                                            vm::Reflection::get_klass_reflection_object(field->parent));
    (void)declaring_type;
    RET_OK(method_table);
}

RtResult<const metadata::RtFieldInfo*> SystemRuntimeFieldHandle::get_static_field_for_generic_type(const metadata::RtFieldInfo* field,
                                                                                                  const void* method_table) noexcept
{
    (void)method_table;
    if (field == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(field);
}

RtResult<bool> SystemRuntimeFieldHandle::acquires_context_from_this(const metadata::RtFieldInfo* field) noexcept
{
    if (field == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(false);
}

RtResult<const char*> SystemRuntimeFieldHandle::get_utf8_name(const metadata::RtFieldInfo* field) noexcept
{
    if (field == nullptr || field->name == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(field->name);
}

RtResult<bool> SystemRuntimeFieldHandle::is_fast_path_supported(vm::RtReflectionField* field) noexcept
{
    if (field == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(false);
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

/// @icall: System.RuntimeFieldHandle::GetToken(System.IntPtr)
static RtResultVoid get_token_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    auto field_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, vm::Reflection::get_field_info_from_handle_arg(field_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, token, SystemRuntimeFieldHandle::get_token(field));
    EvalStackOp::set_return(ret, token);
    RET_VOID_OK();
}

/// @icall: System.RuntimeFieldHandle::GetAttributes(System.RuntimeFieldHandleInternal)
static RtResultVoid get_attributes_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, get_runtime_field_handle_internal_param(params, 0));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, attributes, SystemRuntimeFieldHandle::get_attributes(field));
    EvalStackOp::set_return(ret, attributes);
    RET_VOID_OK();
}

/// @icall: System.RuntimeFieldHandle::GetApproxDeclaringMethodTable(System.RuntimeFieldHandleInternal)
static RtResultVoid get_approx_declaring_method_table_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, get_runtime_field_handle_internal_param(params, 0));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const void*, method_table,
                                            SystemRuntimeFieldHandle::get_approx_declaring_method_table(field));
    EvalStackOp::set_return(ret, method_table);
    RET_VOID_OK();
}

/// @icall: System.RuntimeFieldHandle::GetStaticFieldForGenericType(System.RuntimeFieldHandleInternal,System.Runtime.CompilerServices.MethodTable*)
static RtResultVoid get_static_field_for_generic_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, get_runtime_field_handle_internal_param(params, 0));
    auto method_table = EvalStackOp::get_param<const void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, generic_field,
                                            SystemRuntimeFieldHandle::get_static_field_for_generic_type(field, method_table));
    EvalStackOp::set_return(ret, generic_field);
    RET_VOID_OK();
}

/// @icall: System.RuntimeFieldHandle::AcquiresContextFromThis(System.RuntimeFieldHandleInternal)
static RtResultVoid acquires_context_from_this_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, get_runtime_field_handle_internal_param(params, 0));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeFieldHandle::acquires_context_from_this(field));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeFieldHandle::GetUtf8NameInternal(System.RuntimeFieldHandleInternal)
static RtResultVoid get_utf8_name_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, get_runtime_field_handle_internal_param(params, 0));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, name, SystemRuntimeFieldHandle::get_utf8_name(field));
    EvalStackOp::set_return(ret, name);
    RET_VOID_OK();
}

/// @icall: System.RuntimeFieldHandle::IsFastPathSupported(System.Reflection.RtFieldInfo)
static RtResultVoid is_fast_path_supported_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeFieldHandle::is_fast_path_supported(field));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_runtimefieldhandle[] = {
    {"System.RuntimeFieldHandle::GetValueDirect(System.Reflection.RuntimeFieldInfo,System.RuntimeType,System.Void*,System.RuntimeType)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_value_direct, get_value_direct_invoker},
    {"System.RuntimeFieldHandle::SetValueDirect(System.Reflection.RuntimeFieldInfo,System.RuntimeType,System.Void*,System.Object,System.RuntimeType)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::set_value_direct, set_value_direct_invoker},
    {"System.RuntimeFieldHandle::SetValueInternal(System.Reflection.FieldInfo,System.Object,System.Object)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::set_value_internal, set_value_internal_invoker},
    {"System.RuntimeFieldHandle::GetToken(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_token, get_token_invoker},
    {"System.RuntimeFieldHandle::GetAttributes(System.RuntimeFieldHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_attributes, get_attributes_invoker},
    {"System.RuntimeFieldHandle::GetAttributes", (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_attributes, get_attributes_invoker},
    {"System.RuntimeFieldHandle::GetApproxDeclaringMethodTable(System.RuntimeFieldHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_approx_declaring_method_table, get_approx_declaring_method_table_invoker},
    {"System.RuntimeFieldHandle::GetApproxDeclaringMethodTable",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_approx_declaring_method_table, get_approx_declaring_method_table_invoker},
    {"System.RuntimeFieldHandle::GetStaticFieldForGenericType(System.RuntimeFieldHandleInternal,System.Runtime.CompilerServices.MethodTable*)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_static_field_for_generic_type, get_static_field_for_generic_type_invoker},
    {"System.RuntimeFieldHandle::GetStaticFieldForGenericType",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_static_field_for_generic_type, get_static_field_for_generic_type_invoker},
    {"System.RuntimeFieldHandle::AcquiresContextFromThis(System.RuntimeFieldHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::acquires_context_from_this, acquires_context_from_this_invoker},
    {"System.RuntimeFieldHandle::AcquiresContextFromThis",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::acquires_context_from_this, acquires_context_from_this_invoker},
    {"System.RuntimeFieldHandle::GetUtf8NameInternal(System.RuntimeFieldHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_utf8_name, get_utf8_name_invoker},
    {"System.RuntimeFieldHandle::GetUtf8NameInternal", (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_utf8_name,
     get_utf8_name_invoker},
};

static vm::InternalCallEntry s_net10_internal_call_entries_system_runtimefieldhandle[] = {
    {"System.RuntimeFieldHandle::GetValueDirect(System.Reflection.RuntimeFieldInfo,System.RuntimeType,System.Void*,System.RuntimeType)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_value_direct, get_value_direct_invoker},
    {"System.RuntimeFieldHandle::SetValueDirect(System.Reflection.RuntimeFieldInfo,System.RuntimeType,System.Void*,System.Object,System.RuntimeType)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::set_value_direct, set_value_direct_invoker},
    {"System.RuntimeFieldHandle::GetToken(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_token, get_token_invoker},
    {"System.RuntimeFieldHandle::GetToken", (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_token, get_token_invoker},
    {"System.RuntimeFieldHandle::GetAttributes(System.RuntimeFieldHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_attributes, get_attributes_invoker},
    {"System.RuntimeFieldHandle::GetAttributes", (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_attributes, get_attributes_invoker},
    {"System.RuntimeFieldHandle::GetApproxDeclaringMethodTable(System.RuntimeFieldHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_approx_declaring_method_table, get_approx_declaring_method_table_invoker},
    {"System.RuntimeFieldHandle::GetApproxDeclaringMethodTable",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_approx_declaring_method_table, get_approx_declaring_method_table_invoker},
    {"System.RuntimeFieldHandle::GetStaticFieldForGenericType(System.RuntimeFieldHandleInternal,System.Runtime.CompilerServices.MethodTable*)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_static_field_for_generic_type, get_static_field_for_generic_type_invoker},
    {"System.RuntimeFieldHandle::AcquiresContextFromThis(System.RuntimeFieldHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::acquires_context_from_this, acquires_context_from_this_invoker},
    {"System.RuntimeFieldHandle::AcquiresContextFromThis",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::acquires_context_from_this, acquires_context_from_this_invoker},
    {"System.RuntimeFieldHandle::GetUtf8NameInternal(System.RuntimeFieldHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_utf8_name, get_utf8_name_invoker},
    {"System.RuntimeFieldHandle::GetUtf8NameInternal", (vm::InternalCallFunction)&SystemRuntimeFieldHandle::get_utf8_name,
     get_utf8_name_invoker},
    {"System.RuntimeFieldHandle::IsFastPathSupported(System.Reflection.RtFieldInfo)",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::is_fast_path_supported, is_fast_path_supported_invoker},
    {"System.RuntimeFieldHandle::IsFastPathSupported",
     (vm::InternalCallFunction)&SystemRuntimeFieldHandle::is_fast_path_supported, is_fast_path_supported_invoker},
};

utils::Span<vm::InternalCallEntry> SystemRuntimeFieldHandle::get_net10_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_net10_internal_call_entries_system_runtimefieldhandle,
                                              sizeof(s_net10_internal_call_entries_system_runtimefieldhandle) / sizeof(vm::InternalCallEntry));
}

utils::Span<vm::InternalCallEntry> SystemRuntimeFieldHandle::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_runtimefieldhandle,
                                              sizeof(s_internal_call_entries_system_runtimefieldhandle) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

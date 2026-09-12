#include "system_runtimemethodhandle.h"

#include "metadata/metadata_cache.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/reflection.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"
#include "vm/shim.h"

namespace leanclr
{
namespace icalls
{
namespace
{
RtResult<const metadata::RtMethodInfo*> get_method_from_handle_arg(const void* method_arg) noexcept
{
    return vm::Reflection::get_method_info_from_handle_arg(method_arg);
}
} // namespace

RtResult<intptr_t> SystemRuntimeMethodHandle::get_function_pointer(intptr_t method) noexcept
{
    if (method == 0)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method_info,
                                            vm::Reflection::get_method_info_from_handle_arg(reinterpret_cast<const void*>(method)));
    RET_OK(reinterpret_cast<intptr_t>(method_info->method_ptr));
}

RtResult<int32_t> SystemRuntimeMethodHandle::get_attributes(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(static_cast<int32_t>(method->flags));
}

RtResult<int32_t> SystemRuntimeMethodHandle::get_impl_attributes(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(static_cast<int32_t>(method->iflags));
}

RtResult<const void*> SystemRuntimeMethodHandle::get_method_table(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const void*, method_table,
                                            vm::Reflection::get_net10_method_table(vm::Class::get_by_val_type_sig(method->parent)));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, declaring_type,
                                            vm::Reflection::get_klass_reflection_object(method->parent));
    (void)declaring_type;
    RET_OK(method_table);
}

RtResult<int32_t> SystemRuntimeMethodHandle::get_slot(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(static_cast<int32_t>(method->slot));
}

RtResult<int32_t> SystemRuntimeMethodHandle::get_method_def(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(static_cast<int32_t>(method->token));
}

RtResult<vm::RtString*> SystemRuntimeMethodHandle::get_name(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr || method->name == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(vm::String::create_string_from_utf8cstr(method->name));
}

RtResult<const char*> SystemRuntimeMethodHandle::get_utf8_name(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(method->name);
}

RtResult<bool> SystemRuntimeMethodHandle::has_method_instantiation(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(vm::Method::get_generic_param_count(method) != 0);
}

RtResult<bool> SystemRuntimeMethodHandle::is_generic_method_definition(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(method->generic_container != nullptr &&
           (method->generic_method == nullptr || method->generic_method->generic_context.method_inst == nullptr));
}

RtResult<int32_t> SystemRuntimeMethodHandle::get_generic_parameter_count(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(static_cast<int32_t>(vm::Method::get_generic_param_count(method)));
}

RtResult<vm::RtArray*> SystemRuntimeMethodHandle::get_method_instantiation(const metadata::RtMethodInfo* method, bool runtime_array) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    const auto& corlib_types = vm::Class::get_corlib_types();
    metadata::RtClass* element_klass = runtime_array ? corlib_types.cls_runtimetype : corlib_types.cls_systemtype;

    if (method->generic_method != nullptr && method->generic_method->generic_context.method_inst != nullptr)
    {
        const metadata::RtGenericInst* inst = method->generic_method->generic_context.method_inst;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
            vm::RtArray*, result,
            LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(element_klass, inst->generic_arg_count,
                                                        "SystemRuntimeMethodHandle::get_method_instantiation"));
        for (uint8_t i = 0; i < inst->generic_arg_count; ++i)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, type_obj,
                                                    vm::Reflection::get_type_reflection_object(inst->generic_args[i]));
            vm::Array::set_array_data_at<vm::RtReflectionType*>(result, i, type_obj);
        }
        RET_OK(result);
    }

    if (method->generic_container != nullptr)
    {
        const metadata::RtGenericContainer* generic_container = method->generic_container;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
            vm::RtArray*, result,
            LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(element_klass, generic_container->generic_param_count,
                                                        "SystemRuntimeMethodHandle::get_method_instantiation"));
        for (uint8_t i = 0; i < generic_container->generic_param_count; ++i)
        {
            const metadata::RtGenericParam* generic_param = &generic_container->generic_params[i];
            metadata::RtTypeSig generic_param_type_sig =
                metadata::RtTypeSig::new_byval_with_data(metadata::RtElementType::MVar, generic_param);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, pooled,
                                                    metadata::MetadataCache::get_pooled_typesig(generic_param_type_sig));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, type_obj,
                                                    vm::Reflection::get_type_reflection_object(pooled));
            vm::Array::set_array_data_at<vm::RtReflectionType*>(result, i, type_obj);
        }
        RET_OK(result);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        vm::RtArray*, result,
        LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS_INTERNAL(element_klass, "SystemRuntimeMethodHandle::get_method_instantiation"));
    RET_OK(result);
}

RtResult<bool> SystemRuntimeMethodHandle::is_typical_method_definition(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(method->generic_method == nullptr);
}

RtResult<const metadata::RtMethodInfo*> SystemRuntimeMethodHandle::get_stub_if_needed(
    const metadata::RtMethodInfo* method, const vm::RtReflectionRuntimeType* declaring_type) noexcept
{
    if (method == nullptr || declaring_type == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(method);
}

RtResult<const metadata::RtMethodInfo*> SystemRuntimeMethodHandle::get_method_from_canonical(
    const metadata::RtMethodInfo* method, const vm::RtReflectionRuntimeType* declaring_type) noexcept
{
    return get_stub_if_needed(method, declaring_type);
}

RtResult<bool> SystemRuntimeMethodHandle::is_dynamic_method(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(false);
}

RtResult<bool> SystemRuntimeMethodHandle::is_constructor(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(vm::Method::is_ctor_or_cctor(method));
}

RtResult<vm::RtObject*> SystemRuntimeMethodHandle::get_resolver(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(nullptr);
}

RtResult<vm::RtObject*> SystemRuntimeMethodHandle::get_loader_allocator(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(nullptr);
}

static RtResult<vm::RtReflectionMethodBody*> get_method_body(const void* method_arg) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    return vm::Method::create_reflection_method_body(method);
}

/// @icall: System.RuntimeMethodHandle::GetFunctionPointer(System.IntPtr)
static RtResultVoid get_function_pointer_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    intptr_t method = EvalStackOp::get_param<intptr_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, result, SystemRuntimeMethodHandle::get_function_pointer(method));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetAttributes
static RtResultVoid get_attributes_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, attributes, SystemRuntimeMethodHandle::get_attributes(method));
    EvalStackOp::set_return(ret, attributes);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetImplAttributes
static RtResultVoid get_impl_attributes_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, impl_attributes, SystemRuntimeMethodHandle::get_impl_attributes(method));
    EvalStackOp::set_return(ret, impl_attributes);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetMethodTable
static RtResultVoid get_method_table_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const void*, method_table, SystemRuntimeMethodHandle::get_method_table(method));
    EvalStackOp::set_return(ret, method_table);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetSlot
static RtResultVoid get_slot_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, slot, SystemRuntimeMethodHandle::get_slot(method));
    EvalStackOp::set_return(ret, slot);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetMethodDef
static RtResultVoid get_method_def_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, token, SystemRuntimeMethodHandle::get_method_def(method));
    EvalStackOp::set_return(ret, token);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetName(System.RuntimeMethodHandleInternal)
static RtResultVoid get_name_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, name, SystemRuntimeMethodHandle::get_name(method));
    EvalStackOp::set_return(ret, name);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetUtf8NameInternal
static RtResultVoid get_utf8_name_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, name, SystemRuntimeMethodHandle::get_utf8_name(method));
    EvalStackOp::set_return(ret, name);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::HasMethodInstantiation
static RtResultVoid has_method_instantiation_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                     interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeMethodHandle::has_method_instantiation(method));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::IsGenericMethodDefinition
static RtResultVoid is_generic_method_definition_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeMethodHandle::is_generic_method_definition(method));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetGenericParameterCount
static RtResultVoid get_generic_parameter_count_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemRuntimeMethodHandle::get_generic_parameter_count(method));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::IsTypicalMethodDefinition
static RtResultVoid is_typical_method_definition_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeMethodHandle::is_typical_method_definition(method));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetStubIfNeededInternal
static RtResultVoid get_stub_if_needed_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    auto declaring_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, stub,
                                            SystemRuntimeMethodHandle::get_stub_if_needed(method, declaring_type));
    EvalStackOp::set_return(ret, stub);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetMethodFromCanonical
static RtResultVoid get_method_from_canonical_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                      const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    auto declaring_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, result,
                                            SystemRuntimeMethodHandle::get_method_from_canonical(method, declaring_type));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::IsDynamicMethod
static RtResultVoid is_dynamic_method_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeMethodHandle::is_dynamic_method(method));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::IsConstructor
static RtResultVoid is_constructor_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeMethodHandle::is_constructor(method));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetResolver
static RtResultVoid get_resolver_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                         interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, resolver, SystemRuntimeMethodHandle::get_resolver(method));
    EvalStackOp::set_return(ret, resolver);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetLoaderAllocatorInternal
static RtResultVoid get_loader_allocator_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, get_method_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, loader_allocator, SystemRuntimeMethodHandle::get_loader_allocator(method));
    EvalStackOp::set_return(ret, loader_allocator);
    RET_VOID_OK();
}

/// @icall: System.RuntimeMethodHandle::GetMethodBody(System.IRuntimeMethodInfo,System.RuntimeType)
static RtResultVoid get_method_body_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                            interp::RtStackObject* ret) noexcept
{
    auto method_arg = EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethodBody*, body, get_method_body(method_arg));
    EvalStackOp::set_return(ret, body);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_runtimemethodhandle[] = {
    {"System.RuntimeMethodHandle::GetFunctionPointer(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_function_pointer,
     get_function_pointer_invoker},
    {"System.RuntimeMethodHandle::GetAttributes", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_attributes, get_attributes_invoker},
    {"System.RuntimeMethodHandle::GetAttributes(System.RuntimeMethodHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_attributes, get_attributes_invoker},
    {"System.RuntimeMethodHandle::GetImplAttributes", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_impl_attributes,
     get_impl_attributes_invoker},
    {"System.RuntimeMethodHandle::GetImplAttributes(System.IRuntimeMethodInfo)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_impl_attributes, get_impl_attributes_invoker},
    {"System.RuntimeMethodHandle::GetMethodTable", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_method_table, get_method_table_invoker},
    {"System.RuntimeMethodHandle::GetMethodTable(System.RuntimeMethodHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_method_table, get_method_table_invoker},
    {"System.RuntimeMethodHandle::GetSlot", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_slot, get_slot_invoker},
    {"System.RuntimeMethodHandle::GetSlot(System.RuntimeMethodHandleInternal)", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_slot,
     get_slot_invoker},
    {"System.RuntimeMethodHandle::GetMethodDef", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_method_def, get_method_def_invoker},
    {"System.RuntimeMethodHandle::GetMethodDef(System.RuntimeMethodHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_method_def, get_method_def_invoker},
    {"System.RuntimeMethodHandle::GetName(System.RuntimeMethodHandleInternal)", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_name,
     get_name_invoker},
    {"System.RuntimeMethodHandle::GetUtf8NameInternal", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_utf8_name, get_utf8_name_invoker},
    {"System.RuntimeMethodHandle::GetUtf8NameInternal(System.RuntimeMethodHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_utf8_name, get_utf8_name_invoker},
    {"System.RuntimeMethodHandle::HasMethodInstantiation", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::has_method_instantiation,
     has_method_instantiation_invoker},
    {"System.RuntimeMethodHandle::HasMethodInstantiation(System.RuntimeMethodHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::has_method_instantiation, has_method_instantiation_invoker},
    {"System.RuntimeMethodHandle::IsGenericMethodDefinition", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::is_generic_method_definition,
     is_generic_method_definition_invoker},
    {"System.RuntimeMethodHandle::IsGenericMethodDefinition(System.RuntimeMethodHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::is_generic_method_definition, is_generic_method_definition_invoker},
    {"System.RuntimeMethodHandle::GetGenericParameterCount", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_generic_parameter_count,
     get_generic_parameter_count_invoker},
    {"System.RuntimeMethodHandle::GetGenericParameterCount(System.RuntimeMethodHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_generic_parameter_count, get_generic_parameter_count_invoker},
    {"System.RuntimeMethodHandle::IsTypicalMethodDefinition", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::is_typical_method_definition,
     is_typical_method_definition_invoker},
    {"System.RuntimeMethodHandle::IsTypicalMethodDefinition(System.IRuntimeMethodInfo)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::is_typical_method_definition, is_typical_method_definition_invoker},
    {"System.RuntimeMethodHandle::GetStubIfNeededInternal", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_stub_if_needed,
     get_stub_if_needed_invoker},
    {"System.RuntimeMethodHandle::GetStubIfNeededInternal(System.RuntimeMethodHandleInternal,System.RuntimeType)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_stub_if_needed, get_stub_if_needed_invoker},
    {"System.RuntimeMethodHandle::GetMethodFromCanonical", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_method_from_canonical,
     get_method_from_canonical_invoker},
    {"System.RuntimeMethodHandle::GetMethodFromCanonical(System.RuntimeMethodHandleInternal,System.RuntimeType)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_method_from_canonical, get_method_from_canonical_invoker},
    {"System.RuntimeMethodHandle::IsDynamicMethod", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::is_dynamic_method, is_dynamic_method_invoker},
    {"System.RuntimeMethodHandle::IsDynamicMethod(System.RuntimeMethodHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::is_dynamic_method, is_dynamic_method_invoker},
    {"System.RuntimeMethodHandle::IsConstructor", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::is_constructor, is_constructor_invoker},
    {"System.RuntimeMethodHandle::IsConstructor(System.RuntimeMethodHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::is_constructor, is_constructor_invoker},
    {"System.RuntimeMethodHandle::GetResolver", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_resolver, get_resolver_invoker},
    {"System.RuntimeMethodHandle::GetResolver(System.RuntimeMethodHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_resolver, get_resolver_invoker},
    {"System.RuntimeMethodHandle::GetLoaderAllocatorInternal", (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_loader_allocator,
     get_loader_allocator_invoker},
    {"System.RuntimeMethodHandle::GetLoaderAllocatorInternal(System.RuntimeMethodHandleInternal)",
     (vm::InternalCallFunction)&SystemRuntimeMethodHandle::get_loader_allocator, get_loader_allocator_invoker},
    {"System.RuntimeMethodHandle::GetMethodBody(System.IRuntimeMethodInfo,System.RuntimeType)", nullptr, get_method_body_invoker},
    {"System.RuntimeMethodHandle::GetMethodBody", nullptr, get_method_body_invoker},
};

utils::Span<vm::InternalCallEntry> SystemRuntimeMethodHandle::get_net10_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_runtimemethodhandle,
                                              sizeof(s_internal_call_entries_system_runtimemethodhandle) / sizeof(vm::InternalCallEntry));
}

utils::Span<vm::InternalCallEntry> SystemRuntimeMethodHandle::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_runtimemethodhandle,
                                              sizeof(s_internal_call_entries_system_runtimemethodhandle) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

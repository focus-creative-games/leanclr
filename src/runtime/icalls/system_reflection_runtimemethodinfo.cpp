#include "system_reflection_runtimemethodinfo.h"

#include "icall_base.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/reflection.h"
#include "vm/rt_array.h"
#include "vm/generic_method.h"
#include "vm/rt_string.h"
#include "metadata/metadata_cache.h"
#include "metadata/module_def.h"

namespace leanclr
{
namespace icalls
{

RtResult<vm::RtReflectionMethodBody*> SystemReflectionRuntimeMethodInfo::get_method_body_internal(const metadata::RtMethodInfo* method) noexcept
{
    return vm::Method::create_reflection_method_body(method);
}

RtResult<vm::RtReflectionMethod*> SystemReflectionRuntimeMethodInfo::get_method_from_handle_internal_type_native(const metadata::RtMethodInfo* method,
                                                                                                                 const metadata::RtTypeSig* type_sig,
                                                                                                                 bool check_same_generic_base) noexcept
{
    const metadata::RtClass* klass = nullptr;
    const metadata::RtMethodInfo* target_method = method;

    if (type_sig != nullptr)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, resolved, vm::Class::get_class_from_typesig(type_sig));
        klass = resolved;
        if (check_same_generic_base)
        {
            const metadata::RtClass* method_klass = method->parent;
            if (vm::Class::get_generic_base_klass_or_self(klass) != vm::Class::get_generic_base_klass_or_self(method_klass))
            {
                RET_OK(nullptr);
            }

            RET_ERR_ON_FAIL(vm::Class::initialize_methods(const_cast<metadata::RtClass*>(klass)));
            if (method_klass != klass)
            {
                size_t index = vm::Method::get_method_index_in_class(method);
                target_method = klass->methods[index];
                if (target_method == nullptr)
                {
                    RET_OK(nullptr);
                }
            }
        }
    }
    else
    {
        klass = method->parent;
    }

    return vm::Reflection::get_method_reflection_object(target_method, klass);
}

RtResult<vm::RtString*> SystemReflectionRuntimeMethodInfo::get_name(vm::RtReflectionMethod* method) noexcept
{
    const metadata::RtMethodInfo* m = method->method;
    vm::RtString* name = vm::String::create_string_from_utf8cstr(m->name);
    RET_OK(name);
}

static RtResult<std::pair<const metadata::RtMethodInfo*, const metadata::RtClass*>> get_base_method_impl(const metadata::RtMethodInfo* method,
                                                                                                         bool definition) noexcept
{
    const metadata::RtClass* klass = method->parent;
    if (!vm::Method::is_virtual(method) || vm::Method::is_new_slot(method) || vm::Class::is_interface(klass))
    {
        RET_OK(std::make_pair(method, klass));
    }

    RET_ERR_ON_FAIL(vm::Class::initialize_vtables(const_cast<metadata::RtClass*>(klass)));
    uint16_t slot = method->slot;
    const metadata::RtMethodInfo* base_method = method;
    const metadata::RtClass* cur_klass = klass;

    if (definition)
    {
        while (cur_klass->parent)
        {
            const metadata::RtClass* next_klass = cur_klass->parent;
            RET_ERR_ON_FAIL(vm::Class::initialize_vtables(const_cast<metadata::RtClass*>(next_klass)));
            if (next_klass->vtable_count <= slot)
            {
                break;
            }
            cur_klass = next_klass;
        }

        const metadata::RtMethodInfo** methods = cur_klass->methods;
        for (uint32_t i = 0; i < cur_klass->method_count; ++i)
        {
            const metadata::RtMethodInfo* m = methods[i];
            if (m->slot == slot && vm::Method::is_new_slot(m))
            {
                RET_OK(std::make_pair(m, cur_klass));
            }
        }

        RET_OK(std::make_pair(base_method, cur_klass));
    }

    while (cur_klass->parent)
    {
        cur_klass = cur_klass->parent;
        RET_ERR_ON_FAIL(vm::Class::initialize_vtables(const_cast<metadata::RtClass*>(cur_klass)));
        if (cur_klass->vtable_count <= slot)
        {
            break;
        }
        base_method = cur_klass->vtable[slot].method_impl;
        if (base_method->parent == cur_klass)
        {
            break;
        }
    }

    RET_OK(std::make_pair(base_method, cur_klass));
}

RtResult<vm::RtReflectionMethod*> SystemReflectionRuntimeMethodInfo::get_base_method(vm::RtReflectionMethod* method, bool definition) noexcept
{
    const metadata::RtMethodInfo* m = method->method;
    std::pair<const metadata::RtMethodInfo*, const metadata::RtClass*> pair_result;
    UNWRAP_OR_RET_ERR_ON_FAIL(pair_result, get_base_method_impl(m, definition));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, ref_method,
                                            vm::Reflection::get_method_reflection_object(pair_result.first, pair_result.second));
    RET_OK(ref_method);
}

RtResult<int32_t> SystemReflectionRuntimeMethodInfo::get_metadata_token(vm::RtReflectionMethod* method) noexcept
{
    RET_OK(static_cast<int32_t>(method->method->token));
}

RtResult<bool> SystemReflectionRuntimeMethodInfo::get_is_generic_method_definition(vm::RtReflectionMethod* method) noexcept
{
    const metadata::RtMethodInfo* m = method->method;
    const metadata::RtClass* klass = m->parent;
    RET_OK(vm::Class::is_generic(klass) || m->generic_container != nullptr);
}

RtResult<vm::RtArray*> SystemReflectionRuntimeMethodInfo::get_generic_arguments(vm::RtReflectionMethod* method) noexcept
{
    const metadata::RtMethodInfo* m = method->method;
    const auto& corlib = vm::Class::get_corlib_types();
    const metadata::RtClass* elem_klass = corlib.cls_systemtype;

    if (m->generic_container != nullptr)
    {
        const metadata::RtGenericContainer* gc = m->generic_container;
        uint8_t count = gc->generic_param_count;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, vm::Array::new_szarray_from_ele_klass(elem_klass, count));
        for (uint8_t i = 0; i < count; ++i)
        {
            const metadata::RtGenericParam& gp = gc->generic_params[i];
            metadata::RtTypeSig var_ts = metadata::RtTypeSig::new_byval_with_data(metadata::RtElementType::MVar, &gp);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, pooled, metadata::MetadataCache::get_pooled_typesig(var_ts));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, type_obj, vm::Reflection::get_type_reflection_object(pooled));
            vm::Array::set_array_data_at<vm::RtReflectionType*>(arr, i, type_obj);
        }
        RET_OK(arr);
    }

    if (m->generic_method != nullptr && m->generic_method->generic_context.method_inst != nullptr)
    {
        const metadata::RtGenericInst* inst = m->generic_method->generic_context.method_inst;
        uint8_t count = inst->generic_arg_count;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, vm::Array::new_szarray_from_ele_klass(elem_klass, count));
        for (uint8_t i = 0; i < count; ++i)
        {
            const metadata::RtTypeSig* arg = inst->generic_args[i];
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, type_obj, vm::Reflection::get_type_reflection_object(arg));
            vm::Array::set_array_data_at<vm::RtReflectionType*>(arr, i, type_obj);
        }
        RET_OK(arr);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, empty, vm::Array::new_empty_szarray_by_ele_klass(elem_klass));
    RET_OK(empty);
}

RtResult<vm::RtReflectionMethod*> SystemReflectionRuntimeMethodInfo::get_generic_method_definition_impl(vm::RtReflectionMethod* method) noexcept
{
    const metadata::RtMethodInfo* m = method->method;
    const metadata::RtClass* klass = m->parent;
    if (m->generic_container != nullptr || klass->generic_container != nullptr)
    {
        RET_OK(method);
    }

    if (m->generic_method == nullptr)
    {
        RET_OK(static_cast<vm::RtReflectionMethod*>(nullptr));
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, base_method,
                                            vm::Method::get_method_by_method_def_gid(m->generic_method->base_method_gid));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, ref_method,
                                            vm::Reflection::get_method_reflection_object(base_method, base_method->parent));
    RET_OK(ref_method);
}

RtResult<vm::RtReflectionMethod*> SystemReflectionRuntimeMethodInfo::make_generic_method_impl(vm::RtReflectionMethod* method,
                                                                                              vm::RtArray* generic_args) noexcept
{
    const metadata::RtMethodInfo* m = method->method;
    if (m->generic_container == nullptr)
    {
        RET_ERR(RtErr::Argument);
    }

    uint8_t generic_param_count = m->generic_container->generic_param_count;
    if (generic_param_count != vm::Array::get_array_length(generic_args))
    {
        RET_ERR(RtErr::Argument);
    }

    const metadata::RtGenericInst* class_inst = nullptr;
    const metadata::RtMethodInfo* base_method = m;
    if (m->generic_method != nullptr)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, base,
                                                vm::Method::get_method_by_method_def_gid(m->generic_method->base_method_gid));
        base_method = base;
        class_inst = m->generic_method->generic_context.class_inst;
    }

    const metadata::RtTypeSig** arg_list = (const metadata::RtTypeSig**)alloca(sizeof(metadata::RtTypeSig*) * generic_param_count);
    for (uint8_t i = 0; i < generic_param_count; ++i)
    {
        auto arg_obj = vm::Array::get_array_data_at<vm::RtReflectionType*>(generic_args, i);
        arg_list[i] = arg_obj->type_handle;
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtGenericInst*, method_inst,
                                            metadata::MetadataCache::get_pooled_generic_inst(arg_list, generic_param_count));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, inflated_method,
                                            vm::GenericMethod::get_method(base_method, class_inst, method_inst));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, ref_method,
                                            vm::Reflection::get_method_reflection_object(inflated_method, inflated_method->parent));
    RET_OK(ref_method);
}

RtResult<bool> SystemReflectionRuntimeMethodInfo::get_is_generic_method(vm::RtReflectionMethod* method) noexcept
{
    const metadata::RtMethodInfo* m = method->method;
    if (m->generic_container != nullptr)
    {
        RET_OK(true);
    }

    const metadata::RtGenericMethod* gm = m->generic_method;
    bool is_generic = gm != nullptr && gm->generic_context.method_inst != nullptr;
    RET_OK(is_generic);
}

RtResult<vm::RtObject*> SystemReflectionRuntimeMethodInfo::internal_invoke(vm::RtReflectionMethod* ref_method, vm::RtObject* obj, vm::RtArray* parameters,
                                                                           vm::RtObject** exc) noexcept
{
    const metadata::RtMethodInfo* method = ref_method->method;
    return vm::Reflection::invoke_method(method, obj, parameters, exc);
}

RtResultVoid SystemReflectionRuntimeMethodInfo::get_pinvoke(vm::RtReflectionMethod* method, int32_t* flags, vm::RtString** entry_name,
                                                            vm::RtString** dll_name) noexcept
{
    const metadata::RtMethodInfo* m = method->method;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<metadata::RowImplMap>, pinvoke_info, vm::Method::get_imp_map_info(m));

    if (pinvoke_info.has_value())
    {
        metadata::RtModuleDef* mod = m->parent->image;
        *flags = static_cast<int32_t>(pinvoke_info->mapping_flags);

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, import_name, mod->get_string(pinvoke_info->import_name));
        *entry_name = vm::String::create_string_from_utf8cstr(import_name);

        auto module_ref = mod->get_cli_image().read_module_ref(pinvoke_info->import_scope);
        if (!module_ref.has_value())
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, dll_name_utf8, mod->get_string(module_ref->name));
        *dll_name = vm::String::create_string_from_utf8cstr(dll_name_utf8);
    }
    else
    {
        *flags = 0;
        *entry_name = nullptr;
        *dll_name = nullptr;
    }

    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeMethodInfo::GetMethodBodyInternal(System.IntPtr)
static RtResultVoid get_method_body_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto method_info = EvalStackOp::get_param<const metadata::RtMethodInfo*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethodBody*, body, SystemReflectionRuntimeMethodInfo::get_method_body_internal(method_info));
    EvalStackOp::set_return(ret, body);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeMethodInfo::GetMethodFromHandleInternalType_native
static RtResultVoid get_method_from_handle_internal_type_native_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto method_handle = EvalStackOp::get_param<const metadata::RtMethodInfo*>(params, 0);
    auto type_handle = EvalStackOp::get_param<const metadata::RtTypeSig*>(params, 1);
    bool check_generic_base = EvalStackOp::get_param<bool>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        vm::RtReflectionMethod*, result,
        SystemReflectionRuntimeMethodInfo::get_method_from_handle_internal_type_native(method_handle, type_handle, check_generic_base));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeMethodInfo::get_name
static RtResultVoid runtimemethodinfo_get_name_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto ref_method = EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, name, SystemReflectionRuntimeMethodInfo::get_name(ref_method));
    EvalStackOp::set_return(ret, name);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeMethodInfo::get_base_method(System.Reflection.RuntimeMethodInfo,System.Boolean)
static RtResultVoid get_base_method_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto ref_method = EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    bool definition = EvalStackOp::get_param<bool>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, base_method, SystemReflectionRuntimeMethodInfo::get_base_method(ref_method, definition));
    EvalStackOp::set_return(ret, base_method);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeMethodInfo::get_metadata_token(System.Reflection.RuntimeMethodInfo)
static RtResultVoid get_metadata_token_invoker_system_reflection_runtimemethodinfo(metadata::RtManagedMethodPointer methodPtr,
                                                                                   const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                                                                   interp::RtStackObject* ret) noexcept
{
    auto ref_method = EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, token, SystemReflectionRuntimeMethodInfo::get_metadata_token(ref_method));
    EvalStackOp::set_return(ret, token);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeMethodInfo::get_IsGenericMethodDefinition
static RtResultVoid get_is_generic_method_definition_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto ref_method = EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_generic_def, SystemReflectionRuntimeMethodInfo::get_is_generic_method_definition(ref_method));
    EvalStackOp::set_return(ret, static_cast<int32_t>(is_generic_def));
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeMethodInfo::GetGenericArguments
static RtResultVoid get_generic_arguments_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto ref_method = EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, args, SystemReflectionRuntimeMethodInfo::get_generic_arguments(ref_method));
    EvalStackOp::set_return(ret, args);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeMethodInfo::GetGenericMethodDefinition_impl()
static RtResultVoid get_generic_method_definition_impl_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto ref_method = EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, def_method,
                                            SystemReflectionRuntimeMethodInfo::get_generic_method_definition_impl(ref_method));
    EvalStackOp::set_return(ret, def_method);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeMethodInfo::MakeGenericMethod_impl
static RtResultVoid make_generic_method_impl_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto ref_method = EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    auto generic_args = EvalStackOp::get_param<vm::RtArray*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, generic_method,
                                            SystemReflectionRuntimeMethodInfo::make_generic_method_impl(ref_method, generic_args));
    EvalStackOp::set_return(ret, generic_method);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeMethodInfo::get_IsGenericMethod
static RtResultVoid get_is_generic_method_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto ref_method = EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_generic, SystemReflectionRuntimeMethodInfo::get_is_generic_method(ref_method));
    EvalStackOp::set_return(ret, static_cast<int32_t>(is_generic));
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeMethodInfo::InternalInvoke
static RtResultVoid internal_invoke_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto ref_method = EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    auto parameters = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    auto exc_ptr = EvalStackOp::get_param<vm::RtObject**>(params, 3);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemReflectionRuntimeMethodInfo::internal_invoke(ref_method, obj, parameters, exc_ptr));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeMethodInfo::GetPInvoke(System.Reflection.PInvokeAttributes&,System.String&,System.String&)
static RtResultVoid get_pinvoke_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    auto ref_method = EvalStackOp::get_param<vm::RtReflectionMethod*>(params, 0);
    auto flags_ptr = EvalStackOp::get_param<int32_t*>(params, 1);
    auto entry_name_ptr = EvalStackOp::get_param<vm::RtString**>(params, 2);
    auto dll_name_ptr = EvalStackOp::get_param<vm::RtString**>(params, 3);
    RET_ERR_ON_FAIL(SystemReflectionRuntimeMethodInfo::get_pinvoke(ref_method, flags_ptr, entry_name_ptr, dll_name_ptr));
    RET_VOID_OK();
}

// Internal call registry
static vm::InternalCallEntry s_internal_call_entries_system_reflection_runtimemethodinfo[] = {
    {"System.Reflection.RuntimeMethodInfo::GetMethodBodyInternal(System.IntPtr)",
     (vm::InternalCallFunction)&SystemReflectionRuntimeMethodInfo::get_method_body_internal, get_method_body_internal_invoker},
    {"System.Reflection.RuntimeMethodInfo::GetMethodFromHandleInternalType_native",
     (vm::InternalCallFunction)&SystemReflectionRuntimeMethodInfo::get_method_from_handle_internal_type_native,
     get_method_from_handle_internal_type_native_invoker},
    {"System.Reflection.RuntimeMethodInfo::get_name", (vm::InternalCallFunction)&SystemReflectionRuntimeMethodInfo::get_name,
     runtimemethodinfo_get_name_invoker},
    {"System.Reflection.RuntimeMethodInfo::get_base_method(System.Reflection.RuntimeMethodInfo,System.Boolean)",
     (vm::InternalCallFunction)&SystemReflectionRuntimeMethodInfo::get_base_method, get_base_method_invoker},
    {"System.Reflection.RuntimeMethodInfo::get_metadata_token(System.Reflection.RuntimeMethodInfo)",
     (vm::InternalCallFunction)&SystemReflectionRuntimeMethodInfo::get_metadata_token, get_metadata_token_invoker_system_reflection_runtimemethodinfo},
    {"System.Reflection.RuntimeMethodInfo::get_IsGenericMethodDefinition",
     (vm::InternalCallFunction)&SystemReflectionRuntimeMethodInfo::get_is_generic_method_definition, get_is_generic_method_definition_invoker},
    {"System.Reflection.RuntimeMethodInfo::GetGenericArguments", (vm::InternalCallFunction)&SystemReflectionRuntimeMethodInfo::get_generic_arguments,
     get_generic_arguments_invoker},
    {"System.Reflection.RuntimeMethodInfo::GetGenericMethodDefinition_impl()",
     (vm::InternalCallFunction)&SystemReflectionRuntimeMethodInfo::get_generic_method_definition_impl, get_generic_method_definition_impl_invoker},
    {"System.Reflection.RuntimeMethodInfo::MakeGenericMethod_impl", (vm::InternalCallFunction)&SystemReflectionRuntimeMethodInfo::make_generic_method_impl,
     make_generic_method_impl_invoker},
    {"System.Reflection.RuntimeMethodInfo::get_IsGenericMethod", (vm::InternalCallFunction)&SystemReflectionRuntimeMethodInfo::get_is_generic_method,
     get_is_generic_method_invoker},
    {"System.Reflection.RuntimeMethodInfo::InternalInvoke", (vm::InternalCallFunction)&SystemReflectionRuntimeMethodInfo::internal_invoke,
     internal_invoke_invoker},
    {"System.Reflection.RuntimeMethodInfo::GetPInvoke(System.Reflection.PInvokeAttributes&,System.String&,System.String&)",
     (vm::InternalCallFunction)&SystemReflectionRuntimeMethodInfo::get_pinvoke, get_pinvoke_invoker},
};

utils::Span<vm::InternalCallEntry> SystemReflectionRuntimeMethodInfo::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_reflection_runtimemethodinfo,
                                              sizeof(s_internal_call_entries_system_reflection_runtimemethodinfo) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

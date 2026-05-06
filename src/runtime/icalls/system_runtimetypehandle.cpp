#include "system_runtimetypehandle.h"

#include "icall_base.h"
#include "vm/class.h"
#include "vm/object.h"
#include "vm/reflection.h"
#include "vm/rt_string.h"
#include "vm/type.h"
#include "vm/assembly.h"
#include "utils/string_builder.h"
#include "utils/string_util.h"
#include "interp/interp_defs.h"
#include "interp/machine_state.h"
#include "metadata/module_def.h"

namespace leanclr
{
namespace icalls
{

// Implementation functions

RtResult<uint32_t> SystemRuntimeTypeHandle::get_attributes(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    RET_OK(klass->flags);
}

RtResult<int32_t> SystemRuntimeTypeHandle::get_metadata_token(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    RET_OK(static_cast<int32_t>(klass->token));
}

RtResult<metadata::RtElementType> SystemRuntimeTypeHandle::get_cor_element_type(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    auto ret = type_sig->is_by_ref() ? metadata::RtElementType::ByRef : type_sig->ele_type;
    RET_OK(ret);
}

RtResult<bool> SystemRuntimeTypeHandle::has_instantiation(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    RET_OK(vm::Class::is_generic(klass) || vm::Class::is_generic_inst(klass));
}

RtResult<bool> SystemRuntimeTypeHandle::is_com_object(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    RET_OK(false);
}

RtResult<bool> SystemRuntimeTypeHandle::has_references(metadata::RtClass* klass) noexcept
{
    RET_OK(vm::Class::get_has_references(klass));
}

RtResult<int32_t> SystemRuntimeTypeHandle::get_array_rank(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    switch (type_sig->ele_type)
    {
    case metadata::RtElementType::Array:
        RET_OK(static_cast<int32_t>(type_sig->data.array_type->rank));
    case metadata::RtElementType::SZArray:
        RET_OK(1);
    default:
        RET_OK(0);
    }
}

RtResult<vm::RtReflectionRuntimeType*> SystemRuntimeTypeHandle::get_element_type(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;

    vm::RtReflectionType* ref_type = nullptr;
    if (type_sig->is_by_ref())
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, temp_ref_type, vm::Reflection::get_type_reflection_object(klass->by_val));
        ref_type = temp_ref_type;
    }
    else
    {
        auto ele_type = type_sig->ele_type;
        switch (ele_type)
        {
        case metadata::RtElementType::Array:
        case metadata::RtElementType::SZArray:
        case metadata::RtElementType::Ptr:
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, temp_ref_type,
                                                    vm::Reflection::get_type_reflection_object(type_sig->data.element_type));
            ref_type = temp_ref_type;
            break;
        default:
            ref_type = nullptr;
            break;
        }
    }

    RET_OK(reinterpret_cast<vm::RtReflectionRuntimeType*>(ref_type));
}

RtResult<bool> SystemRuntimeTypeHandle::is_generic_variable(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    auto ele_type = type_sig->ele_type;
    RET_OK(!type_sig->is_by_ref() && (ele_type == metadata::RtElementType::Var || ele_type == metadata::RtElementType::MVar));
}

RtResult<vm::RtReflectionRuntimeType*> SystemRuntimeTypeHandle::get_base_type(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    auto base_klass = klass->parent;
    if (base_klass == nullptr)
    {
        RET_OK(nullptr);
    }
    else
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, base_ref_type, vm::Reflection::get_klass_reflection_object(base_klass));
        RET_OK(reinterpret_cast<vm::RtReflectionRuntimeType*>(base_ref_type));
    }
}

RtResult<bool> SystemRuntimeTypeHandle::is_generic_type_definition(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    RET_OK(vm::Class::is_generic(klass));
}

RtResult<const metadata::RtGenericParam*> SystemRuntimeTypeHandle::get_generic_parameter_info(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    switch (type_sig->ele_type)
    {
    case metadata::RtElementType::Var:
    case metadata::RtElementType::MVar:
        RET_OK(type_sig->data.generic_param);
    default:
        RET_OK(nullptr);
    }
}

RtResult<bool> SystemRuntimeTypeHandle::is_subclass_of(const metadata::RtTypeSig* child_type, const metadata::RtTypeSig* parent_type) noexcept
{
    if (parent_type->is_by_ref())
    {
        RET_OK(false);
    }
    if (child_type->is_by_ref())
    {
        RET_OK(parent_type->ele_type == metadata::RtElementType::Object);
    }
    if (child_type == parent_type)
    {
        RET_OK(true);
    }

    auto child_ele_type = child_type->ele_type;
    if (child_ele_type == metadata::RtElementType::Var || child_ele_type == metadata::RtElementType::MVar)
    {
        // TODO: handle generic parameter constraints
        RETURN_NOT_IMPLEMENTED_ERROR();
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, child_class, vm::Class::get_class_from_typesig(child_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, parent_class, vm::Class::get_class_from_typesig(parent_type));
    RET_ERR_ON_FAIL(vm::Class::initialize_super_types(child_class));
    RET_OK(vm::Class::is_subclass_of_initialized(child_class, parent_class, true));
}

RtResult<bool> SystemRuntimeTypeHandle::is_by_ref_like(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    RET_OK(vm::Class::is_by_ref_like(klass));
}

RtResult<bool> SystemRuntimeTypeHandle::type_is_assignable_from(vm::RtReflectionRuntimeType* to_type, vm::RtReflectionRuntimeType* from_type) noexcept
{
    auto to_type_sig = to_type->reflection_type.type_handle;
    auto from_type_sig = from_type->reflection_type.type_handle;

    if (to_type_sig->is_by_ref() != from_type_sig->is_by_ref())
    {
        RET_OK(false);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, to_klass, vm::Class::get_class_from_typesig(to_type_sig));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, from_klass, vm::Class::get_class_from_typesig(from_type_sig));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(from_klass));
    RET_OK(vm::Class::is_assignable_from(from_klass, to_klass));
}

RtResult<vm::RtReflectionAssembly*> SystemRuntimeTypeHandle::get_assembly(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    auto mod = klass->image;
    return vm::Reflection::get_assembly_reflection_object(mod->get_assembly());
}

RtResult<bool> SystemRuntimeTypeHandle::is_instance_of_type(const vm::RtReflectionRuntimeType* runtime_type, vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        RET_OK(false);
    }

    auto type_sig = runtime_type->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    RET_OK(vm::Object::is_inst(obj, klass) != nullptr);
}

RtResult<vm::RtReflectionRuntimeType*> SystemRuntimeTypeHandle::get_generic_type_definition_impl(vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    if (type_sig->is_by_ref())
    {
        RET_OK(nullptr);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));

    if (vm::Class::is_generic(klass))
    {
        RET_OK(runtime_type);
    }

    if (vm::Class::is_generic_inst(klass))
    {
        auto generic_definition_klass = vm::Class::get_generic_base_klass_of_generic_class(klass);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, generic_def_ref_type,
                                                vm::Reflection::get_klass_reflection_object(generic_definition_klass));
        RET_OK(reinterpret_cast<vm::RtReflectionRuntimeType*>(generic_def_ref_type));
    }

    RET_OK(nullptr);
}

RtResult<vm::RtReflectionModule*> SystemRuntimeTypeHandle::get_module(const vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    auto type_sig = runtime_type->reflection_type.type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    auto module = klass->image;
    return vm::Reflection::get_module_reflection_object(module);
}

RtResult<vm::RtReflectionType*> SystemRuntimeTypeHandle::internal_from_name(vm::RtString* name, int32_t* stack_crawl_mark, vm::RtReflectionAssembly* assembly,
                                                                            bool throw_on_error, bool ignore_case, bool reflection_only) noexcept
{
    utils::StringBuilder name_buf;
    utils::StringUtil::utf16_to_utf8(vm::String::get_chars_ptr(name), static_cast<size_t>(vm::String::get_length(name)), name_buf);
    metadata::RtModuleDef* default_mod = nullptr;
    if (assembly)
    {
        default_mod = assembly->assembly->mod;
    }
    else
    {
        interp::InterpFrame* execuing_frame = interp::MachineState::get_global_machine_state().get_executing_frame_stack();
        if (execuing_frame != nullptr)
        {
            default_mod = execuing_frame->method->parent->image;
        }
        else
        {
            default_mod = vm::Assembly::get_corlib()->mod;
        }
    }
    auto ret_typesig = vm::Type::parse_assembly_qualified_type(default_mod, name_buf.as_cstr(), name_buf.length(), ignore_case);
    if (ret_typesig.is_err())
    {
        if (throw_on_error)
        {
            RET_ERR(RtErr::TypeLoad);
        }
        else
        {
            RET_OK(nullptr);
        }
    }

    return vm::Reflection::get_type_reflection_object(ret_typesig.unwrap());
}

// Invoker functions

/// @icall: System.RuntimeTypeHandle::GetAttributes
static RtResultVoid get_attributes_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, attributes, SystemRuntimeTypeHandle::get_attributes(runtime_type));
    EvalStackOp::set_return(ret, attributes);
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::GetMetadataToken(System.RuntimeType)
static RtResultVoid get_metadata_token_invoker_system_runtimetypehandle(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, token, SystemRuntimeTypeHandle::get_metadata_token(runtime_type));
    EvalStackOp::set_return(ret, token);
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::GetCorElementType
static RtResultVoid get_cor_element_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtElementType, element_type, SystemRuntimeTypeHandle::get_cor_element_type(runtime_type));
    EvalStackOp::set_return(ret, static_cast<int32_t>(element_type));
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::HasInstantiation
static RtResultVoid has_instantiation_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeTypeHandle::has_instantiation(runtime_type));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::IsComObject(System.RuntimeType)
static RtResultVoid is_com_object_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeTypeHandle::is_com_object(runtime_type));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::HasReferences
static RtResultVoid has_references_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto ref_type = EvalStackOp::get_param<const vm::RtReflectionType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(ref_type->type_handle));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeTypeHandle::has_references(klass));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::GetArrayRank(System.RuntimeType)
static RtResultVoid get_array_rank_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, rank, SystemRuntimeTypeHandle::get_array_rank(runtime_type));
    EvalStackOp::set_return(ret, rank);
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::GetElementType
static RtResultVoid get_element_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionRuntimeType*, element_type, SystemRuntimeTypeHandle::get_element_type(runtime_type));
    EvalStackOp::set_return(ret, element_type);
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::IsGenericVariable
static RtResultVoid is_generic_variable_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeTypeHandle::is_generic_variable(runtime_type));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::GetBaseType
static RtResultVoid get_base_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionRuntimeType*, base_type, SystemRuntimeTypeHandle::get_base_type(runtime_type));
    EvalStackOp::set_return(ret, base_type);
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::IsGenericTypeDefinition
static RtResultVoid is_generic_type_definition_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeTypeHandle::is_generic_type_definition(runtime_type));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::GetGenericParameterInfo(System.RuntimeType)
static RtResultVoid get_generic_parameter_info_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtGenericParam*, generic_param, SystemRuntimeTypeHandle::get_generic_parameter_info(runtime_type));
    EvalStackOp::set_return(ret, generic_param);
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::is_subclass_of
static RtResultVoid is_subclass_of_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto child_type_sig = EvalStackOp::get_param<const metadata::RtTypeSig*>(params, 0);
    auto parent_type_sig = EvalStackOp::get_param<const metadata::RtTypeSig*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeTypeHandle::is_subclass_of(child_type_sig, parent_type_sig));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::IsByRefLike(System.RuntimeType)
static RtResultVoid is_by_ref_like_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeTypeHandle::is_by_ref_like(runtime_type));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::type_is_assignable_from
static RtResultVoid type_is_assignable_from_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                    const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto to_ref_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto from_ref_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeTypeHandle::type_is_assignable_from(to_ref_type, from_ref_type));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::GetAssembly
static RtResultVoid get_assembly_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                         interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionAssembly*, assembly, SystemRuntimeTypeHandle::get_assembly(runtime_type));
    EvalStackOp::set_return(ret, assembly);
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::IsInstanceOfType
static RtResultVoid is_instance_of_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    auto obj = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeTypeHandle::is_instance_of_type(runtime_type, obj));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::GetGenericTypeDefinition_impl
static RtResultVoid get_generic_type_definition_impl_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionRuntimeType*, generic_type_def,
                                            SystemRuntimeTypeHandle::get_generic_type_definition_impl(runtime_type));
    EvalStackOp::set_return(ret, generic_type_def);
    RET_VOID_OK();
}

/// @icall: System.RuntimeTypeHandle::GetModule(System.RuntimeType)
static RtResultVoid get_module_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    auto runtime_type = EvalStackOp::get_param<const vm::RtReflectionRuntimeType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionModule*, module, SystemRuntimeTypeHandle::get_module(runtime_type));
    EvalStackOp::set_return(ret, module);
    RET_VOID_OK();
}

/// @icall:
/// System.RuntimeTypeHandle::internal_from_name(System.String,System.Threading.StackCrawlMark&,System.Reflection.Assembly,System.Boolean,System.Boolean,System.Boolean)
static RtResultVoid internal_from_name_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto name = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto stack_crawl_mark = EvalStackOp::get_param<int32_t*>(params, 1);
    auto assembly = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 2);
    auto throw_on_error = EvalStackOp::get_param<bool>(params, 3);
    auto ignore_case = EvalStackOp::get_param<bool>(params, 4);
    auto reflection_only = EvalStackOp::get_param<bool>(params, 5);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        vm::RtReflectionType*, type_obj,
        SystemRuntimeTypeHandle::internal_from_name(name, stack_crawl_mark, assembly, throw_on_error, ignore_case, reflection_only));
    EvalStackOp::set_return(ret, type_obj);
    RET_VOID_OK();
}

// Internal call registry
static vm::InternalCallEntry s_internal_call_entries_system_runtimetypehandle[] = {
    {"System.RuntimeTypeHandle::GetAttributes", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::get_attributes, get_attributes_invoker},
    {"System.RuntimeTypeHandle::GetMetadataToken(System.RuntimeType)", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::get_metadata_token,
     get_metadata_token_invoker_system_runtimetypehandle},
    {"System.RuntimeTypeHandle::GetCorElementType", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::get_cor_element_type, get_cor_element_type_invoker},
    {"System.RuntimeTypeHandle::HasInstantiation", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::has_instantiation, has_instantiation_invoker},
    {"System.RuntimeTypeHandle::IsComObject(System.RuntimeType)", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::is_com_object, is_com_object_invoker},
    {"System.RuntimeTypeHandle::HasReferences", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::has_references, has_references_invoker},
    {"System.RuntimeTypeHandle::GetArrayRank(System.RuntimeType)", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::get_array_rank, get_array_rank_invoker},
    {"System.RuntimeTypeHandle::GetElementType", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::get_element_type, get_element_type_invoker},
    {"System.RuntimeTypeHandle::IsGenericVariable", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::is_generic_variable, is_generic_variable_invoker},
    {"System.RuntimeTypeHandle::GetBaseType", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::get_base_type, get_base_type_invoker},
    {"System.RuntimeTypeHandle::IsGenericTypeDefinition", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::is_generic_type_definition,
     is_generic_type_definition_invoker},
    {"System.RuntimeTypeHandle::GetGenericParameterInfo(System.RuntimeType)", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::get_generic_parameter_info,
     get_generic_parameter_info_invoker},
    {"System.RuntimeTypeHandle::is_subclass_of", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::is_subclass_of, is_subclass_of_invoker},
    {"System.RuntimeTypeHandle::IsByRefLike(System.RuntimeType)", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::is_by_ref_like, is_by_ref_like_invoker},
    {"System.RuntimeTypeHandle::type_is_assignable_from", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::type_is_assignable_from,
     type_is_assignable_from_invoker},
    {"System.RuntimeTypeHandle::GetAssembly", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::get_assembly, get_assembly_invoker},
    {"System.RuntimeTypeHandle::IsInstanceOfType", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::is_instance_of_type, is_instance_of_type_invoker},
    {"System.RuntimeTypeHandle::GetGenericTypeDefinition_impl", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::get_generic_type_definition_impl,
     get_generic_type_definition_impl_invoker},
    {"System.RuntimeTypeHandle::GetModule(System.RuntimeType)", (vm::InternalCallFunction)&SystemRuntimeTypeHandle::get_module, get_module_invoker},
    {"System.RuntimeTypeHandle::internal_from_name(System.String,System.Threading.StackCrawlMark&,System.Reflection.Assembly,System.Boolean,System.Boolean,"
     "System.Boolean)",
     (vm::InternalCallFunction)&SystemRuntimeTypeHandle::internal_from_name, internal_from_name_invoker},
};

utils::Span<vm::InternalCallEntry> SystemRuntimeTypeHandle::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_runtimetypehandle,
                                              sizeof(s_internal_call_entries_system_runtimetypehandle) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

#include "system_reflection_runtimemodule.h"

#include <cstring>

#include "icall_base.h"
#include "vm/reflection.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/assembly.h"
#include "vm/rt_array.h"
#include "metadata/module_def.h"
#include "metadata/metadata_cache.h"

namespace leanclr
{
namespace icalls
{

RtResult<int32_t> SystemReflectionRuntimeModule::get_metadata_token(vm::RtReflectionModule* module) noexcept
{
    RET_OK(static_cast<int32_t>(module->assembly->assembly->mod->get_module_token()));
}

/// @icall: System.Reflection.RuntimeModule::get_MetadataToken(System.Reflection.Module)
static RtResultVoid get_metadata_token_invoker_system_reflection_runtimemodule(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<vm::RtReflectionModule*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, token, SystemReflectionRuntimeModule::get_metadata_token(module));
    EvalStackOp::set_return(ret, token);
    RET_VOID_OK();
}

RtResult<int32_t> SystemReflectionRuntimeModule::get_md_stream_version(intptr_t module) noexcept
{
    (void)module;
    // Metadata stream version is not exposed in this runtime; return 0 as a benign default.
    RET_OK(0);
}

/// @icall: System.Reflection.RuntimeModule::GetMDStreamVersion(System.IntPtr)
static RtResultVoid get_md_stream_version_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<intptr_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, version, SystemReflectionRuntimeModule::get_md_stream_version(module));
    EvalStackOp::set_return(ret, version);
    RET_VOID_OK();
}

RtResult<vm::RtArray*> SystemReflectionRuntimeModule::internal_get_types(metadata::RtModuleDef* module) noexcept
{
    return vm::Assembly::get_types(module->get_assembly(), false);
}

/// @icall: System.Reflection.RuntimeModule::InternalGetTypes(System.IntPtr)
static RtResultVoid internal_get_types_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<metadata::RtModuleDef*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, types, SystemReflectionRuntimeModule::internal_get_types(module));
    EvalStackOp::set_return(ret, types);
    RET_VOID_OK();
}

RtResult<intptr_t> SystemReflectionRuntimeModule::get_hinstance(metadata::RtModuleDef* module) noexcept
{
    (void)module;
    // Return 0 for now - HINSTANCE is not applicable in this runtime
    RET_OK(0);
}

/// @icall: System.Reflection.RuntimeModule::GetHINSTANCE(System.IntPtr)
static RtResultVoid get_hinstance_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<metadata::RtModuleDef*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, hinstance, SystemReflectionRuntimeModule::get_hinstance(module));
    EvalStackOp::set_return(ret, hinstance);
    RET_VOID_OK();
}

RtResultVoid SystemReflectionRuntimeModule::get_guid_internal(metadata::RtModuleDef* module, vm::RtArray* guid_bytes) noexcept
{
    (void)module;
    (void)guid_bytes;
    // Just return zeroed GUID - do nothing, guid_bytes will be zeroed by default allocation
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeModule::GetGuidInternal(System.IntPtr,System.Byte[])
static RtResultVoid get_guid_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto module = EvalStackOp::get_param<metadata::RtModuleDef*>(params, 0);
    auto guid_bytes = EvalStackOp::get_param<vm::RtArray*>(params, 1);
    RET_ERR_ON_FAIL(SystemReflectionRuntimeModule::get_guid_internal(module, guid_bytes));
    RET_VOID_OK();
}

RtResult<vm::RtReflectionType*> SystemReflectionRuntimeModule::get_global_type(metadata::RtModuleDef* module) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, global_cls, module->get_global_type_def());
    return vm::Reflection::get_klass_reflection_object(global_cls);
}

/// @icall: System.Reflection.RuntimeModule::GetGlobalType(System.IntPtr)
static RtResultVoid get_global_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<metadata::RtModuleDef*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, global_type, SystemReflectionRuntimeModule::get_global_type(module));
    EvalStackOp::set_return(ret, global_type);
    RET_VOID_OK();
}

enum class ResolveTokenError
{
    OutOfRange,
    BadTable,
    Other,
};

RtResult<const metadata::RtTypeSig*> SystemReflectionRuntimeModule::resolve_type_token(metadata::RtModuleDef* module, int32_t token, vm::RtArray* type_args,
                                                                                       vm::RtArray* method_args, int32_t* error) noexcept
{
    const metadata::RtTypeSig** typesig_arr = type_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(type_args) : nullptr;
    const metadata::RtTypeSig** methodsig_arr = method_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(method_args) : nullptr;

    const metadata::RtGenericInst* class_inst;
    if (type_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(class_inst,
                                  metadata::MetadataCache::get_pooled_generic_inst(typesig_arr, static_cast<uint8_t>(vm::Array::get_array_length(type_args))));
    }
    else
    {
        class_inst = nullptr;
    }
    const metadata::RtGenericInst* method_inst;
    if (method_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(
            method_inst, metadata::MetadataCache::get_pooled_generic_inst(methodsig_arr, static_cast<uint8_t>(vm::Array::get_array_length(method_args))));
    }
    else
    {
        method_inst = nullptr;
    }
    metadata::RtGenericContainerContext gcc{};
    metadata::RtGenericContext gc{class_inst, method_inst};
    metadata::RtToken rt_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token));
    auto ret = module->get_typesig_by_type_def_ref_spec_token(rt_token, gcc, &gc);
    if (ret.is_err())
    {
        *error = (int32_t)ResolveTokenError::Other;
        RET_OK(nullptr);
    }
    else
    {
        return ret;
    }
}

/// @icall: System.Reflection.RuntimeModule::ResolveTypeToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)
static RtResultVoid resolve_type_token_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<metadata::RtModuleDef*>(params, 0);
    auto token = EvalStackOp::get_param<int32_t>(params, 1);
    auto type_args = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    auto method_args = EvalStackOp::get_param<vm::RtArray*>(params, 3);
    auto error = EvalStackOp::get_param<int32_t*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig,
                                            SystemReflectionRuntimeModule::resolve_type_token(module, token, type_args, method_args, error));
    EvalStackOp::set_return(ret, type_sig);
    RET_VOID_OK();
}

RtResult<const metadata::RtMethodInfo*> SystemReflectionRuntimeModule::resolve_method_token(metadata::RtModuleDef* module, int32_t token,
                                                                                            vm::RtArray* type_args, vm::RtArray* method_args,
                                                                                            int32_t* error) noexcept
{
    const metadata::RtTypeSig** typesig_arr = type_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(type_args) : nullptr;
    const metadata::RtTypeSig** methodsig_arr = method_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(method_args) : nullptr;

    const metadata::RtGenericInst* class_inst;
    if (type_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(class_inst,
                                  metadata::MetadataCache::get_pooled_generic_inst(typesig_arr, static_cast<uint8_t>(vm::Array::get_array_length(type_args))));
    }
    else
    {
        class_inst = nullptr;
    }
    const metadata::RtGenericInst* method_inst;
    if (method_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(
            method_inst, metadata::MetadataCache::get_pooled_generic_inst(methodsig_arr, static_cast<uint8_t>(vm::Array::get_array_length(method_args))));
    }
    else
    {
        method_inst = nullptr;
    }
    metadata::RtGenericContainerContext gcc{};
    metadata::RtGenericContext gc{class_inst, method_inst};
    metadata::RtToken rt_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token));
    auto ret = module->get_method_by_token(rt_token, gcc, &gc);
    if (ret.is_err())
    {
        *error = (int32_t)ResolveTokenError::Other;
        RET_OK(nullptr);
    }
    else
    {
        return ret;
    }
}

/// @icall: System.Reflection.RuntimeModule::ResolveMethodToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)
static RtResultVoid resolve_method_token_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<metadata::RtModuleDef*>(params, 0);
    auto token = EvalStackOp::get_param<int32_t>(params, 1);
    auto type_args = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    auto method_args = EvalStackOp::get_param<vm::RtArray*>(params, 3);
    auto error = EvalStackOp::get_param<int32_t*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method_info,
                                            SystemReflectionRuntimeModule::resolve_method_token(module, token, type_args, method_args, error));
    EvalStackOp::set_return(ret, method_info);
    RET_VOID_OK();
}

RtResult<const metadata::RtFieldInfo*> SystemReflectionRuntimeModule::resolve_field_token(metadata::RtModuleDef* module, int32_t token, vm::RtArray* type_args,
                                                                                          vm::RtArray* method_args, int32_t* error) noexcept
{
    const metadata::RtTypeSig** typesig_arr = type_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(type_args) : nullptr;
    const metadata::RtTypeSig** methodsig_arr = method_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(method_args) : nullptr;

    const metadata::RtGenericInst* class_inst;
    if (type_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(class_inst,
                                  metadata::MetadataCache::get_pooled_generic_inst(typesig_arr, static_cast<uint8_t>(vm::Array::get_array_length(type_args))));
    }
    else
    {
        class_inst = nullptr;
    }
    const metadata::RtGenericInst* method_inst;
    if (method_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(
            method_inst, metadata::MetadataCache::get_pooled_generic_inst(methodsig_arr, static_cast<uint8_t>(vm::Array::get_array_length(method_args))));
    }
    else
    {
        method_inst = nullptr;
    }
    metadata::RtGenericContainerContext gcc{};
    metadata::RtGenericContext gc{class_inst, method_inst};
    metadata::RtToken rt_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token));
    auto ret = module->get_field_by_token(rt_token, gcc, &gc);
    if (ret.is_err())
    {
        *error = (int32_t)ResolveTokenError::Other;
        RET_OK(nullptr);
    }
    else
    {
        return ret;
    }
}

/// @icall: System.Reflection.RuntimeModule::ResolveFieldToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)
static RtResultVoid resolve_field_token_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<metadata::RtModuleDef*>(params, 0);
    auto token = EvalStackOp::get_param<int32_t>(params, 1);
    auto type_args = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    auto method_args = EvalStackOp::get_param<vm::RtArray*>(params, 3);
    auto error = EvalStackOp::get_param<int32_t*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field_info,
                                            SystemReflectionRuntimeModule::resolve_field_token(module, token, type_args, method_args, error));
    EvalStackOp::set_return(ret, field_info);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemReflectionRuntimeModule::resolve_string_token(metadata::RtModuleDef* module, int32_t token, int32_t* error) noexcept
{
    metadata::RtToken rt_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token));
    if (rt_token.table_type != metadata::TableType::String)
    {
        *error = (int32_t)ResolveTokenError::BadTable;
        RET_OK(nullptr);
    }
    auto ret = module->get_user_string(rt_token.rid);
    if (ret.is_err())
    {
        *error = (int32_t)ResolveTokenError::Other;
        RET_OK(nullptr);
    }
    else
    {
        return ret;
    }
}

/// @icall: System.Reflection.RuntimeModule::ResolveStringToken(System.IntPtr,System.Int32,System.Reflection.ResolveTokenError&)
static RtResultVoid resolve_string_token_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<metadata::RtModuleDef*>(params, 0);
    auto token = EvalStackOp::get_param<int32_t>(params, 1);
    auto error = EvalStackOp::get_param<int32_t*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, string, SystemReflectionRuntimeModule::resolve_string_token(module, token, error));
    EvalStackOp::set_return(ret, string);
    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemReflectionRuntimeModule::resolve_member_token(metadata::RtModuleDef* module, int32_t token, vm::RtArray* type_args,
                                                                            vm::RtArray* method_args, int32_t* error) noexcept
{
    const metadata::RtTypeSig** typesig_arr = type_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(type_args) : nullptr;
    const metadata::RtTypeSig** methodsig_arr = method_args ? vm::Array::get_array_data_start_as<const metadata::RtTypeSig*>(method_args) : nullptr;

    const metadata::RtGenericInst* class_inst;
    if (type_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(class_inst,
                                  metadata::MetadataCache::get_pooled_generic_inst(typesig_arr, static_cast<uint8_t>(vm::Array::get_array_length(type_args))));
    }
    else
    {
        class_inst = nullptr;
    }
    const metadata::RtGenericInst* method_inst;
    if (method_args != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(
            method_inst, metadata::MetadataCache::get_pooled_generic_inst(methodsig_arr, static_cast<uint8_t>(vm::Array::get_array_length(method_args))));
    }
    else
    {
        method_inst = nullptr;
    }
    metadata::RtGenericContainerContext gcc{};
    metadata::RtGenericContext gc{class_inst, method_inst};
    metadata::RtToken rt_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token));
    vm::RtObject* ret = nullptr;
    switch (rt_token.table_type)
    {
    case metadata::TableType::TypeDef:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, module->get_class_by_type_def_rid(rt_token.rid));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, ref_type, vm::Reflection::get_klass_reflection_object(klass));
        ret = (vm::RtObject*)ref_type;
        break;
    }
    case metadata::TableType::Field:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field_info, module->get_field_by_rid(rt_token.rid));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionField*, field_obj, vm::Reflection::get_field_reflection_object(field_info, field_info->parent));
        ret = (vm::RtObject*)field_obj;
        break;
    }
    case metadata::TableType::Method:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method_info, module->get_method_by_rid(rt_token.rid));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, method_obj,
                                                vm::Reflection::get_method_reflection_object(method_info, method_info->parent));
        ret = (vm::RtObject*)method_obj;
        break;
    }
    case metadata::TableType::MemberRef:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtRuntimeHandle, handle, module->get_member_ref_by_rid(rt_token.rid, gcc, &gc));
        switch (handle.type)
        {
        case metadata::RtRuntimeHandleType::Method:
        {
            const metadata::RtMethodInfo* method_info = handle.method;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, method_obj,
                                                    vm::Reflection::get_method_reflection_object(method_info, method_info->parent));
            ret = (vm::RtObject*)method_obj;
            break;
        }
        case metadata::RtRuntimeHandleType::Field:
        {
            const metadata::RtFieldInfo* field_info = handle.field;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionField*, field_obj,
                                                    vm::Reflection::get_field_reflection_object(field_info, field_info->parent));
            ret = (vm::RtObject*)field_obj;
            break;
        }
        case metadata::RtRuntimeHandleType::Type:
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(handle.typeSig));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, ref_type, vm::Reflection::get_klass_reflection_object(klass));
            ret = (vm::RtObject*)ref_type;
            break;
        }
        default:
        {
            assert(false && "Invalid MemberRef handle type");
            *error = (int32_t)ResolveTokenError::Other;
            ret = nullptr;
        }
        }
        break;
    }
    default:
    {
        *error = (int32_t)ResolveTokenError::BadTable;
        ret = nullptr;
    }
    }
    RET_OK(ret);
}

RtResult<vm::RtArray*> SystemReflectionRuntimeModule::resolve_signature(metadata::RtModuleDef* module, int32_t token, int32_t* error) noexcept
{
    metadata::RtToken rt_token = metadata::RtToken::decode(static_cast<metadata::EncodedTokenId>(token));
    const metadata::CliImage& cli_image = module->get_cli_image();

    uint32_t signature = 0;
    switch (rt_token.table_type)
    {
    case metadata::TableType::TypeSpec:
    {
        auto row_type_spec = cli_image.read_type_spec(rt_token.rid);
        if (!row_type_spec)
        {
            *error = (int32_t)ResolveTokenError::OutOfRange;
            RET_OK(nullptr);
        }
        signature = row_type_spec->signature;
        break;
    }
    case metadata::TableType::Field:
    {
        auto row_field = cli_image.read_field(rt_token.rid);
        if (!row_field)
        {
            *error = (int32_t)ResolveTokenError::OutOfRange;
            RET_OK(nullptr);
        }
        signature = row_field->signature;
        break;
    }
    case metadata::TableType::Method:
    {
        auto row_method = cli_image.read_method(rt_token.rid);
        if (!row_method)
        {
            *error = (int32_t)ResolveTokenError::OutOfRange;
            RET_OK(nullptr);
        }
        signature = row_method->signature;
        break;
    }
    case metadata::TableType::MemberRef:
    {
        auto row_member_ref = cli_image.read_member_ref(rt_token.rid);
        if (!row_member_ref)
        {
            *error = (int32_t)ResolveTokenError::OutOfRange;
            RET_OK(nullptr);
        }
        signature = row_member_ref->signature;
        break;
    }
    case metadata::TableType::StandaloneSig:
    {
        auto row_standalone = cli_image.read_stand_alone_sig(rt_token.rid);
        if (!row_standalone)
        {
            *error = (int32_t)ResolveTokenError::OutOfRange;
            RET_OK(nullptr);
        }
        signature = row_standalone->signature;
        break;
    }
    default:
    {
        *error = (int32_t)ResolveTokenError::BadTable;
        RET_OK(nullptr);
    }
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, blob_reader, module->get_decoded_blob_reader(signature));
    metadata::RtClass* byte_klass = vm::Class::get_corlib_types().cls_byte;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, byte_arr,
                                            vm::Array::new_szarray_from_ele_klass(byte_klass, static_cast<int32_t>(blob_reader.length())));
    std::memcpy(vm::Array::get_array_data_start_as<uint8_t>(byte_arr), blob_reader.data(), blob_reader.length());
    RET_OK(byte_arr);
}

/// @icall: System.Reflection.RuntimeModule::ResolveSignature(System.IntPtr,System.Int32,System.Reflection.ResolveTokenError&)
static RtResultVoid resolve_signature_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<metadata::RtModuleDef*>(params, 0);
    auto token = EvalStackOp::get_param<int32_t>(params, 1);
    auto error = EvalStackOp::get_param<int32_t*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, sig, SystemReflectionRuntimeModule::resolve_signature(module, token, error));
    EvalStackOp::set_return(ret, sig);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeModule::ResolveMemberToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)
static RtResultVoid resolve_member_token_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto module = EvalStackOp::get_param<metadata::RtModuleDef*>(params, 0);
    auto token = EvalStackOp::get_param<int32_t>(params, 1);
    auto type_args = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    auto method_args = EvalStackOp::get_param<vm::RtArray*>(params, 3);
    auto error = EvalStackOp::get_param<int32_t*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, member,
                                            SystemReflectionRuntimeModule::resolve_member_token(module, token, type_args, method_args, error));
    EvalStackOp::set_return(ret, member);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemReflectionRuntimeModule::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Reflection.RuntimeModule::get_MetadataToken(System.Reflection.Module)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::get_metadata_token, get_metadata_token_invoker_system_reflection_runtimemodule},
        {"System.Reflection.RuntimeModule::GetMDStreamVersion(System.IntPtr)", (vm::InternalCallFunction)&SystemReflectionRuntimeModule::get_md_stream_version,
         get_md_stream_version_invoker},
        {"System.Reflection.RuntimeModule::InternalGetTypes(System.IntPtr)", (vm::InternalCallFunction)&SystemReflectionRuntimeModule::internal_get_types,
         internal_get_types_invoker},
        {"System.Reflection.RuntimeModule::GetHINSTANCE(System.IntPtr)", (vm::InternalCallFunction)&SystemReflectionRuntimeModule::get_hinstance,
         get_hinstance_invoker},
        {"System.Reflection.RuntimeModule::GetGuidInternal(System.IntPtr,System.Byte[])",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::get_guid_internal, get_guid_internal_invoker},
        {"System.Reflection.RuntimeModule::GetGlobalType(System.IntPtr)", (vm::InternalCallFunction)&SystemReflectionRuntimeModule::get_global_type,
         get_global_type_invoker},
        {"System.Reflection.RuntimeModule::ResolveTypeToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::resolve_type_token, resolve_type_token_invoker},
        {"System.Reflection.RuntimeModule::ResolveMethodToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::resolve_method_token, resolve_method_token_invoker},
        {"System.Reflection.RuntimeModule::ResolveFieldToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::resolve_field_token, resolve_field_token_invoker},
        {"System.Reflection.RuntimeModule::ResolveStringToken(System.IntPtr,System.Int32,System.Reflection.ResolveTokenError&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::resolve_string_token, resolve_string_token_invoker},
        {"System.Reflection.RuntimeModule::ResolveMemberToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::resolve_member_token, resolve_member_token_invoker},
        {"System.Reflection.RuntimeModule::ResolveSignature(System.IntPtr,System.Int32,System.Reflection.ResolveTokenError&)",
         (vm::InternalCallFunction)&SystemReflectionRuntimeModule::resolve_signature, resolve_signature_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

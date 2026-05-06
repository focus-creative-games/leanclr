#include "system_reflection_runtimeassembly.h"
#include "icall_base.h"

#include "metadata/module_def.h"
#include "utils/string_builder.h"
#include "vm/class.h"
#include "vm/reflection.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"
#include "vm/type.h"

namespace leanclr
{
namespace icalls
{

RtResult<intptr_t> SystemReflectionRuntimeAssembly::get_manifest_resource_internal(vm::RtReflectionAssembly* ref_ass, vm::RtString* name, int32_t* size,
                                                                                   vm::RtReflectionModule** module) noexcept
{
    (void)ref_ass;
    (void)name;
    *size = 0;
    *module = nullptr;
    RET_OK(0);
}

/// @icall: System.Reflection.RuntimeAssembly::GetManifestResourceInternal
static RtResultVoid get_manifest_resource_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                           interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    auto name = EvalStackOp::get_param<vm::RtString*>(params, 1);
    auto size_ptr = EvalStackOp::get_param<int32_t*>(params, 2);
    auto module_ptr = EvalStackOp::get_param<vm::RtReflectionModule**>(params, 3);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, result,
                                            SystemReflectionRuntimeAssembly::get_manifest_resource_internal(ref_ass, name, size_ptr, module_ptr));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemReflectionRuntimeAssembly::get_code_base(vm::RtReflectionAssembly* ref_ass, bool escaped) noexcept
{
    (void)escaped;
    metadata::RtModuleDef* mod = ref_ass->assembly->mod;
    RET_OK(vm::String::create_string_from_utf8cstr(mod->get_name()));
}

/// @icall: System.Reflection.RuntimeAssembly::get_code_base(System.Reflection.Assembly,System.Boolean)
static RtResultVoid get_code_base_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    auto escaped = EvalStackOp::get_param<bool>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemReflectionRuntimeAssembly::get_code_base(ref_ass, escaped));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemReflectionRuntimeAssembly::get_location(vm::RtReflectionAssembly* ref_ass) noexcept
{
    metadata::RtModuleDef* mod = ref_ass->assembly->mod;
    RET_OK(vm::String::create_string_from_utf8cstr(mod->get_name()));
}

/// @icall: System.Reflection.RuntimeAssembly::get_location
static RtResultVoid get_location_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                         interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemReflectionRuntimeAssembly::get_location(ref_ass));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemReflectionRuntimeAssembly::get_fullname(vm::RtReflectionAssembly* ref_ass) noexcept
{
    metadata::RtModuleDef* mod = ref_ass->assembly->mod;
    utils::StringBuilder sb;
    vm::Type::append_assembly_name(sb, mod->get_assembly_name());
    RET_OK(vm::String::create_string_from_utf8chars(sb.as_cstr(), static_cast<int32_t>(sb.length())));
}

/// @icall: System.Reflection.RuntimeAssembly::get_fullname(System.Reflection.Assembly)
static RtResultVoid get_fullname_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                         interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemReflectionRuntimeAssembly::get_fullname(ref_ass));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> SystemReflectionRuntimeAssembly::get_aot_id_internal(vm::RtReflectionAssembly* ref_ass, vm::RtArray* buffer) noexcept
{
    (void)ref_ass;
    (void)buffer;
    RET_OK(false);
}

/// @icall: System.Reflection.RuntimeAssembly::GetAotIdInternal(System.Byte[])
static RtResultVoid get_aot_id_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    auto buffer = EvalStackOp::get_param<vm::RtArray*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemReflectionRuntimeAssembly::get_aot_id_internal(ref_ass, buffer));
    EvalStackOp::set_return(ret, (int32_t)result);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemReflectionRuntimeAssembly::internal_image_runtime_version(vm::RtReflectionAssembly* ref_ass) noexcept
{
    (void)ref_ass;
    // .net 2.0-3.5
    // .net 4.x "v4.0.30319"
    // .net core/5+ "v4.0.0"
    RET_OK(vm::String::create_string_from_utf8cstr("v4.0.30319"));
}

/// @icall: System.Reflection.RuntimeAssembly::InternalImageRuntimeVersion(System.Reflection.Assembly)
static RtResultVoid internal_image_runtime_version_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                           interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemReflectionRuntimeAssembly::internal_image_runtime_version(ref_ass));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> SystemReflectionRuntimeAssembly::get_global_assembly_cache(vm::RtReflectionAssembly* ref_ass) noexcept
{
    (void)ref_ass;
    RET_OK(false);
}

/// @icall: System.Reflection.RuntimeAssembly::get_global_assembly_cache
static RtResultVoid get_global_assembly_cache_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                      interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemReflectionRuntimeAssembly::get_global_assembly_cache(ref_ass));
    EvalStackOp::set_return(ret, (int32_t)result);
    RET_VOID_OK();
}

RtResult<vm::RtReflectionMethod*> SystemReflectionRuntimeAssembly::get_entry_point(vm::RtReflectionAssembly* ref_ass) noexcept
{
    metadata::RtModuleDef* mod = ref_ass->assembly->mod;
    metadata::EncodedTokenId entrypoint_token = mod->get_entrypoint_token();
    if (entrypoint_token == 0)
    {
        RET_OK(nullptr);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, mod->get_method_by_rid(metadata::RtToken::decode_rid(entrypoint_token)));
    return vm::Reflection::get_method_reflection_object(method, method->parent);
}

/// @icall: System.Reflection.RuntimeAssembly::get_EntryPoint
static RtResultVoid get_entry_point_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                            interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionMethod*, result, SystemReflectionRuntimeAssembly::get_entry_point(ref_ass));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> SystemReflectionRuntimeAssembly::get_reflection_only(vm::RtReflectionAssembly* ref_ass) noexcept
{
    metadata::RtModuleDef* mod = ref_ass->assembly->mod;
    RET_OK(mod->get_ref_only());
}

/// @icall: System.Reflection.RuntimeAssembly::get_ReflectionOnly
static RtResultVoid get_reflection_only_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemReflectionRuntimeAssembly::get_reflection_only(ref_ass));
    EvalStackOp::set_return(ret, (int32_t)result);
    RET_VOID_OK();
}

RtResult<bool> SystemReflectionRuntimeAssembly::get_manifest_resource_info_internal(vm::RtReflectionAssembly* ref_ass, vm::RtString* name,
                                                                                    vm::RtObject* info) noexcept
{
    (void)ref_ass;
    (void)name;
    (void)info;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.Reflection.RuntimeAssembly::GetManifestResourceInfoInternal(System.String,System.Reflection.ManifestResourceInfo)
static RtResultVoid get_manifest_resource_info_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    auto name = EvalStackOp::get_param<vm::RtString*>(params, 1);
    auto info = EvalStackOp::get_param<vm::RtObject*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemReflectionRuntimeAssembly::get_manifest_resource_info_internal(ref_ass, name, info));
    EvalStackOp::set_return(ret, (int32_t)result);
    RET_VOID_OK();
}

RtResult<vm::RtArray*> SystemReflectionRuntimeAssembly::get_manifest_resource_names(vm::RtReflectionAssembly* ref_ass) noexcept
{
    (void)ref_ass;
    return vm::Array::new_empty_szarray_by_ele_klass(vm::Class::get_corlib_types().cls_string);
}

/// @icall: System.Reflection.RuntimeAssembly::GetManifestResourceNames
static RtResultVoid get_manifest_resource_names_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                        interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result, SystemReflectionRuntimeAssembly::get_manifest_resource_names(ref_ass));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtReflectionModule*> SystemReflectionRuntimeAssembly::get_manifest_module_internal(vm::RtReflectionAssembly* ref_ass) noexcept
{
    metadata::RtAssembly* ass = ref_ass->assembly;
    return vm::Reflection::get_module_reflection_object(ass->mod);
}

/// @icall: System.Reflection.RuntimeAssembly::GetManifestModuleInternal
static RtResultVoid get_manifest_module_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                         interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionModule*, result, SystemReflectionRuntimeAssembly::get_manifest_module_internal(ref_ass));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtArray*> SystemReflectionRuntimeAssembly::get_modules_internal(vm::RtReflectionAssembly* ref_ass) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, module_arr,
                                            vm::Array::new_szarray_from_ele_klass(vm::Class::get_corlib_types().cls_reflection_module, 1));
    metadata::RtAssembly* ass = ref_ass->assembly;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionModule*, ref_module, vm::Reflection::get_module_reflection_object(ass->mod));
    vm::Array::set_array_data_at<vm::RtReflectionModule*>(module_arr, 0, ref_module);
    RET_OK(module_arr);
}

/// @icall: System.Reflection.RuntimeAssembly::GetModulesInternal
static RtResultVoid get_modules_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result, SystemReflectionRuntimeAssembly::get_modules_internal(ref_ass));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemReflectionRuntimeAssembly::get_files_internal(vm::RtReflectionAssembly* ref_ass, vm::RtString* path,
                                                                            bool get_resource_modules) noexcept
{
    (void)ref_ass;
    (void)path;
    (void)get_resource_modules;
    RET_OK(nullptr);
}

/// @icall: System.Reflection.RuntimeAssembly::GetFilesInternal(System.String,System.Boolean)
static RtResultVoid get_files_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    auto ref_ass = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 1);
    auto get_resource_modules = EvalStackOp::get_param<bool>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemReflectionRuntimeAssembly::get_files_internal(ref_ass, path, get_resource_modules));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_reflection_runtimeassembly[] = {
    {"System.Reflection.RuntimeAssembly::GetManifestResourceInternal",
     (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_manifest_resource_internal, get_manifest_resource_internal_invoker},
    {"System.Reflection.RuntimeAssembly::get_code_base(System.Reflection.Assembly,System.Boolean)",
     (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_code_base, get_code_base_invoker},
    {"System.Reflection.RuntimeAssembly::get_location", (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_location, get_location_invoker},
    {"System.Reflection.RuntimeAssembly::get_fullname(System.Reflection.Assembly)", (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_fullname,
     get_fullname_invoker},
    {"System.Reflection.RuntimeAssembly::GetAotIdInternal(System.Byte[])", (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_aot_id_internal,
     get_aot_id_internal_invoker},
    {"System.Reflection.RuntimeAssembly::InternalImageRuntimeVersion(System.Reflection.Assembly)",
     (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::internal_image_runtime_version, internal_image_runtime_version_invoker},
    {"System.Reflection.RuntimeAssembly::get_global_assembly_cache", (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_global_assembly_cache,
     get_global_assembly_cache_invoker},
    {"System.Reflection.RuntimeAssembly::get_EntryPoint", (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_entry_point, get_entry_point_invoker},
    {"System.Reflection.RuntimeAssembly::get_ReflectionOnly", (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_reflection_only,
     get_reflection_only_invoker},
    {"System.Reflection.RuntimeAssembly::GetManifestResourceInfoInternal(System.String,System.Reflection.ManifestResourceInfo)",
     (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_manifest_resource_info_internal, get_manifest_resource_info_internal_invoker},
    {"System.Reflection.RuntimeAssembly::GetManifestResourceNames", (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_manifest_resource_names,
     get_manifest_resource_names_invoker},
    {"System.Reflection.RuntimeAssembly::GetManifestModuleInternal", (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_manifest_module_internal,
     get_manifest_module_internal_invoker},
    {"System.Reflection.RuntimeAssembly::GetModulesInternal", (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_modules_internal,
     get_modules_internal_invoker},
    {"System.Reflection.RuntimeAssembly::GetFilesInternal(System.String,System.Boolean)",
     (vm::InternalCallFunction)&SystemReflectionRuntimeAssembly::get_files_internal, get_files_internal_invoker},
};

utils::Span<vm::InternalCallEntry> SystemReflectionRuntimeAssembly::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_reflection_runtimeassembly,
                                              sizeof(s_internal_call_entries_system_reflection_runtimeassembly) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

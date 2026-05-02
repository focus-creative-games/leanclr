#include "system_reflection_assembly.h"
#include "icall_base.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/reflection.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"
#include "vm/type.h"
#include "vm/appdomain.h"
#include "interp/machine_state.h"
#include "metadata/module_def.h"
#include "utils/safegptrarray.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace icalls
{

// ========== Implementation Functions ==========

RtResult<vm::RtReflectionAssembly*> SystemReflectionAssembly::get_executing_assembly() noexcept
{
    interp::InterpFrame* executing_frame = interp::MachineState::get_global_machine_state().get_executing_frame_stack();
    if (executing_frame == nullptr)
    {
        metadata::RtAssembly* corlib = vm::Assembly::get_corlib();
        return vm::Reflection::get_assembly_reflection_object(corlib);
    }
    metadata::RtModuleDef* mod = executing_frame->method->parent->image;
    return vm::Reflection::get_assembly_reflection_object(mod->get_assembly());
}

RtResult<vm::RtReflectionAssembly*> SystemReflectionAssembly::get_calling_assembly() noexcept
{
    interp::InterpFrame* calling_frame = interp::MachineState::get_global_machine_state().get_calling_frame_stack();
    if (calling_frame == nullptr)
    {
        metadata::RtAssembly* corlib = vm::Assembly::get_corlib();
        return vm::Reflection::get_assembly_reflection_object(corlib);
    }
    metadata::RtModuleDef* mod = calling_frame->method->parent->image;
    return vm::Reflection::get_assembly_reflection_object(mod->get_assembly());
}

RtResult<vm::RtReflectionType*> SystemReflectionAssembly::internal_get_type(vm::RtReflectionAssembly* assembly, vm::RtReflectionModule* module,
                                                                            vm::RtString* name, bool throw_on_error, bool ignore_case) noexcept
{
    (void)module; // unused
    utils::StringBuilder name_buf;
    utils::StringUtil::utf16_to_utf8(vm::String::get_chars_ptr(name), static_cast<size_t>(vm::String::get_length(name)), name_buf);
    name_buf.sure_null_terminator_but_not_append();

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        const metadata::RtTypeSig*, resolved_type_sig,
        vm::Type::resolve_assembly_qualified_name(assembly->assembly->mod, name_buf.as_cstr(), name_buf.length(), ignore_case));
    if (resolved_type_sig == nullptr)
    {
        if (throw_on_error)
        {
            RET_ERR(RtErr::TypeLoad);
        }
        RET_OK(nullptr);
    }
    return vm::Reflection::get_type_reflection_object(resolved_type_sig);
}

RtResult<vm::RtArray*> SystemReflectionAssembly::get_types(vm::RtReflectionAssembly* ref_assembly, bool exported_only) noexcept
{
    metadata::RtAssembly* ass = ref_assembly->assembly;
    return vm::Assembly::get_types(ass, exported_only);
}

RtResult<vm::RtReflectionAssembly*> SystemReflectionAssembly::get_entry_assembly() noexcept
{
    // Find assembly with entrypoint
    for (metadata::RtModuleDef* mod : metadata::RtModuleDef::get_registered_modules())
    {
        metadata::EncodedTokenId entry_token = mod->get_entrypoint_token();
        if (entry_token != 0)
        {
            return vm::Reflection::get_assembly_reflection_object(mod->get_assembly());
        }
    }
    RET_OK(nullptr);
}

RtResultVoid SystemReflectionAssembly::internal_get_assembly_name(vm::RtString* path, metadata::RtMonoAssemblyName* aname, vm::RtString** codebase_out) noexcept
{
    (void)path;
    (void)aname;
    (void)codebase_out;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<vm::RtReflectionAssembly*> SystemReflectionAssembly::load_from(vm::RtString* path, bool ref_only, int32_t* mark) noexcept
{
    utils::StringBuilder name_buf;
    utils::StringUtil::utf16_to_utf8(vm::String::get_chars_ptr(path), static_cast<size_t>(vm::String::get_length(path)), name_buf);
    vm::RtAppDomain* current_app_domain = vm::AppDomain::get_default_appdomain();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        metadata::RtAssembly*, loaded_ass, vm::Assembly::load_by_name(current_app_domain, name_buf.as_cstr(), nullptr, ref_only, *(vm::RtStackCrawlMark*)mark));
    return vm::Reflection::get_assembly_reflection_object(loaded_ass);
}

RtResult<vm::RtReflectionAssembly*> SystemReflectionAssembly::load_file_internal(vm::RtString* path, int32_t* mark) noexcept
{
    (void)mark;
    utils::StringBuilder name_buf;
    utils::StringUtil::utf16_to_utf8(vm::String::get_chars_ptr(path), static_cast<size_t>(vm::String::get_length(path)), name_buf);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtAssembly*, loaded_ass, vm::Assembly::load_by_name(name_buf.as_cstr()));
    return vm::Reflection::get_assembly_reflection_object(loaded_ass);
}

RtResult<vm::RtReflectionAssembly*> SystemReflectionAssembly::load_with_partial_name(vm::RtString* name, vm::RtObject* evidence) noexcept
{
    (void)evidence;
    utils::StringBuilder name_buf;
    utils::StringUtil::utf16_to_utf8(vm::String::get_chars_ptr(name), static_cast<size_t>(vm::String::get_length(name)), name_buf);
    name_buf.sure_null_terminator_but_not_append();

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtAssembly*, loaded_ass, vm::Assembly::load_by_name(name_buf.as_cstr()));
    return vm::Reflection::get_assembly_reflection_object(loaded_ass);
}

RtResult<intptr_t> SystemReflectionAssembly::internal_get_referenced_assemblies(vm::RtReflectionAssembly* ref_ass) noexcept
{
    metadata::RtModuleDef* mod = ref_ass->assembly->mod;
    utils::Vector<metadata::RtAssembly*> ref_asses;
    RET_ERR_ON_FAIL(mod->get_reference_assemblies(ref_asses));

    utils::Vector<const metadata::RtMonoAssemblyName*> ass_names_buf;
    for (size_t i = 0; i < ref_asses.size(); ++i)
    {
        metadata::RtModuleDef* ref_mod = ref_asses[i]->mod;
        metadata::RtMonoAssemblyName* ass_name_info = alloc::GeneralAllocation::malloc_any_zeroed<metadata::RtMonoAssemblyName>();
        ref_mod->fill_assembly_name(*ass_name_info);
        ass_names_buf.push_back(ass_name_info);
    }

    auto gptr_arr = utils::SafeGPtrArray<metadata::RtMonoAssemblyName>::create_from_data(ass_names_buf.data(), static_cast<int32_t>(ass_names_buf.size()));
    RET_OK(reinterpret_cast<intptr_t>(gptr_arr));
}

// ========== Invoker Functions ==========

// @icall: System.Reflection.Assembly::GetExecutingAssembly
static RtResultVoid get_executing_assembly_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionAssembly*, assembly, SystemReflectionAssembly::get_executing_assembly());
    EvalStackOp::set_return(ret, assembly);
    RET_VOID_OK();
}

// @icall: System.Reflection.Assembly::GetCallingAssembly
static RtResultVoid get_calling_assembly_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionAssembly*, assembly, SystemReflectionAssembly::get_calling_assembly());
    EvalStackOp::set_return(ret, assembly);
    RET_VOID_OK();
}

// @icall: System.Reflection.Assembly::InternalGetType
static RtResultVoid internal_get_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto assembly = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    auto module = EvalStackOp::get_param<vm::RtReflectionModule*>(params, 1);
    auto name = EvalStackOp::get_param<vm::RtString*>(params, 2);
    auto throw_on_error = EvalStackOp::get_param<int32_t>(params, 3) != 0;
    auto ignore_case = EvalStackOp::get_param<int32_t>(params, 4) != 0;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, ref_type,
                                            SystemReflectionAssembly::internal_get_type(assembly, module, name, throw_on_error, ignore_case));
    EvalStackOp::set_return(ret, ref_type);
    RET_VOID_OK();
}

// @icall: System.Reflection.Assembly::GetTypes(System.Boolean)
static RtResultVoid get_types_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto assembly = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    auto exported_only = EvalStackOp::get_param<bool>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, SystemReflectionAssembly::get_types(assembly, exported_only));
    EvalStackOp::set_return(ret, arr);
    RET_VOID_OK();
}

// @icall: System.Reflection.Assembly::InternalGetAssemblyName(System.String,Mono.MonoAssemblyName&,System.String&)
static RtResultVoid internal_get_assembly_name_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto aname = EvalStackOp::get_param<metadata::RtMonoAssemblyName*>(params, 1);
    auto codebase_ref = EvalStackOp::get_param<vm::RtString**>(params, 2);
    return SystemReflectionAssembly::internal_get_assembly_name(path, aname, codebase_ref);
}

// @icall: System.Reflection.Assembly::GetEntryAssembly
static RtResultVoid get_entry_assembly_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionAssembly*, ass, SystemReflectionAssembly::get_entry_assembly());
    EvalStackOp::set_return(ret, ass);
    RET_VOID_OK();
}

// @icall: System.Reflection.Assembly::LoadFrom(System.String,System.Boolean,System.Threading.StackCrawlMark&)
static RtResultVoid load_from_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto ref_only = EvalStackOp::get_param<bool>(params, 1);
    auto mark = EvalStackOp::get_param<int32_t*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionAssembly*, ass, SystemReflectionAssembly::load_from(path, ref_only, mark));
    EvalStackOp::set_return(ret, ass);
    RET_VOID_OK();
}

// @icall: System.Reflection.Assembly::LoadFile_internal(System.String,System.Threading.StackCrawlMark&)
static RtResultVoid load_file_internal_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto path = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto mark = EvalStackOp::get_param<int32_t*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionAssembly*, ass, SystemReflectionAssembly::load_file_internal(path, mark));
    EvalStackOp::set_return(ret, ass);
    RET_VOID_OK();
}

// @icall: System.Reflection.Assembly::load_with_partial_name(System.String,System.Security.Policy.Evidence)
static RtResultVoid load_with_partial_name_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto name = EvalStackOp::get_param<vm::RtString*>(params, 0);
    auto evidence = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionAssembly*, ass, SystemReflectionAssembly::load_with_partial_name(name, evidence));
    EvalStackOp::set_return(ret, ass);
    RET_VOID_OK();
}

// @icall: System.Reflection.Assembly::InternalGetReferencedAssemblies(System.Reflection.Assembly)
static RtResultVoid internal_get_referenced_assemblies_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto assembly = EvalStackOp::get_param<vm::RtReflectionAssembly*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, res, SystemReflectionAssembly::internal_get_referenced_assemblies(assembly));
    EvalStackOp::set_return(ret, res);
    RET_VOID_OK();
}

// ========== Internal Call Entries ==========

static vm::InternalCallEntry s_internal_call_entries_system_reflection_assembly[] = {
    {"System.Reflection.Assembly::GetExecutingAssembly", (vm::InternalCallFunction)&SystemReflectionAssembly::get_executing_assembly,
     get_executing_assembly_invoker},
    {"System.Reflection.Assembly::GetCallingAssembly", (vm::InternalCallFunction)&SystemReflectionAssembly::get_calling_assembly, get_calling_assembly_invoker},
    {"System.Reflection.Assembly::InternalGetType", (vm::InternalCallFunction)&SystemReflectionAssembly::internal_get_type, internal_get_type_invoker},
    {"System.Reflection.Assembly::GetTypes(System.Boolean)", (vm::InternalCallFunction)&SystemReflectionAssembly::get_types, get_types_invoker},
    {"System.Reflection.Assembly::InternalGetAssemblyName(System.String,Mono.MonoAssemblyName&,System.String&)",
     (vm::InternalCallFunction)&SystemReflectionAssembly::internal_get_assembly_name, internal_get_assembly_name_invoker},
    {"System.Reflection.Assembly::GetEntryAssembly", (vm::InternalCallFunction)&SystemReflectionAssembly::get_entry_assembly, get_entry_assembly_invoker},
    {"System.Reflection.Assembly::LoadFrom(System.String,System.Boolean,System.Threading.StackCrawlMark&)",
     (vm::InternalCallFunction)&SystemReflectionAssembly::load_from, load_from_invoker},
    {"System.Reflection.Assembly::LoadFile_internal(System.String,System.Threading.StackCrawlMark&)",
     (vm::InternalCallFunction)&SystemReflectionAssembly::load_file_internal, load_file_internal_invoker},
    {"System.Reflection.Assembly::load_with_partial_name(System.String,System.Security.Policy.Evidence)",
     (vm::InternalCallFunction)&SystemReflectionAssembly::load_with_partial_name, load_with_partial_name_invoker},
    {"System.Reflection.Assembly::InternalGetReferencedAssemblies(System.Reflection.Assembly)",
     (vm::InternalCallFunction)&SystemReflectionAssembly::internal_get_referenced_assemblies, internal_get_referenced_assemblies_invoker},
};

utils::Span<vm::InternalCallEntry> SystemReflectionAssembly::get_internal_call_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_internal_call_entries_system_reflection_assembly) / sizeof(s_internal_call_entries_system_reflection_assembly[0]);
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_reflection_assembly, entry_count);
}

} // namespace icalls
} // namespace leanclr

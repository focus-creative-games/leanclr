#include "system_appdomain.h"
#include "icall_base.h"

#include "utils/string_builder.h"
#include "vm/assembly.h"
#include "vm/appdomain.h"
#include "vm/reflection.h"
#include "vm/class.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"
#include "vm/rt_thread.h"
#include "metadata/module_def.h"

namespace leanclr
{
namespace icalls
{

RtResult<vm::RtObject*> SystemAppDomain::get_setup() noexcept
{
    RET_OK(vm::AppDomain::get_setup());
}

/// @icall: System.AppDomain::getSetup()
static RtResultVoid get_setup_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemAppDomain::get_setup());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemAppDomain::get_friendly_name() noexcept
{
    const char* name = vm::AppDomain::get_friendly_name();
    RET_OK(vm::String::create_string_from_utf8cstr(name));
}

/// @icall: System.AppDomain::getFriendlyName()
static RtResultVoid get_friendly_name_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemAppDomain::get_friendly_name());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtAppDomain*> SystemAppDomain::get_cur_domain() noexcept
{
    RET_OK(vm::AppDomain::get_default_appdomain());
}

/// @icall: System.AppDomain::getCurDomain()
static RtResultVoid get_cur_domain_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtAppDomain*, result, SystemAppDomain::get_cur_domain());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtAppDomain*> SystemAppDomain::get_root_domain() noexcept
{
    RET_OK(vm::AppDomain::get_default_appdomain());
}

/// @icall: System.AppDomain::getRootDomain()
static RtResultVoid get_root_domain_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                            interp::RtStackObject* ret) noexcept
{
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtAppDomain*, result, SystemAppDomain::get_root_domain());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<int32_t> SystemAppDomain::execute_assembly(vm::RtObject* assembly, vm::RtObject* args) noexcept
{
    (void)assembly;
    (void)args;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.AppDomain::ExecuteAssembly(System.Reflection.Assembly,System.String[])
static RtResultVoid execute_assembly_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    auto assembly_obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    auto args = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemAppDomain::execute_assembly(assembly_obj, args));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtArray*> SystemAppDomain::get_assemblies(vm::RtAppDomain* this_domain, bool ref_only) noexcept
{
    utils::Span<metadata::RtModuleDef*> mods = vm::AppDomain::get_modules(this_domain);
    metadata::RtClass* cls_assembly = vm::Class::get_corlib_types().cls_reflection_assembly;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, ass_arr, vm::Array::new_szarray_from_ele_klass(cls_assembly, static_cast<int32_t>(mods.size())));
    for (size_t i = 0; i < mods.size(); ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionAssembly*, reflection_assembly,
                                                vm::Reflection::get_assembly_reflection_object(mods[i]->get_assembly()));
        vm::Array::set_array_data_at<vm::RtReflectionAssembly*>(ass_arr, static_cast<int32_t>(i), reflection_assembly);
    }
    RET_OK(ass_arr);
}

/// @icall: System.AppDomain::GetAssemblies(System.Boolean)
static RtResultVoid get_assemblies_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    auto this_domain = EvalStackOp::get_param<vm::RtAppDomain*>(params, 0);
    auto ref_only = EvalStackOp::get_param<bool>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, result, SystemAppDomain::get_assemblies(this_domain, ref_only));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemAppDomain::get_data(vm::RtAppDomain* this_domain, vm::RtString* name) noexcept
{
    (void)this_domain;
    RET_OK(vm::AppDomain::get_domain_data(name));
}

/// @icall: System.AppDomain::GetData(System.String)
static RtResultVoid get_data_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto this_domain = EvalStackOp::get_param<vm::RtAppDomain*>(params, 0);
    auto name = EvalStackOp::get_param<vm::RtString*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemAppDomain::get_data(this_domain, name));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResultVoid SystemAppDomain::set_data(vm::RtAppDomain* this_domain, vm::RtString* name, vm::RtObject* value) noexcept
{
    (void)this_domain;
    vm::AppDomain::set_domain_data(name, value);
    RET_VOID_OK();
}

/// @icall: System.AppDomain::SetData(System.String,System.Object)
static RtResultVoid set_data_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto this_domain = EvalStackOp::get_param<vm::RtAppDomain*>(params, 0);
    auto name = EvalStackOp::get_param<vm::RtString*>(params, 1);
    auto value = EvalStackOp::get_param<vm::RtObject*>(params, 2);
    RET_ERR_ON_FAIL(SystemAppDomain::set_data(this_domain, name, value));
    RET_VOID_OK();
}

RtResult<vm::RtReflectionAssembly*> SystemAppDomain::load_assembly(vm::RtAppDomain* this_domain, vm::RtString* name, vm::RtObject* evidence, bool ref_only,
                                                                   vm::RtStackCrawlMark* stack_crawl_mark) noexcept
{
    utils::StringBuilder name_buf;
    utils::StringUtil::utf16_to_utf8(vm::String::get_chars_ptr(name), static_cast<size_t>(vm::String::get_length(name)), name_buf);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtAssembly*, loaded_ass,
                                            vm::Assembly::load_by_name(this_domain, name_buf.as_cstr(), evidence, ref_only, *stack_crawl_mark));
    return vm::Reflection::get_assembly_reflection_object(loaded_ass);
}

/// @icall: System.AppDomain::LoadAssembly(System.String,System.Security.Policy.Evidence,System.Boolean,System.Threading.StackCrawlMark&)
static RtResultVoid load_assembly_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    auto this_domain = EvalStackOp::get_param<vm::RtAppDomain*>(params, 0);
    auto name = EvalStackOp::get_param<vm::RtString*>(params, 1);
    auto evidence = EvalStackOp::get_param<vm::RtObject*>(params, 2);
    auto ref_only = EvalStackOp::get_param<bool>(params, 3);
    auto mark = EvalStackOp::get_param<vm::RtStackCrawlMark*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionAssembly*, result, SystemAppDomain::load_assembly(this_domain, name, evidence, ref_only, mark));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtReflectionAssembly*> SystemAppDomain::load_assembly_raw(vm::RtAppDomain* this_domain, vm::RtArray* raw, vm::RtArray* symbols,
                                                                       vm::RtObject* evidence, bool ref_only) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtAssembly*, ass, vm::Assembly::load_from_data(this_domain, raw, symbols, evidence, ref_only));
    return vm::Reflection::get_assembly_reflection_object(ass);
}

/// @icall: System.AppDomain::LoadAssemblyRaw(System.Byte[],System.Byte[],System.Security.Policy.Evidence,System.Boolean)
static RtResultVoid load_assembly_raw_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    auto this_domain = EvalStackOp::get_param<vm::RtAppDomain*>(params, 0);
    auto raw = EvalStackOp::get_param<vm::RtArray*>(params, 1);
    auto symbols = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    auto evidence = EvalStackOp::get_param<vm::RtObject*>(params, 3);
    auto ref_only = EvalStackOp::get_param<bool>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionAssembly*, result,
                                            SystemAppDomain::load_assembly_raw(this_domain, raw, symbols, evidence, ref_only));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemAppDomain::internal_set_domain_by_id(int32_t id) noexcept
{
    (void)id;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.AppDomain::InternalSetDomainByID(System.Int32)
static RtResultVoid internal_set_domain_by_id_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                      interp::RtStackObject* ret) noexcept
{
    auto id = EvalStackOp::get_param<int32_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemAppDomain::internal_set_domain_by_id(id));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemAppDomain::internal_set_domain(vm::RtObject* domain) noexcept
{
    (void)domain;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.AppDomain::InternalSetDomain(System.AppDomain)
static RtResultVoid internal_set_domain_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                interp::RtStackObject* ret) noexcept
{
    auto domain = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemAppDomain::internal_set_domain(domain));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResultVoid SystemAppDomain::internal_push_domain_ref(vm::RtObject* domain) noexcept
{
    (void)domain;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.AppDomain::InternalPushDomainRef(System.AppDomain)
static RtResultVoid internal_push_domain_ref_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                     interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto domain = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    RET_ERR_ON_FAIL(SystemAppDomain::internal_push_domain_ref(domain));
    RET_VOID_OK();
}

RtResultVoid SystemAppDomain::internal_push_domain_ref_by_id(int32_t id) noexcept
{
    (void)id;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.AppDomain::InternalPushDomainRefByID(System.Int32)
static RtResultVoid internal_push_domain_ref_by_id_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                           interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto id = EvalStackOp::get_param<int32_t>(params, 0);
    RET_ERR_ON_FAIL(SystemAppDomain::internal_push_domain_ref_by_id(id));
    RET_VOID_OK();
}

RtResultVoid SystemAppDomain::internal_pop_domain_ref() noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.AppDomain::InternalPopDomainRef()
static RtResultVoid internal_pop_domain_ref_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                    interp::RtStackObject* ret) noexcept
{
    (void)params;
    (void)ret;
    RET_ERR_ON_FAIL(SystemAppDomain::internal_pop_domain_ref());
    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemAppDomain::internal_set_context(vm::RtObject* ctx) noexcept
{
    (void)ctx;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.AppDomain::InternalSetContext(System.Runtime.Remoting.Contexts.Context)
static RtResultVoid internal_set_context_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    auto ctx = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemAppDomain::internal_set_context(ctx));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtAppContext*> SystemAppDomain::internal_get_context() noexcept
{
    RET_OK(vm::AppDomain::get_default_appcontext());
}

/// @icall: System.AppDomain::InternalGetContext()
static RtResultVoid internal_get_context_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtAppContext*, result, SystemAppDomain::internal_get_context());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtAppContext*> SystemAppDomain::internal_get_default_context() noexcept
{
    RET_OK(vm::AppDomain::get_default_appcontext());
}

/// @icall: System.AppDomain::InternalGetDefaultContext()
static RtResultVoid internal_get_default_context_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                         interp::RtStackObject* ret) noexcept
{
    (void)params;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtAppContext*, result, SystemAppDomain::internal_get_default_context());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtString*> SystemAppDomain::internal_get_process_guid(vm::RtObject* new_guid) noexcept
{
    (void)new_guid;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.AppDomain::InternalGetProcessGuid(System.String)
static RtResultVoid internal_get_process_guid_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                      interp::RtStackObject* ret) noexcept
{
    auto guid = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, result, SystemAppDomain::internal_get_process_guid(guid));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemAppDomain::create_domain(vm::RtObject* friendly_name, vm::RtObject* setup) noexcept
{
    (void)friendly_name;
    (void)setup;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.AppDomain::createDomain(System.String,System.AppDomainSetup)
static RtResultVoid create_domain_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    auto friendly_name = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    auto setup = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemAppDomain::create_domain(friendly_name, setup));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

RtResult<bool> SystemAppDomain::internal_is_finalizing_for_unload(int32_t id) noexcept
{
    (void)id;
    RET_OK(false);
}

/// @icall: System.AppDomain::InternalIsFinalizingForUnload(System.Int32)
static RtResultVoid internal_is_finalizing_for_unload_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto id = EvalStackOp::get_param<int32_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemAppDomain::internal_is_finalizing_for_unload(id));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

RtResultVoid SystemAppDomain::internal_unload(int32_t id) noexcept
{
    (void)id;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.AppDomain::InternalUnload(System.Int32)
static RtResultVoid internal_unload_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                            interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto id = EvalStackOp::get_param<int32_t>(params, 0);
    RET_ERR_ON_FAIL(SystemAppDomain::internal_unload(id));
    RET_VOID_OK();
}

RtResultVoid SystemAppDomain::do_unhandled_exception(vm::RtObject* ex) noexcept
{
    (void)ex;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

/// @icall: System.AppDomain::DoUnhandledException(System.Exception)
static RtResultVoid do_unhandled_exception_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto ex = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    RET_ERR_ON_FAIL(SystemAppDomain::do_unhandled_exception(ex));
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemAppDomain::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.AppDomain::getSetup()", (vm::InternalCallFunction)&SystemAppDomain::get_setup, get_setup_invoker},
        {"System.AppDomain::getFriendlyName()", (vm::InternalCallFunction)&SystemAppDomain::get_friendly_name, get_friendly_name_invoker},
        {"System.AppDomain::getCurDomain()", (vm::InternalCallFunction)&SystemAppDomain::get_cur_domain, get_cur_domain_invoker},
        {"System.AppDomain::getRootDomain()", (vm::InternalCallFunction)&SystemAppDomain::get_root_domain, get_root_domain_invoker},
        {"System.AppDomain::ExecuteAssembly(System.Reflection.Assembly,System.String[])", (vm::InternalCallFunction)&SystemAppDomain::execute_assembly,
         execute_assembly_invoker},
        {"System.AppDomain::GetAssemblies(System.Boolean)", (vm::InternalCallFunction)&SystemAppDomain::get_assemblies, get_assemblies_invoker},
        {"System.AppDomain::GetData(System.String)", (vm::InternalCallFunction)&SystemAppDomain::get_data, get_data_invoker},
        {"System.AppDomain::SetData(System.String,System.Object)", (vm::InternalCallFunction)&SystemAppDomain::set_data, set_data_invoker},
        {"System.AppDomain::LoadAssembly(System.String,System.Security.Policy.Evidence,System.Boolean,System.Threading.StackCrawlMark&)",
         (vm::InternalCallFunction)&SystemAppDomain::load_assembly, load_assembly_invoker},
        {"System.AppDomain::LoadAssemblyRaw(System.Byte[],System.Byte[],System.Security.Policy.Evidence,System.Boolean)",
         (vm::InternalCallFunction)&SystemAppDomain::load_assembly_raw, load_assembly_raw_invoker},
        {"System.AppDomain::InternalSetDomainByID(System.Int32)", (vm::InternalCallFunction)&SystemAppDomain::internal_set_domain_by_id,
         internal_set_domain_by_id_invoker},
        {"System.AppDomain::InternalSetDomain(System.AppDomain)", (vm::InternalCallFunction)&SystemAppDomain::internal_set_domain, internal_set_domain_invoker},
        {"System.AppDomain::InternalPushDomainRef(System.AppDomain)", (vm::InternalCallFunction)&SystemAppDomain::internal_push_domain_ref,
         internal_push_domain_ref_invoker},
        {"System.AppDomain::InternalPushDomainRefByID(System.Int32)", (vm::InternalCallFunction)&SystemAppDomain::internal_push_domain_ref_by_id,
         internal_push_domain_ref_by_id_invoker},
        {"System.AppDomain::InternalPopDomainRef()", (vm::InternalCallFunction)&SystemAppDomain::internal_pop_domain_ref, internal_pop_domain_ref_invoker},
        {"System.AppDomain::InternalSetContext(System.Runtime.Remoting.Contexts.Context)", (vm::InternalCallFunction)&SystemAppDomain::internal_set_context,
         internal_set_context_invoker},
        {"System.AppDomain::InternalGetContext()", (vm::InternalCallFunction)&SystemAppDomain::internal_get_context, internal_get_context_invoker},
        {"System.AppDomain::InternalGetDefaultContext()", (vm::InternalCallFunction)&SystemAppDomain::internal_get_default_context,
         internal_get_default_context_invoker},
        {"System.AppDomain::InternalGetProcessGuid(System.String)", (vm::InternalCallFunction)&SystemAppDomain::internal_get_process_guid,
         internal_get_process_guid_invoker},
        {"System.AppDomain::createDomain(System.String,System.AppDomainSetup)", (vm::InternalCallFunction)&SystemAppDomain::create_domain,
         create_domain_invoker},
        {"System.AppDomain::InternalIsFinalizingForUnload(System.Int32)", (vm::InternalCallFunction)&SystemAppDomain::internal_is_finalizing_for_unload,
         internal_is_finalizing_for_unload_invoker},
        {"System.AppDomain::InternalUnload(System.Int32)", (vm::InternalCallFunction)&SystemAppDomain::internal_unload, internal_unload_invoker},
        {"System.AppDomain::DoUnhandledException(System.Exception)", (vm::InternalCallFunction)&SystemAppDomain::do_unhandled_exception,
         do_unhandled_exception_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

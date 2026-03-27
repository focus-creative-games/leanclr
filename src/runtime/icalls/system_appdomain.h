#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemAppDomain
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // Get setup
    static RtResult<vm::RtObject*> get_setup();

    // Get friendly name
    static RtResult<vm::RtString*> get_friendly_name();

    // Get current domain
    static RtResult<vm::RtAppDomain*> get_cur_domain();

    // Get root domain
    static RtResult<vm::RtAppDomain*> get_root_domain();

    // Execute assembly
    static RtResult<int32_t> execute_assembly(vm::RtObject* assembly, vm::RtObject* args);

    // Get assemblies
    static RtResult<vm::RtArray*> get_assemblies(bool ref_only);

    // Get data
    static RtResult<vm::RtObject*> get_data(vm::RtAppDomain* this_domain, vm::RtString* name);

    // Set data
    static RtResultVoid set_data(vm::RtAppDomain* this_domain, vm::RtString* name, vm::RtObject* value);

    // Load assembly
    static RtResult<vm::RtReflectionAssembly*> load_assembly(vm::RtAppDomain* this_domain, vm::RtString* name, vm::RtObject* evidence, bool ref_only,
                                                             vm::RtStackCrawlMark* stack_crawl_mark);

    // Load assembly raw
    static RtResult<vm::RtReflectionAssembly*> load_assembly_raw(vm::RtAppDomain* this_domain, vm::RtArray* raw, vm::RtArray* symbols, vm::RtObject* evidence,
                                                                 bool ref_only);

    // Internal set domain by ID
    static RtResult<vm::RtObject*> internal_set_domain_by_id(int32_t id);

    // Internal set domain
    static RtResult<vm::RtObject*> internal_set_domain(vm::RtObject* domain);

    // Internal push domain ref
    static RtResultVoid internal_push_domain_ref(vm::RtObject* domain);

    // Internal push domain ref by ID
    static RtResultVoid internal_push_domain_ref_by_id(int32_t id);

    // Internal pop domain ref
    static RtResultVoid internal_pop_domain_ref();

    // Internal set context
    static RtResult<vm::RtObject*> internal_set_context(vm::RtObject* ctx);

    // Internal get context
    static RtResult<vm::RtAppContext*> internal_get_context();

    // Internal get default context
    static RtResult<vm::RtAppContext*> internal_get_default_context();

    // Internal get process GUID
    static RtResult<vm::RtString*> internal_get_process_guid(vm::RtObject* new_guid);

    // Create domain
    static RtResult<vm::RtObject*> create_domain(vm::RtObject* friendly_name, vm::RtObject* setup);

    // Internal is finalizing for unload
    static RtResult<bool> internal_is_finalizing_for_unload(int32_t id);

    // Internal unload
    static RtResultVoid internal_unload(int32_t id);

    // Do unhandled exception
    static RtResultVoid do_unhandled_exception(vm::RtObject* ex);
};

} // namespace icalls
} // namespace leanclr

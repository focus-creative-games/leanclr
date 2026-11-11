#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemAppDomain
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get setup
    static RtResult<vm::RtObject*> get_setup() noexcept;

    // Get friendly name
    static RtResult<vm::RtString*> get_friendly_name() noexcept;

    // Get current domain
    static RtResult<vm::RtAppDomain*> get_cur_domain() noexcept;

    // Get root domain
    static RtResult<vm::RtAppDomain*> get_root_domain() noexcept;

    // Execute assembly
    static RtResult<int32_t> execute_assembly(vm::RtObject* assembly, vm::RtObject* args) noexcept;

    // Get assemblies
    static RtResult<vm::RtArray*> get_assemblies(vm::RtAppDomain* this_domain, bool ref_only) noexcept;

    // Get data
    static RtResult<vm::RtObject*> get_data(vm::RtAppDomain* this_domain, vm::RtString* name) noexcept;

    // Set data
    static RtResultVoid set_data(vm::RtAppDomain* this_domain, vm::RtString* name, vm::RtObject* value) noexcept;

    // Load assembly
    static RtResult<vm::RtReflectionAssembly*> load_assembly(vm::RtAppDomain* this_domain, vm::RtString* name, vm::RtObject* evidence, bool ref_only,
                                                             vm::RtStackCrawlMark* stack_crawl_mark) noexcept;

    // Load assembly raw
    static RtResult<vm::RtReflectionAssembly*> load_assembly_raw(vm::RtAppDomain* this_domain, vm::RtArray* raw, vm::RtArray* symbols, vm::RtObject* evidence,
                                                                 bool ref_only) noexcept;

    // Internal set domain by ID
    static RtResult<vm::RtObject*> internal_set_domain_by_id(int32_t id) noexcept;

    // Internal set domain
    static RtResult<vm::RtObject*> internal_set_domain(vm::RtObject* domain) noexcept;

    // Internal push domain ref
    static RtResultVoid internal_push_domain_ref(vm::RtObject* domain) noexcept;

    // Internal push domain ref by ID
    static RtResultVoid internal_push_domain_ref_by_id(int32_t id) noexcept;

    // Internal pop domain ref
    static RtResultVoid internal_pop_domain_ref() noexcept;

    // Internal set context
    static RtResult<vm::RtObject*> internal_set_context(vm::RtObject* ctx) noexcept;

    // Internal get context
    static RtResult<vm::RtAppContext*> internal_get_context() noexcept;

    // Internal get default context
    static RtResult<vm::RtAppContext*> internal_get_default_context() noexcept;

    // Internal get process GUID
    static RtResult<vm::RtString*> internal_get_process_guid(vm::RtObject* new_guid) noexcept;

    // Create domain
    static RtResult<vm::RtObject*> create_domain(vm::RtObject* friendly_name, vm::RtObject* setup) noexcept;

    // Internal is finalizing for unload
    static RtResult<bool> internal_is_finalizing_for_unload(int32_t id) noexcept;

    // Internal unload
    static RtResultVoid internal_unload(int32_t id) noexcept;

    // Do unhandled exception
    static RtResultVoid do_unhandled_exception(vm::RtObject* ex) noexcept;
};

} // namespace icalls
} // namespace leanclr

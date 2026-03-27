#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionRuntimeAssembly
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // Get manifest resource internal
    static RtResult<intptr_t> get_manifest_resource_internal(vm::RtReflectionAssembly* ref_ass, vm::RtString* name, int32_t* size,
                                                             vm::RtReflectionModule** module);

    // Get code base
    static RtResult<vm::RtString*> get_code_base(vm::RtReflectionAssembly* ref_ass, bool escaped);

    // Get location
    static RtResult<vm::RtString*> get_location(vm::RtReflectionAssembly* ref_ass);

    // Get full name
    static RtResult<vm::RtString*> get_fullname(vm::RtReflectionAssembly* ref_ass);

    // Get AOT ID internal
    static RtResult<bool> get_aot_id_internal(vm::RtReflectionAssembly* ref_ass, vm::RtArray* buffer);

    // Internal image runtime version
    static RtResult<vm::RtString*> internal_image_runtime_version(vm::RtReflectionAssembly* ref_ass);

    // Get global assembly cache
    static RtResult<bool> get_global_assembly_cache(vm::RtReflectionAssembly* ref_ass);

    // Get entry point
    static RtResult<vm::RtReflectionMethod*> get_entry_point(vm::RtReflectionAssembly* ref_ass);

    // Get reflection only
    static RtResult<bool> get_reflection_only(vm::RtReflectionAssembly* ref_ass);

    // Get manifest resource info internal
    static RtResult<bool> get_manifest_resource_info_internal(vm::RtReflectionAssembly* ref_ass, vm::RtString* name, vm::RtObject* info);

    // Get manifest resource names
    static RtResult<vm::RtArray*> get_manifest_resource_names(vm::RtReflectionAssembly* ref_ass);

    // Get manifest module internal
    static RtResult<vm::RtReflectionModule*> get_manifest_module_internal(vm::RtReflectionAssembly* ref_ass);

    // Get modules internal
    static RtResult<vm::RtArray*> get_modules_internal(vm::RtReflectionAssembly* ref_ass);

    // Get files internal
    static RtResult<vm::RtObject*> get_files_internal(vm::RtReflectionAssembly* ref_ass, vm::RtString* path, bool get_resource_modules);
};

} // namespace icalls
} // namespace leanclr

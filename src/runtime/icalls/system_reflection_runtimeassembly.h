#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionRuntimeAssembly
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get manifest resource internal
    static RtResult<intptr_t> get_manifest_resource_internal(vm::RtReflectionAssembly* ref_ass, vm::RtString* name, int32_t* size,
                                                             vm::RtReflectionModule** module) noexcept;

    // Get code base
    static RtResult<vm::RtString*> get_code_base(vm::RtReflectionAssembly* ref_ass, bool escaped) noexcept;

    // Get location
    static RtResult<vm::RtString*> get_location(vm::RtReflectionAssembly* ref_ass) noexcept;

    // Get full name
    static RtResult<vm::RtString*> get_fullname(vm::RtReflectionAssembly* ref_ass) noexcept;

    // Get AOT ID internal
    static RtResult<bool> get_aot_id_internal(vm::RtReflectionAssembly* ref_ass, vm::RtArray* buffer) noexcept;

    // Internal image runtime version
    static RtResult<vm::RtString*> internal_image_runtime_version(vm::RtReflectionAssembly* ref_ass) noexcept;

    // Get global assembly cache
    static RtResult<bool> get_global_assembly_cache(vm::RtReflectionAssembly* ref_ass) noexcept;

    // Get entry point
    static RtResult<vm::RtReflectionMethod*> get_entry_point(vm::RtReflectionAssembly* ref_ass) noexcept;

    // Get reflection only
    static RtResult<bool> get_reflection_only(vm::RtReflectionAssembly* ref_ass) noexcept;

    // Get manifest resource info internal
    static RtResult<bool> get_manifest_resource_info_internal(vm::RtReflectionAssembly* ref_ass, vm::RtString* name, vm::RtObject* info) noexcept;

    // Get manifest resource names
    static RtResult<vm::RtArray*> get_manifest_resource_names(vm::RtReflectionAssembly* ref_ass) noexcept;

    // Get manifest module internal
    static RtResult<vm::RtReflectionModule*> get_manifest_module_internal(vm::RtReflectionAssembly* ref_ass) noexcept;

    // Get modules internal
    static RtResult<vm::RtArray*> get_modules_internal(vm::RtReflectionAssembly* ref_ass) noexcept;

    // Get files internal
    static RtResult<vm::RtObject*> get_files_internal(vm::RtReflectionAssembly* ref_ass, vm::RtString* path, bool get_resource_modules) noexcept;
};

} // namespace icalls
} // namespace leanclr

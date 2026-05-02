#pragma once

#include "vm/internal_calls.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionAssembly
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get the currently executing assembly
    static RtResult<vm::RtReflectionAssembly*> get_executing_assembly() noexcept;

    // Get the calling assembly
    static RtResult<vm::RtReflectionAssembly*> get_calling_assembly() noexcept;

    // Get type from assembly by name
    static RtResult<vm::RtReflectionType*> internal_get_type(vm::RtReflectionAssembly* assembly, vm::RtReflectionModule* module, vm::RtString* name,
                                                             bool throw_on_error, bool ignore_case) noexcept;

    // Get all types from assembly
    static RtResult<vm::RtArray*> get_types(vm::RtReflectionAssembly* ref_assembly, bool exported_only) noexcept;

    // Get the entry assembly
    static RtResult<vm::RtReflectionAssembly*> get_entry_assembly() noexcept;

    // Get assembly name from path (not implemented)
    static RtResultVoid internal_get_assembly_name(vm::RtString* path, metadata::RtMonoAssemblyName* aname, vm::RtString** codebase_out) noexcept;

    // Load assembly from path
    static RtResult<vm::RtReflectionAssembly*> load_from(vm::RtString* path, bool ref_only, int32_t* mark) noexcept;

    // Load assembly from file
    static RtResult<vm::RtReflectionAssembly*> load_file_internal(vm::RtString* path, int32_t* mark) noexcept;

    // Load assembly with partial name
    static RtResult<vm::RtReflectionAssembly*> load_with_partial_name(vm::RtString* name, vm::RtObject* evidence) noexcept;

    // Get referenced assemblies
    static RtResult<intptr_t> internal_get_referenced_assemblies(vm::RtReflectionAssembly* ref_ass) noexcept;
};

} // namespace icalls
} // namespace leanclr

#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionRuntimeModule
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get metadata token from module
    static RtResult<int32_t> get_metadata_token(vm::RtReflectionModule* module) noexcept;

    // Get MD stream version
    static RtResult<int32_t> get_md_stream_version(intptr_t module) noexcept;

    // Get internal types
    static RtResult<vm::RtArray*> internal_get_types(metadata::RtModuleDef* module) noexcept;

    // Get HINSTANCE for module
    static RtResult<intptr_t> get_hinstance(metadata::RtModuleDef* module) noexcept;

    // Get GUID internal
    static RtResultVoid get_guid_internal(metadata::RtModuleDef* module, vm::RtArray* guid_bytes) noexcept;

    // Get global type
    static RtResult<vm::RtReflectionType*> get_global_type(metadata::RtModuleDef* module) noexcept;

    // Resolve type token
    static RtResult<const metadata::RtTypeSig*> resolve_type_token(metadata::RtModuleDef* module, int32_t token, vm::RtArray* type_args,
                                                                   vm::RtArray* method_args, int32_t* error) noexcept;

    // Resolve method token
    static RtResult<const metadata::RtMethodInfo*> resolve_method_token(metadata::RtModuleDef* module, int32_t token, vm::RtArray* type_args,
                                                                        vm::RtArray* method_args, int32_t* error) noexcept;

    // Resolve field token
    static RtResult<const metadata::RtFieldInfo*> resolve_field_token(metadata::RtModuleDef* module, int32_t token, vm::RtArray* type_args,
                                                                      vm::RtArray* method_args, int32_t* error) noexcept;

    // Resolve string token
    static RtResult<vm::RtString*> resolve_string_token(metadata::RtModuleDef* module, int32_t token, int32_t* error) noexcept;

    // Resolve member token
    static RtResult<vm::RtObject*> resolve_member_token(metadata::RtModuleDef* module, int32_t token, vm::RtArray* type_args, vm::RtArray* method_args,
                                                        int32_t* error) noexcept;

    // Resolve signature
    static RtResult<vm::RtArray*> resolve_signature(metadata::RtModuleDef* module, int32_t token, int32_t* error) noexcept;
};

} // namespace icalls
} // namespace leanclr

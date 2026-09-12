#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

struct MetadataConstArray;

class SystemReflectionRuntimeModule
{
  public:
    static utils::Span<vm::InternalCallEntry> get_net10_internal_call_entries() noexcept;
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

RtResult<intptr_t> get_metadata_import(vm::RtReflectionModule* module) noexcept;
RtResult<int32_t> metadata_import_get_property_props(metadata::RtModuleDef* module, int32_t md_token, void** name,
                                                     int32_t* property_attributes, MetadataConstArray* signature) noexcept;
RtResult<int32_t> metadata_import_get_event_props(metadata::RtModuleDef* module, int32_t md_token, void** name,
                                                  int32_t* event_attributes) noexcept;
RtResult<int32_t> metadata_import_get_field_def_props(metadata::RtModuleDef* module, int32_t md_token, int32_t* field_attributes) noexcept;
RtResult<int32_t> metadata_import_get_param_def_props(metadata::RtModuleDef* module, int32_t md_token, int32_t* sequence,
                                                      int32_t* attributes) noexcept;
RtResult<int32_t> metadata_import_get_field_marshal(metadata::RtModuleDef* module, int32_t md_token,
                                                    MetadataConstArray* field_marshal) noexcept;
RtResult<int32_t> metadata_import_get_field_offset(metadata::RtModuleDef* module, int32_t type_token_value,
                                                   int32_t field_token_value, int32_t* offset, bool* found) noexcept;
RtResult<int32_t> metadata_import_get_class_layout(metadata::RtModuleDef* module, int32_t type_token_value,
                                                   int32_t* pack_size, int32_t* class_size) noexcept;
RtResult<int32_t> metadata_import_get_default_value(metadata::RtModuleDef* module, int32_t md_token, int64_t* value,
                                                    const uint16_t** string_metadata_encoding, int32_t* length,
                                                    int32_t* cor_element_type) noexcept;
RtResult<int32_t> metadata_import_get_user_string(metadata::RtModuleDef* module, int32_t md_token,
                                                  const uint16_t** string_metadata_encoding, int32_t* length) noexcept;
RtResult<int32_t> metadata_import_get_scope_props(metadata::RtModuleDef* module, uint8_t* mvid) noexcept;
RtResult<int32_t> metadata_import_get_signature_from_token(metadata::RtModuleDef* module, int32_t md_token,
                                                           MetadataConstArray* signature) noexcept;
RtResult<int32_t> metadata_import_get_member_ref_props(metadata::RtModuleDef* module, int32_t md_token,
                                                       MetadataConstArray* signature) noexcept;
RtResult<int32_t> metadata_import_get_sig_of_method_def(metadata::RtModuleDef* module, int32_t md_token,
                                                        MetadataConstArray* signature) noexcept;
RtResult<int32_t> metadata_import_get_sig_of_field_def(metadata::RtModuleDef* module, int32_t md_token,
                                                       MetadataConstArray* signature) noexcept;
RtResult<int32_t> metadata_import_get_custom_attribute_props(metadata::RtModuleDef* module, int32_t md_token,
                                                             int32_t* constructor_token, MetadataConstArray* signature) noexcept;
RtResult<int32_t> metadata_import_get_parent_token(metadata::RtModuleDef* module, int32_t md_token, int32_t* parent_token) noexcept;
RtResult<int32_t> metadata_import_get_name(metadata::RtModuleDef* module, int32_t md_token, void** name) noexcept;
RtResult<int32_t> metadata_import_get_namespace(metadata::RtModuleDef* module, int32_t md_token, void** namespaze) noexcept;
RtResult<bool> metadata_import_is_valid_token(metadata::RtModuleDef* module, int32_t token_value) noexcept;

} // namespace icalls
} // namespace leanclr

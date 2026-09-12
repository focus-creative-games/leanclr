#pragma once

#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{
class Reflection
{
  public:
    static RtResult<RtReflectionType*> get_type_reflection_object(const metadata::RtTypeSig* type_sig);
    static RtResult<RtReflectionRuntimeType*> get_runtime_type_from_type_sig(const metadata::RtTypeSig* type_sig);
    static RtResult<RtReflectionType*> get_klass_reflection_object(const metadata::RtClass* klass);
    static RtResult<RtObject*> get_or_create_runtime_type_cache(RtReflectionRuntimeType* runtime_type);
    static bool is_coreclr_reflection_object_class(const metadata::RtClass* klass);
    static RtResult<RtObject*> normalize_coreclr_reflection_object(RtObject* obj);
    static RtResult<const metadata::RtTypeSig*> get_net10_type_handle(const metadata::RtTypeSig* type_sig);
    static RtResult<const void*> get_net10_method_table(const metadata::RtTypeSig* type_sig);
    static RtResult<const metadata::RtTypeSig*> get_type_sig_from_net10_method_table(const void* method_table);
    static RtResult<const metadata::RtTypeSig*> get_type_sig_from_net10_type_handle(const void* type_handle);
    static RtResult<const metadata::RtClass*> get_class_from_net10_method_table(const void* method_table);
    static RtResult<RtReflectionRuntimeType*> get_runtime_type_from_handle_arg(const void* type_handle);
    static RtResult<const metadata::RtTypeSig*> get_type_sig_from_runtime_type_handle_arg(const void* type_handle);
    static RtResult<const metadata::RtTypeSig*> get_type_sig_from_qcall_type_handle(void* qcall_type_handle, void* native_handle);
    static RtResult<const metadata::RtTypeSig*> get_type_sig_from_reflection_type_object(const RtReflectionType* type_obj);
    static RtResult<metadata::RtClass*> get_class_from_reflection_type_object(const RtReflectionType* type_obj);
    static RtResult<const metadata::RtTypeSig*> get_type_sig_from_runtime_type_object(const RtReflectionRuntimeType* runtime_type);
    static RtResult<metadata::RtClass*> get_class_from_runtime_type_object(const RtReflectionRuntimeType* runtime_type);
    static RtResult<RtReflectionMethod*> get_method_reflection_object(const metadata::RtMethodInfo* method, const metadata::RtClass* reflection_at_klass);
    static RtResult<RtReflectionMethod*> create_runtime_method_info_object(const metadata::RtMethodInfo* method,
                                                                           RtReflectionRuntimeType* declaring_type,
                                                                           RtObject* reflected_type_cache,
                                                                           int32_t method_attributes,
                                                                           int32_t binding_flags,
                                                                           RtObject* keepalive);
    static RtResult<RtObject*> create_runtime_method_info_stub(const metadata::RtMethodInfo* method, RtObject* keepalive);
    static RtResult<RtReflectionConstructor*> create_runtime_constructor_info_object(const metadata::RtMethodInfo* method,
                                                                                    RtReflectionRuntimeType* declaring_type,
                                                                                    RtObject* reflected_type_cache,
                                                                                    int32_t method_attributes,
                                                                                    int32_t binding_flags);
    static RtResult<const metadata::RtMethodInfo*> get_method_info_from_reflection_object(RtReflectionMethod* method_obj);
    static RtResult<const metadata::RtMethodInfo*> get_method_info_from_handle_arg(const void* method_arg);
    static RtResult<const metadata::RtClass*> get_reflection_method_klass(RtReflectionMethod* method_obj);
    static RtResult<RtArray*> get_param_objects(const metadata::RtMethodInfo* method, const metadata::RtClass* reflection_at_klass);
    static RtResult<const metadata::RtMethodInfo*> get_parameter_method_info_from_reflection_object(RtReflectionParameter* parameter_obj);
    static RtResult<std::optional<uint32_t>> get_parameter_token_from_reflection_object(RtReflectionParameter* parameter_obj);
    static RtResult<RtReflectionField*> get_field_reflection_object(const metadata::RtFieldInfo* field, const metadata::RtClass* reflection_at_klass);
    static RtResult<const metadata::RtFieldInfo*> get_field_info_from_reflection_object(RtReflectionField* field_obj);
    static RtResult<const metadata::RtFieldInfo*> get_field_info_from_handle_arg(const void* field_arg);
    static RtResult<RtObject*> create_runtime_field_info_object(const metadata::RtFieldInfo* field,
                                                                RtReflectionRuntimeType* declaring_type,
                                                                RtObject* reflected_type_cache,
                                                                int32_t binding_flags);
    static RtResult<RtObject*> create_runtime_field_info_stub(const metadata::RtFieldInfo* field);
    static RtResult<const metadata::RtClass*> get_reflection_field_klass(RtReflectionField* field_obj);
    static RtResult<RtReflectionProperty*> get_property_reflection_object(const metadata::RtPropertyInfo* prop, const metadata::RtClass* reflection_at_klass);
    static RtResult<const metadata::RtPropertyInfo*> get_property_info_from_runtime_type(RtReflectionRuntimeType* declaring_type,
                                                                                         int32_t property_token);
    static RtResult<RtObject*> create_runtime_property_info_object(const metadata::RtPropertyInfo* property,
                                                                   RtReflectionRuntimeType* declaring_type,
                                                                   RtObject* reflected_type_cache,
                                                                   bool* is_private);
    static RtResult<const metadata::RtPropertyInfo*> get_property_info_from_reflection_object(RtReflectionProperty* property_obj);
    static RtResult<const metadata::RtPropertyInfo*> get_property_info_from_handle_arg(const void* property_arg);
    static RtResult<const metadata::RtClass*> get_reflection_property_klass(RtReflectionProperty* property_obj);
    static RtResult<RtReflectionEventInfo*> get_event_reflection_object(metadata::RtEventInfo* event_info, const metadata::RtClass* reflection_at_klass);
    static RtResult<metadata::RtEventInfo*> get_event_info_from_runtime_type(RtReflectionRuntimeType* declaring_type,
                                                                             int32_t event_token);
    static RtResult<metadata::RtEventInfo*> get_event_info_from_reflection_object(RtReflectionEventInfo* event_obj);
    static RtResult<metadata::RtEventInfo*> get_event_info_from_handle_arg(const void* event_arg);
    static RtResult<const metadata::RtClass*> get_reflection_event_klass(RtReflectionEventInfo* event_obj);
    static RtResult<RtReflectionAssembly*> get_assembly_reflection_object(metadata::RtAssembly* assembly);
    static RtResult<metadata::RtAssembly*> get_assembly_from_handle_arg(const void* assembly_arg);
    static RtResult<metadata::RtAssembly*> get_assembly_from_qcall_assembly(void* qcall_assembly, void* native_handle);
    static RtResult<metadata::RtAssembly*> get_assembly_from_reflection_object(RtReflectionAssembly* assembly_obj);
    static RtResult<metadata::RtMonoAssemblyName*> get_assembly_name_object(metadata::RtAssembly* ass);
    static RtResult<RtObject*> create_runtime_assembly_name_object(const metadata::RtAssemblyName& assembly_name);
    static RtResult<RtReflectionModule*> get_module_reflection_object(metadata::RtModuleDef* mod);
    static RtResult<metadata::RtModuleDef*> get_module_from_handle_arg(const void* module_arg);
    static RtResult<metadata::RtModuleDef*> get_module_from_qcall_module(void* qcall_module, void* native_handle);
    static RtResult<metadata::RtModuleDef*> get_module_from_reflection_object(RtReflectionModule* module_obj);
    static RtResult<RtObject*> invoke_method(const metadata::RtMethodInfo* method, RtObject* obj, RtArray* params, RtObject** out_ex);
};
} // namespace vm
} // namespace leanclr

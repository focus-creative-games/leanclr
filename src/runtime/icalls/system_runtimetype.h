#pragma once

#include "icall_base.h"
#include "utils/safegptrarray.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeType
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // Type construction
    static RtResult<vm::RtReflectionType*> make_array_type(vm::RtReflectionRuntimeType* runtime_type, int32_t rank);
    static RtResult<vm::RtReflectionType*> make_byref_type(vm::RtReflectionRuntimeType* runtime_type);
    static RtResult<vm::RtReflectionType*> make_pointer_type(vm::RtReflectionRuntimeType* runtime_type);
    static RtResult<vm::RtReflectionType*> make_generic_type(vm::RtReflectionRuntimeType* generic_base_type, vm::RtArray* generic_args);

    // Member enumeration - using SafeGPtrArray templates
    static RtResult<utils::SafeGPtrArray<metadata::RtMethodInfo>*> get_methods_by_name_native(vm::RtReflectionRuntimeType* runtime_type, const char* name,
                                                                                              int32_t bind_flags, int32_t list_type);
    static RtResult<utils::SafeGPtrArray<metadata::RtPropertyInfo>*> get_properties_by_name_native(vm::RtReflectionRuntimeType* runtime_type, const char* name,
                                                                                                   int32_t bind_flags, int32_t list_type);
    static RtResult<utils::SafeGPtrArray<metadata::RtMethodInfo>*> get_constructors_native(vm::RtReflectionRuntimeType* runtime_type, int32_t bind_flags);
    static RtResult<utils::SafeGPtrArray<metadata::RtEventInfo>*> get_events_native(vm::RtReflectionRuntimeType* runtime_type, const char* name,
                                                                                    int32_t list_type);
    static RtResult<utils::SafeGPtrArray<metadata::RtFieldInfo>*> get_fields_native(vm::RtReflectionRuntimeType* runtime_type, const char* name,
                                                                                    int32_t bind_flags, int32_t list_type);
    static RtResult<utils::SafeGPtrArray<metadata::RtClass>*> get_nested_types_native(vm::RtReflectionRuntimeType* runtime_type, const char* name,
                                                                                      int32_t bind_flags);

    // Reflection helpers
    static RtResultVoid get_interface_map_data(vm::RtReflectionRuntimeType* runtime_type, vm::RtReflectionRuntimeType* interface_type, vm::RtArray** targets,
                                               vm::RtArray** methods);
    static RtResultVoid get_guid(vm::RtReflectionRuntimeType* runtime_type, vm::RtArray* guid);
    static RtResultVoid get_packing(vm::RtReflectionRuntimeType* runtime_type, int32_t* packing, int32_t* size);
    static RtResult<int32_t> get_type_code_impl_internal(vm::RtReflectionRuntimeType* runtime_type);
    static RtResult<vm::RtObject*> create_instance_internal(vm::RtReflectionRuntimeType* runtime_type);
    static RtResult<vm::RtReflectionMethod*> get_declaring_method(vm::RtReflectionRuntimeType* runtime_type);
    static RtResult<vm::RtString*> get_full_name(vm::RtReflectionRuntimeType* runtime_type, bool full_name, bool assembly_qualified);
    static RtResult<vm::RtArray*> get_generic_arguments_internal(vm::RtReflectionRuntimeType* runtime_type, bool runtime_array);
    static RtResult<int32_t> get_generic_parameter_position(vm::RtReflectionRuntimeType* runtime_type);
    static RtResult<vm::RtReflectionType*> get_declaring_type(vm::RtReflectionRuntimeType* runtime_type);
    static RtResult<vm::RtString*> get_name(vm::RtReflectionRuntimeType* runtime_type);
    static RtResult<vm::RtString*> get_namespace(vm::RtReflectionRuntimeType* runtime_type);
    static RtResult<vm::RtArray*> get_interfaces(vm::RtReflectionRuntimeType* runtime_type);
};
} // namespace icalls
} // namespace leanclr

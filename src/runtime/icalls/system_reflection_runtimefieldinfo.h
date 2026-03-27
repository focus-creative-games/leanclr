#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionRuntimeFieldInfo
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // Get metadata token of the field
    static RtResult<uint32_t> get_metadata_token(vm::RtReflectionField* field);

    // Get field offset
    static RtResult<int32_t> get_field_offset(vm::RtReflectionField* field);

    // Get raw constant value
    static RtResult<vm::RtObject*> get_raw_const_value(vm::RtReflectionField* field);

    // Get value internal
    static RtResult<vm::RtObject*> get_value_internal(vm::RtReflectionField* field, vm::RtObject* obj);

    // Unsafe get value
    static RtResult<vm::RtObject*> unsafe_get_value(vm::RtReflectionField* field, vm::RtObject* obj);

    // Set value internal
    static RtResultVoid set_value_internal(vm::RtReflectionField* field, vm::RtObject* obj, vm::RtObject* value);

    // Get parent type
    static RtResult<vm::RtReflectionType*> get_parent_type(vm::RtReflectionField* field, bool declaring);

    // Resolve type
    static RtResult<vm::RtReflectionType*> resolve_type(vm::RtReflectionField* field);

    // Get type modifiers
    static RtResult<vm::RtArray*> get_type_modifiers(vm::RtReflectionField* field, bool optional);
};

} // namespace icalls
} // namespace leanclr

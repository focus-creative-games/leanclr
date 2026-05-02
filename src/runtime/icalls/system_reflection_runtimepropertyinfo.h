#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionRuntimePropertyInfo
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get property reflection object from property handle and optional type
    static RtResult<vm::RtReflectionProperty*> internal_from_handle_type(metadata::RtPropertyInfo* property, const metadata::RtTypeSig* type_sig) noexcept;

    // Get property info
    static RtResultVoid get_property_info(vm::RtReflectionProperty* property, vm::RtMonoPropertyInfo* result_info, int32_t pinfo) noexcept;

    // Get type modifiers
    static RtResult<vm::RtArray*> get_type_modifiers(vm::RtReflectionProperty* property, bool optional) noexcept;

    // Get default value
    static RtResult<vm::RtObject*> get_default_value(vm::RtReflectionProperty* property) noexcept;

    // Get metadata token
    static RtResult<int32_t> get_metadata_token(vm::RtReflectionProperty* property) noexcept;
};

} // namespace icalls
} // namespace leanclr

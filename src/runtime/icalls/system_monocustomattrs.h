#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemMonoCustomAttrs
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // Check if custom attribute is defined on target object
    static RtResult<bool> is_defined_internal(vm::RtObject* obj, vm::RtReflectionType* attribute_type);

    // Get custom attributes on target object
    static RtResult<vm::RtArray*> get_custom_attributes_internal(vm::RtObject* obj, vm::RtReflectionType* attribute_type, bool pseudo_attrs);

    // Get custom attributes data on target
    static RtResult<vm::RtArray*> get_custom_attributes_data_internal(vm::RtObject* obj);
};

} // namespace icalls
} // namespace leanclr

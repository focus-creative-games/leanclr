#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{
class SystemRuntimeCompilerServicesRuntimeHelpers
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    static RtResultVoid initialize_array(vm::RtArray* arr, size_t runtime_field_handle);
    static RtResult<int32_t> get_offset_to_string_data();
    static RtResult<vm::RtObject*> get_object_value(vm::RtObject* obj);
    static RtResultVoid run_class_constructor(intptr_t type_handle);
    static RtResult<bool> sufficient_execution_stack();
    static RtResultVoid run_module_constructor(intptr_t module_handle);
};
} // namespace icalls
} // namespace leanclr

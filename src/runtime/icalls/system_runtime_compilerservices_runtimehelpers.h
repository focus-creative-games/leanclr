#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{
class SystemRuntimeCompilerServicesRuntimeHelpers
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResultVoid initialize_array(vm::RtArray* arr, size_t runtime_field_handle) noexcept;
    static RtResult<int32_t> get_offset_to_string_data() noexcept;
    static RtResult<vm::RtObject*> get_object_value(vm::RtObject* obj) noexcept;
    static RtResultVoid run_class_constructor(intptr_t type_handle) noexcept;
    static RtResult<bool> sufficient_execution_stack() noexcept;
    static RtResultVoid run_module_constructor(intptr_t module_handle) noexcept;
};
} // namespace icalls
} // namespace leanclr

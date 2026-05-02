#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionRuntimeParameterInfo
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<int32_t> get_metadata_token(const vm::RtReflectionParameter* param) noexcept;
    static RtResult<vm::RtArray*> get_type_modifiers(vm::RtReflectionType* parameter_type, vm::RtObject* member, int32_t index, bool optional) noexcept;
};

} // namespace icalls
} // namespace leanclr

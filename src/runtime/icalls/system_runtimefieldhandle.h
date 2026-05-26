#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeFieldHandle
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<vm::RtObject*> get_value_direct(vm::RtReflectionField* field, vm::RtReflectionRuntimeType* field_type, vm::RtTypedReference* typed_ref,
                                                     vm::RtReflectionRuntimeType* context_type) noexcept;
    static RtResultVoid set_value_direct(vm::RtReflectionField* field, vm::RtReflectionRuntimeType* field_type, vm::RtTypedReference* typed_ref, vm::RtObject* value,
                                         vm::RtReflectionRuntimeType* context_type) noexcept;
    static RtResultVoid set_value_internal(vm::RtReflectionField* field, vm::RtObject* obj, vm::RtObject* value) noexcept;
};

} // namespace icalls
} // namespace leanclr

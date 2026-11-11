#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{
class SystemTypedReference
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // TypedReference operations
    static RtResultVoid internal_make_typed_reference(vm::RtTypedReference* result, vm::RtObject* target, vm::RtArray* fields,
                                                      vm::RtReflectionType* last_field_type) noexcept;
    static RtResult<vm::RtObject*> internal_to_object(const vm::RtTypedReference* typed_ref) noexcept;
};

} // namespace icalls
} // namespace leanclr

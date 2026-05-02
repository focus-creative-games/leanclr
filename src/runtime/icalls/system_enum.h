#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemEnum
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Compare two enum objects
    static RtResult<int32_t> internal_compare_to(vm::RtObject* obj1, vm::RtObject* obj2) noexcept;

    // Get underlying type of an enum
    static RtResult<vm::RtReflectionRuntimeType*> internal_get_underlying_type(vm::RtReflectionRuntimeType* enum_klass) noexcept;

    // Get enum values and names
    static RtResult<bool> get_enum_values_and_names(vm::RtReflectionRuntimeType* enum_klass, vm::RtArray** values, vm::RtArray** names) noexcept;

    // Box enum value
    static RtResult<vm::RtObject*> internal_box_enum(vm::RtReflectionRuntimeType* runtime_type, uint64_t value) noexcept;

    // Get boxed value from enum object
    static RtResult<vm::RtObject*> get_value(vm::RtObject* obj) noexcept;

    // Check if enum has flag
    static RtResult<bool> internal_has_flag(vm::RtObject* obj, vm::RtObject* flag) noexcept;

    // Get hash code of enum
    static RtResult<int32_t> get_hash_code(vm::RtObject* obj) noexcept;
};

} // namespace icalls
} // namespace leanclr

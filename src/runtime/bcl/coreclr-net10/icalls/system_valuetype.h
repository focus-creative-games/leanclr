#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemValueType
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // System.ValueType icalls
    static RtResult<bool> internal_equals(vm::RtObject* obj1, vm::RtObject* obj2, vm::RtArray** uncompared_field_objs) noexcept;
    static RtResult<int32_t> internal_get_hash_code(vm::RtObject* obj, vm::RtArray** uncompared_field_objs) noexcept;
};

} // namespace icalls
} // namespace leanclr

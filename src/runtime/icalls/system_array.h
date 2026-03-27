#pragma once

#include "vm/internal_calls.h"

namespace leanclr
{
namespace icalls
{

class SystemArray
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // System.Array icalls
    static RtResult<int32_t> get_rank(vm::RtArray* arr);
    static RtResult<int32_t> get_length(vm::RtArray* arr, int32_t dimension);
    static RtResult<int32_t> get_lower_bound(vm::RtArray* arr, int32_t dimension);
    static RtResult<vm::RtObject*> get_value(vm::RtArray* arr, vm::RtArray* indices);
    static RtResultVoid set_value(vm::RtArray* arr, vm::RtObject* value, vm::RtArray* indices);
    static RtResult<vm::RtObject*> get_value_impl(vm::RtArray* arr, int32_t global_index);
    static RtResultVoid set_value_impl(vm::RtArray* arr, vm::RtObject* value, int32_t global_index);
    static RtResult<bool> fast_copy(vm::RtArray* src, int32_t src_index, vm::RtArray* dst, int32_t dst_index, int32_t length);
    static RtResult<vm::RtArray*> create_instance_impl(vm::RtReflectionType* ele_ref_type, vm::RtArray* lengths, vm::RtArray* lower_bounds);
    static RtResultVoid clear_internal(vm::RtArray* arr, int32_t index, int32_t length);
};

} // namespace icalls
} // namespace leanclr

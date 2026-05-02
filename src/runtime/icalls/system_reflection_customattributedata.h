#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionCustomAttributeData
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResultVoid resolve_arguments_internal(vm::RtReflectionMethod* ctor, vm::RtReflectionAssembly* ctor_assembly, intptr_t data, uint32_t data_length,
                                                   vm::RtArray** typed_arg_arr_ptr, vm::RtArray** named_arg_arr_ptr) noexcept;
};

} // namespace icalls
} // namespace leanclr

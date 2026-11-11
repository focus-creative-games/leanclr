#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionRuntimeConstructorInfo
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get metadata token of constructor
    static RtResult<uint32_t> get_metadata_token(vm::RtReflectionConstructor* constructor) noexcept;

    // Invoke constructor
    static RtResult<vm::RtObject*> internal_invoke(vm::RtReflectionConstructor* constructor, vm::RtObject* obj, vm::RtArray* parameters,
                                                   vm::RtObject** out_exc) noexcept;
};

} // namespace icalls
} // namespace leanclr

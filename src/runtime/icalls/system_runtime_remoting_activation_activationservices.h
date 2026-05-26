#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeRemotingActivationActivationServices
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<vm::RtObject*> allocate_uninitialized_class_instance(vm::RtReflectionRuntimeType* type) noexcept;
};

} // namespace icalls
} // namespace leanclr

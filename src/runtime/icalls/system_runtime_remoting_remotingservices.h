#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeRemotingRemotingServices
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<bool> is_transparent_proxy(vm::RtObject* proxy) noexcept;
};

} // namespace icalls
} // namespace leanclr

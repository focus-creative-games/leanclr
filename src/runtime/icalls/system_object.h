#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemObject
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // System.Object icalls
    static RtResult<int32_t> get_hash_code(vm::RtObject* obj) noexcept;
    static RtResult<vm::RtReflectionType*> get_type(vm::RtObject* obj) noexcept;
    static RtResult<vm::RtObject*> memberwise_clone(vm::RtObject* obj) noexcept;
};

} // namespace icalls
} // namespace leanclr

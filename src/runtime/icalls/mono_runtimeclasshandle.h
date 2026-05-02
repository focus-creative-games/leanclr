#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class MonoRuntimeClassHandle
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get type signature from class
    static RtResult<const metadata::RtTypeSig*> get_type_from_class(metadata::RtClass* klass) noexcept;
};

} // namespace icalls
} // namespace leanclr

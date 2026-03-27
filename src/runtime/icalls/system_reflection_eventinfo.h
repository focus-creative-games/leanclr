#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionEventInfo
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    static RtResult<vm::RtReflectionEventInfo*> internal_from_handle_type(metadata::RtEventInfo* event, const metadata::RtTypeSig* type_sig);
};

} // namespace icalls
} // namespace leanclr

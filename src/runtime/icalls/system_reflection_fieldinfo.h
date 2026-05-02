#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionFieldInfo
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get field reflection object from field handle and optional type
    static RtResult<vm::RtReflectionField*> internal_from_handle_type(metadata::RtFieldInfo* field, const metadata::RtTypeSig* type_sig) noexcept;

    // Get marshal info for field (not implemented)
    static RtResult<vm::RtCustomAttribute*> get_marshal_info(vm::RtReflectionField* field) noexcept;
};

} // namespace icalls
} // namespace leanclr

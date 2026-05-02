#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemIOPath
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // System.IO.Path icalls
    static RtResult<vm::RtString*> get_temp_path() noexcept;
};

} // namespace icalls
} // namespace leanclr

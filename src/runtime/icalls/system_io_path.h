#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemIOPath
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // System.IO.Path icalls
    static RtResult<vm::RtString*> get_temp_path();
};

} // namespace icalls
} // namespace leanclr

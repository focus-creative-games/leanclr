#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemException
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // Report an unhandled exception
    static RtResultVoid report_unhandled_exception(vm::RtException* exception);
};

} // namespace icalls
} // namespace leanclr

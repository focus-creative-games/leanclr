#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{
class Interop
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    static RtResult<int32_t> double_to_string(double value, const char* format, char* buffer, int32_t buffer_size);
};
} // namespace icalls
} // namespace leanclr

#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemDiagnosticsStackFrame
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get frame information
    static RtResult<bool> get_frame_info(int32_t skip, bool need_file_info, vm::RtReflectionMethod** method, int32_t* il_offset, int32_t* native_offset,
                                         vm::RtString** file_name, int32_t* line_number, int32_t* column_number) noexcept;
};

} // namespace icalls
} // namespace leanclr

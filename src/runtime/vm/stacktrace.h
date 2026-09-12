#pragma once

#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{
class StackTrace
{
  public:
    // Stack trace related functions can be declared here
    static RtResultVoid setup_trace_ips(RtException* ex);

    static RtResultVoid set_stack_frame_data(RtObject* stack_frame, const metadata::RtMethodInfo* method, int32_t native_offset,
                                             int32_t il_offset, RtString* file_name, int32_t line_number, int32_t column_number,
                                             bool is_last_frame_from_foreign_exception_stack_trace);

    static RtResultVoid get_stack_frame_data(RtObject* stack_frame, RtReflectionMethod** method, int32_t* native_offset, int32_t* il_offset,
                                             RtString** file_name, int32_t* line_number, int32_t* column_number,
                                             bool* is_last_frame_from_foreign_exception_stack_trace);

    static RtResult<bool> get_frame_info(int32_t skip, bool need_file_info, RtReflectionMethod** method, int32_t* il_offset, int32_t* native_offset,
                                         RtString** file_name, int32_t* line_number, int32_t* column_number);

    static RtResult<RtArray*> get_stack_trace(RtException* ex, int32_t skip_frames, bool need_file_info);
};
} // namespace vm
} // namespace leanclr

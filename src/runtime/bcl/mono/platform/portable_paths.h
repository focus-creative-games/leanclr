#pragma once
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace os
{
namespace portable_io
{
vm::RtString* path_get_temp_path();
vm::RtString* path_get_current_directory(int32_t* error);
} // namespace portable_io
} // namespace os
} // namespace leanclr

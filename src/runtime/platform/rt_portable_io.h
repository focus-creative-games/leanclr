#pragma once

#include "core/rt_base.h"
#include "platform/rt_file.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace os
{
namespace portable_io
{

intptr_t file_get_stdin();
intptr_t file_get_stdout();
intptr_t file_get_stderr();
bool file_is_standard_handle(intptr_t handle);

intptr_t file_open(const Utf16Char* filename, int32_t mode, int32_t access, int32_t share, int32_t options, int32_t* error);
bool file_close(intptr_t handle, int32_t* error);
int32_t file_read(intptr_t handle, uint8_t* buffer, int32_t count, int32_t* error);
int32_t file_write(intptr_t handle, const uint8_t* buffer, int32_t count, int32_t* error);
int64_t file_seek(intptr_t handle, int64_t offset, int32_t origin, int32_t* error);
int64_t file_get_length(intptr_t handle, int32_t* error);
int32_t file_get_file_type(intptr_t handle, int32_t* error);

vm::RtString* path_get_temp_path();
vm::RtString* path_get_current_directory(int32_t* error);

} // namespace portable_io
} // namespace os
} // namespace leanclr

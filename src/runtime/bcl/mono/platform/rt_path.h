#pragma once

#include "core/rt_base.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace os
{

class Path
{
  public:
    // Directory separator characters (platform-specific)
    static Utf16Char get_directory_separator_char();
    static Utf16Char get_alt_directory_separator_char();
    static Utf16Char get_path_separator();
    static Utf16Char get_volume_separator_char();

    // System temporary directory. Returns an empty string only if the platform exposes no temp path.
    static vm::RtString* get_temp_path();

    // Current working directory. On failure, sets *error (MonoIOError) and returns the empty string.
    static vm::RtString* get_current_directory(int32_t* error);
};

} // namespace os
} // namespace leanclr

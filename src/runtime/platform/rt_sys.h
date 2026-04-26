#pragma once

#include "core/rt_base.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace platform
{
class RtSys
{
  public:
    static int32_t ch_mod(vm::RtString* path, int32_t mode);
    static int32_t mk_dir(vm::RtString* path, int32_t mode);
    static int32_t rename(vm::RtString* old_path, vm::RtString* new_path);
    static int32_t rm_dir(vm::RtString* path);
    static int32_t unlink(vm::RtString* path);
};
} // namespace platform
} // namespace leanclr

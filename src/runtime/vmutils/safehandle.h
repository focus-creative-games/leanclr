#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace vmutils
{
class SafeHandle
{
  public:
    static intptr_t get_handle(vm::RtObject* o) noexcept;
    static void set_handle(vm::RtObject* o, intptr_t handle) noexcept;
    static RtResult<vm::RtObject*> new_safe_handle(const metadata::RtClass* klass, intptr_t handle) noexcept;
};
} // namespace vmutils
} // namespace leanclr
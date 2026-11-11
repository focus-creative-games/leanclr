#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace os
{
class Architecture
{
  public:
    static vm::RtString* get_architecture_name();
    static vm::RtString* get_os_name();
};
} // namespace os
} // namespace leanclr

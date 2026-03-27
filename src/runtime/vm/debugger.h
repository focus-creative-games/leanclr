#pragma once

#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{

class Debugger
{
  public:
    static void log(int32_t level, RtString* category, RtString* message);
};

} // namespace vm
} // namespace leanclr

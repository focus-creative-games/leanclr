#pragma once

#include "core/rt_base.h"

namespace leanclr
{
namespace platform
{
class Bcrypt
{
  public:
    static void gen_random(intptr_t algo_handle, uint8_t* buffer, int32_t length, int32_t flags);
};
} // namespace platform
} // namespace leanclr
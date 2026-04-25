#pragma once

#include "core/rt_base.h"

namespace leanclr
{
namespace pal
{
class Hardware
{
  public:
    static constexpr bool is_hardware_accelerated()
    {
        return false;
    }
};
} // namespace pal
} // namespace leanclr

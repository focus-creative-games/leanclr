#pragma once

#include "build_config.h"

namespace leanclr
{
namespace utils
{
class Platform
{
  public:
    static bool is_little_endian()
    {
        uint16_t test = 0x0001;
        return *reinterpret_cast<uint8_t*>(&test) == 0x01;
    }

    template <typename T>
    static T select_arch(T v32, T v64)
    {
#if LEANCLR_ARCH_64BIT
        return v64;
#else
        return v32;
#endif
    }
};
} // namespace utils
} // namespace leanclr

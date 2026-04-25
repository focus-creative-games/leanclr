#pragma once

#include <cstdint>

#include "core/rt_base.h"

namespace leanclr
{
namespace misc
{
class RngCryptoServiceProvider
{
  public:
    static bool open();
    static intptr_t initialize(const uint8_t* seed, intptr_t seed_len);
    static intptr_t get_bytes(intptr_t handle, uint8_t* buffer, intptr_t length);
    static void close(intptr_t handle);
};
} // namespace misc
} // namespace leanclr

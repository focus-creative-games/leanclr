#pragma once

namespace leanclr
{
namespace utils
{
class HashUtil
{
  public:
    static size_t combine_hash(size_t value1, size_t value2)
    {
        return value1 ^ (value2 + 0x9e3779b9 + (value1 << 6) + (value1 >> 2));
    }
};

} // namespace utils
} // namespace leanclr

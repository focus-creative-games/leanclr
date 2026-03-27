#pragma once

#include <unordered_set>

namespace leanclr
{
namespace utils
{
template <typename K, class _Hasher = std::hash<K>, class _Keyeq = std::equal_to<K>>
using HashSet = std::unordered_set<K, _Hasher, _Keyeq>;
} // namespace utils
} // namespace leanclr

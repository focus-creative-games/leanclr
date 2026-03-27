#pragma once

#include <unordered_map>

namespace leanclr
{
namespace utils
{
template <typename K, typename V, class _Hasher = std::hash<K>, class _Keyeq = std::equal_to<K>>
using HashMap = std::unordered_map<K, V, _Hasher, _Keyeq>;
} // namespace utils
} // namespace leanclr

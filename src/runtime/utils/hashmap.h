#pragma once

#include <unordered_map>
#include "alloc/stl_allocator.h"

namespace leanclr
{
namespace utils
{
template <typename K, typename V, class _Hasher = std::hash<K>, class _Keyeq = std::equal_to<K>>
using HashMap = std::unordered_map<K, V, _Hasher, _Keyeq, alloc::StlAllocator<std::pair<const K, V>>>;
} // namespace utils
} // namespace leanclr

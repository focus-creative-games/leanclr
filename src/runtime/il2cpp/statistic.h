#pragma once

#include "rt_base.h"

namespace leanclr
{
namespace il2cpp
{
typedef enum
{
    IL2CPP_STAT_NEW_OBJECT_COUNT,
    IL2CPP_STAT_INITIALIZED_CLASS_COUNT,
    IL2CPP_STAT_METHOD_COUNT,
    IL2CPP_STAT_CLASS_STATIC_DATA_SIZE,
    IL2CPP_STAT_GENERIC_INSTANCE_COUNT,
    IL2CPP_STAT_GENERIC_CLASS_COUNT,
    IL2CPP_STAT_INFLATED_METHOD_COUNT,
    IL2CPP_STAT_INFLATED_TYPE_COUNT,
} Il2CppStat;

class Statistic
{
  public:
    static bool dump_to_file(const char* path);
    static uint64_t get_value(Il2CppStat stat);
};
} // namespace il2cpp
} // namespace leanclr
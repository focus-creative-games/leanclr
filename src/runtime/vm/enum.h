#pragma once

#include "rt_managed_types.h"
#include <tuple>

namespace leanclr
{
namespace vm
{
class Enum
{
  public:
    static int32_t get_enum_long_hash_code(int64_t value);

    // Get enum values and names arrays
    static RtResult<std::tuple<bool, RtArray*, RtArray*>> get_enum_values_and_names(metadata::RtClass* klass);

    // Get boxed enum data as unsigned u64
    static RtResult<uint64_t> get_boxed_enum_data_as_unsigned_and_extended_to_u64(RtObject* obj);

    // Get hash code of enum object
    static RtResult<int32_t> get_hash_code(RtObject* obj);
};
} // namespace vm
} // namespace leanclr

#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemGlobalizationCompareInfo
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Internal compare
    static RtResult<int32_t> internal_compare_icall(const char16_t* str1, int32_t length1, const char16_t* str2, int32_t length2, int32_t options) noexcept;

    // Internal index
    static RtResult<int32_t> internal_index_icall(const char16_t* source, int32_t source_start_index, int32_t source_count, const char16_t* value,
                                                  int32_t value_length, bool first) noexcept;
};

} // namespace icalls
} // namespace leanclr

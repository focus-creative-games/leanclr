#pragma once

#include "vm/internal_calls.h"

namespace leanclr
{
namespace icalls
{

class SystemGlobalizationCultureData
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResultVoid fill_culture_data(vm::RtCultureData* _this, int32_t datetime_index) noexcept;
    static RtResult<const uint8_t*> fill_number_data(int32_t number_index, void* managed) noexcept;
};

} // namespace icalls
} // namespace leanclr

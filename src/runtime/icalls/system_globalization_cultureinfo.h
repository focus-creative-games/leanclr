#pragma once

#include "vm/internal_calls.h"

namespace leanclr
{
namespace icalls
{

class SystemGlobalizationCultureInfo
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // System.Globalization.CultureInfo icalls
    static RtResult<bool> construct_internal_locale_from_lcid(vm::RtCultureInfo* _this, int32_t culture_lcid) noexcept;
    static RtResult<bool> construct_internal_locale_from_name(vm::RtCultureInfo* _this, vm::RtString* name) noexcept;
    static RtResult<vm::RtString*> get_current_locale_name() noexcept;
    static RtResult<vm::RtArray*> internal_get_cultures(bool neutral, bool specific, bool installed) noexcept;
    static RtResultVoid set_user_preferred_culture_info_in_app_x(vm::RtCultureInfo* _this, vm::RtString* name) noexcept;
    static RtResultVoid initialize_user_preferred_culture_info_in_app_x(vm::RtMulticastDelegate* onCultureInfoChangedInAppX) noexcept;
    static RtResultVoid fill_culture_data(vm::RtCultureData* _this, int32_t datetime_index) noexcept;
    static RtResult<const uint8_t*> fill_number_data(int32_t number_index, void* managed) noexcept;
};

} // namespace icalls
} // namespace leanclr

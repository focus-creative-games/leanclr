#pragma once

#include "vm/internal_calls.h"

namespace leanclr
{
namespace icalls
{

class SystemGlobalizationCultureInfo
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries();

    // System.Globalization.CultureInfo icalls
    static RtResult<bool> construct_internal_locale_from_lcid(vm::RtCultureInfo* _this, int32_t culture_lcid);
    static RtResult<bool> construct_internal_locale_from_name(vm::RtCultureInfo* _this, vm::RtString* name);
    static RtResult<vm::RtString*> get_current_locale_name();
    static RtResult<vm::RtArray*> internal_get_cultures(bool neutral, bool specific, bool installed);
};

} // namespace icalls
} // namespace leanclr

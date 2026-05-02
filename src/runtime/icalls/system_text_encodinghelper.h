#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemTextEncodingHelper
{
  public:
    // @icall: System.Text.EncodingHelper::InternalCodePage(System.Int32&)
    static RtResult<vm::RtString*> internal_code_page(int32_t* code_page) noexcept;

    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;
};

} // namespace icalls
} // namespace leanclr

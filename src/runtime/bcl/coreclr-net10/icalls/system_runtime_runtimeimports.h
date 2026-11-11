#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeRuntimeImports
{
  public:
    // @icall: System.Runtime.RuntimeImports::ZeroMemory
    static RtResultVoid zero_memory(uint8_t* ptr, uintptr_t size) noexcept;

    // @icall: System.Runtime.RuntimeImports::Memmove
    static RtResultVoid memmove(uint8_t* dest, const uint8_t* src, uintptr_t size) noexcept;

    // @icall: System.Runtime.RuntimeImports::Memmove_wbarrier
    static RtResultVoid memmove_wbarrier(uint8_t* dest, const uint8_t* src, uintptr_t size) noexcept;

    // @icall: System.Runtime.RuntimeImports::_ecvt_s
    static RtResultVoid ecvt_s(uint8_t* buffer, int32_t size, double value, int32_t digits, int32_t* decpt, int32_t* sign) noexcept;

    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;
};

} // namespace icalls
} // namespace leanclr

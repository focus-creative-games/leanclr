#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemReflectionAssemblyName
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<bool> parse_assembly_name(intptr_t name_cstr, metadata::RtMonoAssemblyName* aname, bool* is_version_defined,
                                              bool* is_token_defined) noexcept;
    static RtResultVoid get_public_token(const uint8_t* public_key, uint8_t* public_token, int32_t len) noexcept;
    static RtResult<metadata::RtMonoAssemblyName*> get_native_name(metadata::RtAssembly* ass) noexcept;
};

} // namespace icalls
} // namespace leanclr

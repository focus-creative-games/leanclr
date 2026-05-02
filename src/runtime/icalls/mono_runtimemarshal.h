#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class MonoRuntimeMarshal
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Free an assembly name structure
    static RtResultVoid free_assembly_name(metadata::RtMonoAssemblyName* aname, bool free_struct) noexcept;
};

} // namespace icalls
} // namespace leanclr

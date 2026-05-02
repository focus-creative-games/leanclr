#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

// CallingConventions enum values (matches .NET)
enum class CallingConventions : uint32_t
{
    Standard = 1,
    VarArgs = 2,
    Any = 3,
    HasThis = 0x20,
    ExplicitThis = 0x40,
};

class SystemReflectionMonoMethodInfo
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get parameter info array for a method
    static RtResult<vm::RtArray*> get_parameter_info(const metadata::RtMethodInfo* method, vm::RtReflectionMethod* member) noexcept;

    // Get method attributes
    static RtResult<int32_t> get_method_attributes(const metadata::RtMethodInfo* method) noexcept;

    // Get method info structure
    static RtResultVoid get_method_info(const metadata::RtMethodInfo* method, vm::RtMonoMethodInfo* result) noexcept;

    // Get return value marshal attribute
    static RtResult<vm::RtObject*> get_retval_marshal(const metadata::RtMethodInfo* method) noexcept;
};

} // namespace icalls
} // namespace leanclr

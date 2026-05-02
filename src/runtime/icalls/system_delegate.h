#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemDelegate
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Get the virtual method from the delegate
    static RtResult<vm::RtReflectionMethod*> get_virtual_method_internal(vm::RtDelegate* this_delegate) noexcept;

    // Create a delegate from a reflection type and method
    static RtResult<vm::RtMulticastDelegate*> create_delegate_internal(vm::RtReflectionType* delegate_type, vm::RtObject* target,
                                                                       vm::RtReflectionMethod* method, bool throw_on_bind) noexcept;

    // Allocate a delegate like another delegate
    static RtResult<vm::RtMulticastDelegate*> alloc_delegate_like_internal(vm::RtDelegate* source) noexcept;
};

} // namespace icalls
} // namespace leanclr

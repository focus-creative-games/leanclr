#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{

class SystemReflectionCustomAttribute
{
  public:
    static RtResult<bool> is_defined(vm::RtReflectionMethod* method, vm::RtReflectionRuntimeType* attribute_type, bool inherit) noexcept;
    static RtResult<bool> is_defined_on_type(vm::RtReflectionRuntimeType* type, vm::RtReflectionRuntimeType* attribute_type, bool inherit) noexcept;
    static RtResult<vm::RtArray*> get_custom_attributes(vm::RtReflectionMethod* method, vm::RtReflectionRuntimeType* attribute_type,
                                                        bool inherit) noexcept;
    static RtResult<vm::RtArray*> get_custom_attributes_data_internal(vm::RtReflectionMethod* method) noexcept;
    static RtResult<vm::RtArray*> get_custom_attributes_data_internal_on_type(vm::RtReflectionRuntimeType* type) noexcept;
    static RtResult<vm::RtArray*> get_custom_attributes_data_internal_on_provider(vm::RtObject* provider) noexcept;
    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;
};

} // namespace intrinsics
} // namespace leanclr

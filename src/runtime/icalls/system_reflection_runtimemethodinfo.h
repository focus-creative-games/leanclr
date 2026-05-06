#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{
class SystemReflectionRuntimeMethodInfo
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<vm::RtReflectionMethodBody*> get_method_body_internal(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<vm::RtReflectionMethod*> get_method_from_handle_internal_type_native(const metadata::RtMethodInfo* method,
                                                                                         const metadata::RtTypeSig* type_sig,
                                                                                         bool check_same_generic_base) noexcept;
    static RtResult<vm::RtString*> get_name(vm::RtReflectionMethod* method) noexcept;
    static RtResult<vm::RtReflectionMethod*> get_base_method(vm::RtReflectionMethod* method, bool definition) noexcept;
    static RtResult<int32_t> get_metadata_token(vm::RtReflectionMethod* method) noexcept;
    static RtResult<bool> get_is_generic_method_definition(vm::RtReflectionMethod* method) noexcept;
    static RtResult<vm::RtArray*> get_generic_arguments(vm::RtReflectionMethod* method) noexcept;
    static RtResult<vm::RtReflectionMethod*> get_generic_method_definition_impl(vm::RtReflectionMethod* method) noexcept;
    static RtResult<vm::RtReflectionMethod*> make_generic_method_impl(vm::RtReflectionMethod* method, vm::RtArray* generic_args) noexcept;
    static RtResult<bool> get_is_generic_method(vm::RtReflectionMethod* method) noexcept;
    static RtResult<vm::RtObject*> internal_invoke(vm::RtReflectionMethod* ref_method, vm::RtObject* obj, vm::RtArray* parameters, vm::RtObject** exc) noexcept;
    static RtResultVoid get_pinvoke(vm::RtReflectionMethod* method, int32_t* flags, vm::RtString** entry_name, vm::RtString** dll_name) noexcept;
};
} // namespace icalls
} // namespace leanclr

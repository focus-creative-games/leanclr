#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeMethodHandle
{
  public:
    static utils::Span<vm::InternalCallEntry> get_net10_internal_call_entries() noexcept;
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<intptr_t> get_function_pointer(intptr_t method) noexcept;
    static RtResult<int32_t> get_attributes(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<int32_t> get_impl_attributes(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<const void*> get_method_table(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<int32_t> get_slot(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<int32_t> get_method_def(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<vm::RtString*> get_name(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<const char*> get_utf8_name(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<bool> has_method_instantiation(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<bool> is_generic_method_definition(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<int32_t> get_generic_parameter_count(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<vm::RtArray*> get_method_instantiation(const metadata::RtMethodInfo* method, bool runtime_array) noexcept;
    static RtResult<bool> is_typical_method_definition(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<const metadata::RtMethodInfo*> get_stub_if_needed(const metadata::RtMethodInfo* method,
                                                                      const vm::RtReflectionRuntimeType* declaring_type) noexcept;
    static RtResult<const metadata::RtMethodInfo*> get_method_from_canonical(const metadata::RtMethodInfo* method,
                                                                             const vm::RtReflectionRuntimeType* declaring_type) noexcept;
    static RtResult<bool> is_dynamic_method(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<bool> is_constructor(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<vm::RtObject*> get_resolver(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<vm::RtObject*> get_loader_allocator(const metadata::RtMethodInfo* method) noexcept;
};

} // namespace icalls
} // namespace leanclr

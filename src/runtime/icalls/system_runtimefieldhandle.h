#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeFieldHandle
{
  public:
    static utils::Span<vm::InternalCallEntry> get_net10_internal_call_entries() noexcept;
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    static RtResult<vm::RtObject*> get_value_direct(vm::RtReflectionField* field, vm::RtReflectionRuntimeType* field_type, vm::RtTypedReference* typed_ref,
                                                     vm::RtReflectionRuntimeType* context_type) noexcept;
    static RtResultVoid set_value_direct(vm::RtReflectionField* field, vm::RtReflectionRuntimeType* field_type, vm::RtTypedReference* typed_ref, vm::RtObject* value,
                                         vm::RtReflectionRuntimeType* context_type) noexcept;
    static RtResultVoid set_value_internal(vm::RtReflectionField* field, vm::RtObject* obj, vm::RtObject* value) noexcept;
    static RtResult<int32_t> get_token(const metadata::RtFieldInfo* field) noexcept;
    static RtResult<uint32_t> get_attributes(const metadata::RtFieldInfo* field) noexcept;
    static RtResult<const void*> get_approx_declaring_method_table(const metadata::RtFieldInfo* field) noexcept;
    static RtResult<const metadata::RtFieldInfo*> get_static_field_for_generic_type(const metadata::RtFieldInfo* field, const void* method_table) noexcept;
    static RtResult<bool> acquires_context_from_this(const metadata::RtFieldInfo* field) noexcept;
    static RtResult<const char*> get_utf8_name(const metadata::RtFieldInfo* field) noexcept;
    static RtResult<bool> is_fast_path_supported(vm::RtReflectionField* field) noexcept;
};

} // namespace icalls
} // namespace leanclr

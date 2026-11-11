#pragma once

#include "vm/intrinsics.h"

namespace leanclr
{
namespace intrinsics
{
class SystemRuntimeCompilerServicesRuntimeHelpers
{
  public:
    static utils::Span<vm::IntrinsicEntry> get_intrinsic_entries() noexcept;

    static RtResult<const void*> get_method_table(vm::RtObject* obj) noexcept;
    static RtResult<void*> get_raw_data(vm::RtObject* obj) noexcept;
    static RtResult<bool> object_has_component_size(vm::RtObject* obj) noexcept;
    static RtResultVoid initialize_array(vm::RtArray* array, const metadata::RtFieldInfo* field) noexcept;
    static RtResult<uint32_t> get_num_instance_field_bytes(const void* method_table) noexcept;
    static RtResult<metadata::RtElementType> get_primitive_cor_element_type(const void* method_table) noexcept;
    static RtResult<vm::RtReadOnlySpan<uint8_t>> create_span(const metadata::RtMethodInfo* method, const metadata::RtFieldInfo* field) noexcept;
    static RtResult<vm::RtReadOnlySpan<uint8_t>> inline_array_as_span(void* buffer, int32_t length) noexcept;
    static RtResult<void*> inline_array_first_element_ref(void* buffer) noexcept;
    static RtResult<void*> inline_array_element_ref(const metadata::RtMethodInfo* method, void* buffer, int32_t index) noexcept;
    static RtResult<bool> is_bitwise_equatable(const metadata::RtMethodInfo* method) noexcept;
    static RtResult<bool> is_reference_or_contains_references(const metadata::RtMethodInfo* method) noexcept;
};
} // namespace intrinsics
} // namespace leanclr

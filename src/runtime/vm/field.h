#pragma once

#include "core/rt_base.h"
#include "metadata/rt_metadata.h"
#include "utils/rt_vector.h"
#include "utils/binary_reader.h"
#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{
class Field
{
  public:
    // Check if field is instance field (not static)
    static bool is_instance(const metadata::RtFieldInfo* field);

    // Check if field is static (includes literal and RVA)
    static bool is_static_included_literal_and_rva(const metadata::RtFieldInfo* field);

    // Check if field is static (excludes literal and RVA)
    static bool is_static_excluded_literal_and_rva(const metadata::RtFieldInfo* field);

    // Check if field is static literal
    static bool is_static_literal(const metadata::RtFieldInfo* field);

    // Check if field is static with RVA
    static bool is_static_rva(const metadata::RtFieldInfo* field);

    // Check if field is thread static
    static bool is_thread_static(const metadata::RtFieldInfo* field);

    // Check if field is public
    static bool is_public(const metadata::RtFieldInfo* field);

    // Check if field is private
    static bool is_private(const metadata::RtFieldInfo* field);

    static bool has_field_marshal(const metadata::RtFieldInfo* field);

    // Inflate field with generic context
    static RtResult<const metadata::RtFieldInfo*> inflate_field(const metadata::RtFieldInfo* field, const metadata::RtGenericContext* generic_context);

    static uint32_t get_field_offset_includes_object_header_for_all_type(const metadata::RtFieldInfo* field);
    // Get field offset including object header for reference types
    static uint32_t get_field_offset_includes_object_header_for_reference_type(const metadata::RtFieldInfo* field);

    // Get field offset including object header for all types
    static uint32_t get_instance_field_offset_includes_object_header_for_all_type(const metadata::RtFieldInfo* field);

    // Get field offset excluding object header
    static uint32_t get_field_offset_excludes_object_header_for_all_type(const metadata::RtFieldInfo* field);

    // Get field RVA data
    static RtResult<const uint8_t*> get_field_rva_data(const metadata::RtFieldInfo* field);

    // Get field const blob (for literal fields)
    static RtResult<utils::BinaryReader> get_field_const_reader(const metadata::RtFieldInfo* field);
    static RtResult<const void*> get_field_const_data(const metadata::RtFieldInfo* field);

    // Get field const object (for literal object fields)
    static RtResult<RtObject*> get_field_const_object(const metadata::RtFieldInfo* field);

    // Set instance field value
    static RtResultVoid get_instance_value(const metadata::RtFieldInfo* field, void* obj, void* value);
    static RtResultVoid set_instance_value(const metadata::RtFieldInfo* field, void* obj, const void* value);

    // Set static field value
    static RtResultVoid get_static_value(const metadata::RtFieldInfo* field, void* value);
    static RtResultVoid set_static_value(const metadata::RtFieldInfo* field, const void* value);

    // Get field size
    static RtResult<size_t> get_field_size(const metadata::RtFieldInfo* field);

    // Get field value as object (boxing for value types)
    static RtResult<RtObject*> get_value_object(const metadata::RtFieldInfo* field, RtObject* obj);

    // Set field value from object (unboxing for value types)
    static RtResultVoid set_value_object(const metadata::RtFieldInfo* field, RtObject* obj, RtObject* value);

    // Find field by name in class
    static RtResult<const metadata::RtFieldInfo*> find_field_by_name(metadata::RtClass* klass, const char* fieldName);

    // Get field modifiers
    static RtResultVoid get_field_modifiers(const metadata::RtFieldInfo* field, bool optional, utils::Vector<metadata::RtClass*>& modifiers);
};
} // namespace vm
} // namespace leanclr

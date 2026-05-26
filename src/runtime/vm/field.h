#pragma once

#include <cassert>

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
    static bool is_instance(const metadata::RtFieldInfo* field)
    {
        return (field->flags & static_cast<uint32_t>(metadata::RtFieldAttribute::Static)) == 0;
    }

    // Check if field is static (includes literal and RVA)
    static bool is_static_included_literal_and_rva(const metadata::RtFieldInfo* field)
    {
        return (field->flags & static_cast<uint32_t>(metadata::RtFieldAttribute::Static)) != 0;
    }

    // Check if field is static (excludes literal and RVA)
    static bool is_static_excluded_literal_and_rva(const metadata::RtFieldInfo* field)
    {
        uint32_t flags = field->flags;
        return (flags & static_cast<uint32_t>(metadata::RtFieldAttribute::Static)) != 0 &&
               (flags & (static_cast<uint32_t>(metadata::RtFieldAttribute::Literal) | static_cast<uint32_t>(metadata::RtFieldAttribute::HasFieldRva))) == 0;
    }

    // Check if field is static literal
    static bool is_static_literal(const metadata::RtFieldInfo* field)
    {
        uint32_t flags = field->flags;
        return (flags & static_cast<uint32_t>(metadata::RtFieldAttribute::Static)) != 0 &&
               (flags & static_cast<uint32_t>(metadata::RtFieldAttribute::Literal)) != 0;
    }

    // Check if field is static with RVA
    static bool is_static_rva(const metadata::RtFieldInfo* field)
    {
        uint32_t flags = field->flags;
        return (flags & static_cast<uint32_t>(metadata::RtFieldAttribute::Static)) != 0 &&
               (flags & static_cast<uint32_t>(metadata::RtFieldAttribute::HasFieldRva)) != 0;
    }

    // Check if field is thread static
    static bool is_thread_static(const metadata::RtFieldInfo* field)
    {
        (void)field;
        // Thread static fields not yet implemented
        return false;
    }

    // Check if field is public
    static bool is_public(const metadata::RtFieldInfo* field)
    {
        uint32_t visibility = field->flags & static_cast<uint32_t>(metadata::RtFieldAttribute::FieldAccessMask);
        return visibility == static_cast<uint32_t>(metadata::RtFieldAttribute::Public);
    }

    // Check if field is private
    static bool is_private(const metadata::RtFieldInfo* field)
    {
        uint32_t visibility = field->flags & static_cast<uint32_t>(metadata::RtFieldAttribute::FieldAccessMask);
        return visibility == static_cast<uint32_t>(metadata::RtFieldAttribute::Private);
    }

    static bool has_field_marshal(const metadata::RtFieldInfo* field)
    {
        return (field->flags & static_cast<uint32_t>(metadata::RtFieldAttribute::HasFieldMarshal)) != 0;
    }

    // Inflate field with generic context
    static RtResult<const metadata::RtFieldInfo*> inflate_field(const metadata::RtFieldInfo* field, const metadata::RtGenericContext* generic_context);

    static uint32_t get_field_offset_includes_object_header_for_all_type(const metadata::RtFieldInfo* field)
    {
        if (is_instance(field))
        {
            return field->offset + RT_OBJECT_HEADER_SIZE;
        }
        return field->offset;
    }

    // Get field offset including object header for reference types
    static uint32_t get_field_offset_includes_object_header_for_reference_type(const metadata::RtFieldInfo* field)
    {
        if (is_instance(field))
        {
            const metadata::RtClass* parent = field->parent;
            if ((parent->extra_flags & static_cast<uint32_t>(metadata::RtClassExtraAttribute::ReferenceType)) != 0)
            {
                return field->offset + RT_OBJECT_HEADER_SIZE;
            }
        }
        return field->offset;
    }

    // Get field offset including object header for all types
    static uint32_t get_instance_field_offset_includes_object_header_for_all_type(const metadata::RtFieldInfo* field)
    {
        assert(is_instance(field));
        return field->offset + RT_OBJECT_HEADER_SIZE;
    }

    // Get field offset excluding object header
    static uint32_t get_field_offset_excludes_object_header_for_all_type(const metadata::RtFieldInfo* field)
    {
        return field->offset;
    }

    // Get field RVA data
    static RtResult<const uint8_t*> get_field_rva_data(const metadata::RtFieldInfo* field);

    // Get field const blob (for literal fields)
    static RtResult<metadata::TypedConstRawData> get_field_const_reader(const metadata::RtFieldInfo* field);
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

    static RtResult<RtObject*> get_value_direct(const metadata::RtFieldInfo* field, void* ptr_struct_data);
    static RtResultVoid set_value_direct(const metadata::RtFieldInfo* field, void* ptr_struct_data, void* ptr_field_value);

    static const metadata::RtFieldInfo* get_nullable_has_value_field(const metadata::RtClass* klass)
    {
        // The first field is the "HasValue" boolean
        assert(strcmp(klass->name, "Nullable`1") == 0);
        assert(klass->field_count == 2);
        const metadata::RtFieldInfo* has_value_field = klass->fields;
        assert(strcmp(has_value_field->name, "hasValue") == 0);
        return has_value_field;
    }
    static const metadata::RtFieldInfo* get_nullable_value_field(const metadata::RtClass* klass)
    {
        // The second field is the actual value
        assert(strcmp(klass->name, "Nullable`1") == 0);
        assert(klass->field_count == 2);
        const metadata::RtFieldInfo* value_field = klass->fields + 1;
        assert(strcmp(value_field->name, "value") == 0);
        return value_field;
    }

    // Find field by name in class
    static RtResult<const metadata::RtFieldInfo*> find_field_by_name(metadata::RtClass* klass, const char* fieldName);

    // Get field modifiers
    static RtResultVoid get_field_modifiers(const metadata::RtFieldInfo* field, bool optional, utils::Vector<metadata::RtClass*>& modifiers);
};
} // namespace vm
} // namespace leanclr

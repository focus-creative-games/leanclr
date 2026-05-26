
#include <cstring>
#include <cstdlib>

#include "field.h"
#include "class.h"
#include "runtime.h"
#include "object.h"
#include "type.h"

#include "utils/string_util.h"
#include "metadata/metadata_cache.h"
#include "metadata/module_def.h"
#include "metadata/generic_metadata.h"
#include "metadata/metadata_const.h"

namespace leanclr
{
namespace vm
{

// Inflate field with generic context
RtResult<const metadata::RtFieldInfo*> Field::inflate_field(const metadata::RtFieldInfo* field, const metadata::RtGenericContext* generic_context)
{
    metadata::RtClass* klass = field->parent;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, inflatedClass, metadata::GenericMetadata::inflate_class(klass, generic_context));
    if (inflatedClass == klass)
    {
        RET_OK(field);
    }
    assert(Class::get_family(inflatedClass) == metadata::RtClassFamily::GenericInst);
    RET_ERR_ON_FAIL(Class::initialize_fields(inflatedClass));
    uint32_t fieldIndex = field->token - inflatedClass->fields[0].token;
    assert(fieldIndex < inflatedClass->field_count);
    RET_OK(&inflatedClass->fields[fieldIndex]);
}

// Get field RVA data
RtResult<const uint8_t*> Field::get_field_rva_data(const metadata::RtFieldInfo* field)
{
    if (is_static_rva(field))
    {
        return field->parent->image->get_field_rva_data(field->token);
    }
    else
    {
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
}

// Get field const data (placeholder)
RtResult<metadata::TypedConstRawData> Field::get_field_const_reader(const metadata::RtFieldInfo* field)
{
    if (is_static_literal(field))
    {
        return field->parent->image->get_const_or_default_value(field->token);
    }
    RET_ASSERT_ERR(RtErr::ExecutionEngine);
}

RtResult<const void*> Field::get_field_const_data(const metadata::RtFieldInfo* field)
{
    auto ret_const_data = get_field_const_reader(field);
    if (ret_const_data.is_ok())
    {
        return ret_const_data.unwrap().data;
    }
    else
    {
        return ret_const_data.unwrap_err();
    }
}

// Get field const object
RtResult<RtObject*> Field::get_field_const_object(const metadata::RtFieldInfo* field)
{
    return metadata::MetadataConst::decode_const_object(field->parent->image, field->token, field->type_sig);
}

RtResultVoid Field::get_instance_value(const metadata::RtFieldInfo* field, void* obj, void* value)
{
    assert(is_instance(field));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, size, get_field_size(field));

    uint8_t* target = static_cast<uint8_t*>(obj) + get_instance_field_offset_includes_object_header_for_all_type(field);
    std::memcpy(value, target, size);

    RET_VOID_OK();
}

// Set instance field value
RtResultVoid Field::set_instance_value(const metadata::RtFieldInfo* field, void* obj, const void* value)
{
    assert(is_instance(field));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, size, get_field_size(field));

    uint8_t* target = static_cast<uint8_t*>(obj) + get_instance_field_offset_includes_object_header_for_all_type(field);
    std::memcpy(target, value, size);

    RET_VOID_OK();
}

// Set static field value
RtResultVoid Field::get_static_value(const metadata::RtFieldInfo* field, void* value)
{
    if (!is_static_excluded_literal_and_rva(field))
    {
        RET_ERR(RtErr::FieldAccess);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, size, get_field_size(field));

    std::memcpy(value, field->parent->static_fields_data + field->offset, size);

    RET_VOID_OK();
}

RtResultVoid Field::set_static_value(const metadata::RtFieldInfo* field, const void* value)
{
    if (!is_static_excluded_literal_and_rva(field))
    {
        RET_ERR(RtErr::FieldAccess);
    }

    metadata::RtClass* klass = static_cast<metadata::RtClass*>(field->parent);

    // TODO: Call run_class_static_constructor(klass)?

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, size, get_field_size(field));

    uint8_t* static_field_data_ptr = klass->static_fields_data + field->offset;
    std::memcpy(static_field_data_ptr, value, size);

    RET_VOID_OK();
}

// Get field size
RtResult<size_t> Field::get_field_size(const metadata::RtFieldInfo* field)
{
    return Type::get_size_of_type(field->type_sig);
}

// Get field value as object (boxing for value types)
RtResult<RtObject*> Field::get_value_object(const metadata::RtFieldInfo* field, RtObject* obj)
{
    metadata::RtClass* klass = field->parent;

    RET_ERR_ON_FAIL(vm::Runtime::run_class_static_constructor(klass));

    uint8_t* fieldDataPtr = nullptr;

    if (is_static_included_literal_and_rva(field))
    {
        if (is_static_literal(field))
        {
            return get_field_const_object(field);
        }
        else if (is_static_rva(field))
        {
            RET_OK(nullptr);
        }
        else
        {
            fieldDataPtr = klass->static_fields_data + field->offset;
        }
    }
    else
    {
        if (obj == nullptr)
        {
            RET_ERR(RtErr::NullReference);
        }
        fieldDataPtr = reinterpret_cast<uint8_t*>(obj) + get_field_offset_includes_object_header_for_all_type(field);
    }

    // Get the type class for the field
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, fieldClass, Class::get_class_from_typesig(field->type_sig));

    if (Class::is_reference_type(fieldClass))
    {
        // Return reference type directly
        RET_OK(*reinterpret_cast<RtObject**>(fieldDataPtr));
    }
    else
    {
        return LEANCLR_BOX_OBJECT_INTERNAL(fieldClass, fieldDataPtr, "Field::get_value_object");
    }
}

// Set field value from object (unboxing for value types)
RtResultVoid Field::set_value_object(const metadata::RtFieldInfo* field, RtObject* obj, RtObject* value)
{
    metadata::RtClass* klass = field->parent;

    RET_ERR_ON_FAIL(vm::Runtime::run_class_static_constructor(klass));

    uint8_t* fieldDataPtr = nullptr;

    if (is_static_included_literal_and_rva(field))
    {
        if (is_static_literal(field))
        {
            RET_ERR(RtErr::FieldAccess);
        }
        else if (is_static_rva(field))
        {
            RET_ERR(RtErr::FieldAccess);
        }
        else
        {
            fieldDataPtr = klass->static_fields_data + field->offset;
        }
    }
    else
    {
        if (obj == nullptr)
        {
            RET_ERR(RtErr::NullReference);
        }
        fieldDataPtr = reinterpret_cast<uint8_t*>(obj) + get_field_offset_includes_object_header_for_all_type(field);
    }

    // Get the type class for the field
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, field_klass, Class::get_class_from_typesig(field->type_sig));

    // FIXME: handle gc write barrier
    if (Class::is_reference_type(field_klass))
    {
        // Reference type: direct assignment
        *reinterpret_cast<void**>(fieldDataPtr) = value;
    }
    else
    {
        if (Class::is_nullable_type(field_klass))
        {
            if (value == nullptr)
            {
                std::memset(fieldDataPtr, 0, klass->instance_size_without_header);
            }
            else
            {
                size_t has_value_field_offset = get_nullable_has_value_field(field_klass)->offset;
                size_t value_field_offset = get_nullable_value_field(field_klass)->offset;
                fieldDataPtr[has_value_field_offset] = 1;
                std::memcpy(fieldDataPtr + value_field_offset, value, field_klass->element_class->instance_size_without_header);
            }
        }
        else
        {
            if (value == nullptr)
            {
                RET_ERR(RtErr::ArgumentNull);
            }
            std::memcpy(fieldDataPtr, value + 1, field_klass->instance_size_without_header);
        }
    }

    RET_VOID_OK();
}

RtResult<RtObject*> Field::get_value_direct(const metadata::RtFieldInfo* field, void* ptr_struct_data)
{
    assert(field != nullptr);
    assert(ptr_struct_data != nullptr);
    assert(Class::is_value_type(field->parent));
    if (!is_instance(field))
    {
        RET_ERR(RtErr::Argument);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, fieldClass, Class::get_class_from_typesig(field->type_sig));
    void* field_data_ptr = reinterpret_cast<uint8_t*>(ptr_struct_data) + get_field_offset_excludes_object_header_for_all_type(field);
    if (Class::is_reference_type(fieldClass))
    {
        // Return reference type directly
        RET_OK(*reinterpret_cast<RtObject**>(field_data_ptr));
    }
    else
    {
        return LEANCLR_BOX_OBJECT_INTERNAL(fieldClass, field_data_ptr, "Field::get_value_direct");
    }
}

RtResultVoid Field::set_value_direct(const metadata::RtFieldInfo* field, void* ptr_struct_data, void* ptr_field_value)
{
    assert(field != nullptr);
    assert(ptr_struct_data != nullptr);
    assert(ptr_field_value != nullptr);
    assert(Class::is_value_type(field->parent));
    if (!is_instance(field))
    {
        RET_ERR(RtErr::Argument);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, fieldClass, Class::get_class_from_typesig(field->type_sig));
    uint8_t* field_data_ptr = reinterpret_cast<uint8_t*>(ptr_struct_data) + get_field_offset_excludes_object_header_for_all_type(field);
    if (Class::is_reference_type(fieldClass))
    {
        // Return reference type directly
        *reinterpret_cast<RtObject**>(field_data_ptr) = *reinterpret_cast<RtObject**>(ptr_field_value);
    }
    else
    {
        if (Class::is_nullable_type(fieldClass))
        {
            if (ptr_field_value == nullptr)
            {
                std::memset(field_data_ptr, 0, fieldClass->instance_size_without_header);
            }
            else
            {
                size_t has_value_field_offset = get_nullable_has_value_field(fieldClass)->offset;
                size_t value_field_offset = get_nullable_value_field(fieldClass)->offset;
                field_data_ptr[has_value_field_offset] = 1;
                std::memcpy(field_data_ptr + value_field_offset, ptr_field_value, fieldClass->element_class->instance_size_without_header);
            }
        }
        else
        {
            std::memcpy(field_data_ptr, ptr_field_value, fieldClass->instance_size_without_header);
        }
    }
    RET_VOID_OK();
}

// Find field by name in class
RtResult<const metadata::RtFieldInfo*> Field::find_field_by_name(metadata::RtClass* klass, const char* fieldName)
{
    RET_ERR_ON_FAIL(Class::initialize_fields(klass));

    uint32_t field_count = klass->field_count;
    const metadata::RtFieldInfo* field_data_ptr = klass->fields;

    for (uint32_t i = 0; i < field_count; ++i)
    {
        const metadata::RtFieldInfo* fieldInfo = field_data_ptr + i;
        if (std::strcmp(fieldInfo->name, fieldName) == 0)
        {
            RET_OK(fieldInfo);
        }
    }

    RET_OK(nullptr);
}

// Get field modifiers
RtResultVoid Field::get_field_modifiers(const metadata::RtFieldInfo* field, bool optional, utils::Vector<metadata::RtClass*>& modifiers)
{
    metadata::RtModuleDef* mod = field->parent->image;
    auto optFieldRow = mod->get_cli_image().read_field(metadata::RtToken::decode_rid(field->token));
    assert(optFieldRow.has_value());

    auto retBlobReader = mod->get_decoded_blob_reader(optFieldRow->signature);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, blobReader, retBlobReader);

    metadata::RtGenericContainerContext gcc{};
    return mod->read_member_modifier(blobReader, optional, gcc, nullptr, modifiers);
}

} // namespace vm
} // namespace leanclr

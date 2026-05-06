#include "core/stl_compat.h"

#include "customattribute.h"
#include "class.h"
#include "assembly.h"
#include "object.h"
#include "field.h"
#include "method.h"
#include "rt_string.h"
#include "rt_array.h"
#include "reflection.h"
#include "type.h"
#include "runtime.h"
#include "utils/binary_reader.h"
#include "utils/rt_span.h"
#include "gc/garbage_collector.h"
#include "metadata/module_def.h"
#include "const_strs.h"

namespace leanclr
{
namespace vm
{

// Helper structures
struct FixedArg
{
    metadata::RtElementType ele_type;
    uint64_t value;
};

struct NamedArgWithoutValue
{
    bool is_field;
    metadata::RtElementType field_or_prop_type;
    utils::Span<const char> name;
};

// Static helper functions
static RtResult<std::optional<utils::Span<const char>>> read_ser_string(utils::BinaryReader* reader)
{
    uint8_t first_byte = 0;
    if (!reader->try_read_byte(first_byte))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    if (first_byte == 0xFF)
    {
        return RtResult<std::optional<utils::Span<const char>>>(std::nullopt); // null string
    }
    else if (first_byte == 0)
    {
        return RtResult<std::optional<utils::Span<const char>>>(utils::Span<const char>("", 0)); // empty string
    }
    else
    {
        if (!reader->try_offset(-1))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        uint32_t len = 0;
        if (!reader->try_read_compressed_uint32(len))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        const uint8_t* str_ptr = reader->get_current_ptr();
        if (!reader->try_advance(len))
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        return std::optional<utils::Span<const char>>(utils::Span<const char>(reinterpret_cast<const char*>(str_ptr), len));
    }
}

static RtResult<uint64_t> read_customattribute_elem_simple_value(utils::BinaryReader* reader, metadata::RtElementType ele_type)
{
    uint64_t value = 0;

    switch (ele_type)
    {
    case metadata::RtElementType::Boolean:
    {
        uint8_t b = 0;
        if (!reader->try_read_byte(b))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        value = (b != 0) ? 1u : 0u;
        break;
    }
    case metadata::RtElementType::Char:
    case metadata::RtElementType::U2:
    {
        uint16_t c = 0;
        if (!reader->try_read_u16(c))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        value = (uint64_t)c;
        break;
    }
    case metadata::RtElementType::I1:
    {
        int8_t i = 0;
        if (!reader->try_read_any(i))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        value = (uint64_t)i;
        break;
    }
    case metadata::RtElementType::U1:
    {
        uint8_t u = 0;
        if (!reader->try_read_byte(u))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        value = (uint64_t)u;
        break;
    }
    case metadata::RtElementType::I2:
    {
        int16_t i = 0;
        if (!reader->try_read_any(i))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        value = (uint64_t)i;
        break;
    }
    case metadata::RtElementType::I4:
    {
        int32_t i = 0;
        if (!reader->try_read_any(i))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        value = (uint64_t)i;
        break;
    }
    case metadata::RtElementType::U4:
    {
        uint32_t u = 0;
        if (!reader->try_read_u32(u))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        value = (uint64_t)u;
        break;
    }
    case metadata::RtElementType::I8:
    {
        int64_t i = 0;
        if (!reader->try_read_any(i))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        value = (uint64_t)i;
        break;
    }
    case metadata::RtElementType::U8:
    {
        if (!reader->try_read_u64(value))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        break;
    }
    case metadata::RtElementType::R4:
    {
        float f = 0.0f;
        if (!reader->try_read_f32(f))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        *(float*)&value = f;
        break;
    }
    case metadata::RtElementType::R8:
    {
        double d = 0.0;
        if (!reader->try_read_f64(d))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        *(double*)&value = d;
        break;
    }
    default:
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    RET_OK(value);
}

static RtResult<uint64_t> read_customattribute_elem_value(metadata::RtModuleDef* mod, utils::BinaryReader* reader, metadata::RtElementType ele_type);

static RtResult<metadata::RtElementType> get_custom_attribute_elem_type_from_typesig(const metadata::RtTypeSig* type_sig)
{
    metadata::RtElementType original_ele_type = type_sig->ele_type;
    metadata::RtElementType ca_ele_type = original_ele_type;

    switch (original_ele_type)
    {
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::Char:
    case metadata::RtElementType::I1:
    case metadata::RtElementType::U1:
    case metadata::RtElementType::I2:
    case metadata::RtElementType::U2:
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    case metadata::RtElementType::R4:
    case metadata::RtElementType::R8:
    case metadata::RtElementType::String:
    case metadata::RtElementType::SZArray:
        ca_ele_type = original_ele_type;
        break;

    case metadata::RtElementType::Object:
        ca_ele_type = metadata::RtElementType::CAObject;
        break;

    case metadata::RtElementType::ValueType:
    case metadata::RtElementType::Class:
    case metadata::RtElementType::GenericInst:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(type_sig));
        const CorLibTypes& types = Class::get_corlib_types();

        if (Class::is_enum_type(klass))
        {
            ca_ele_type = Class::get_element_type(klass->element_class);
        }
        else if (klass == types.cls_systemtype)
        {
            ca_ele_type = metadata::RtElementType::CASystemType;
        }
        else
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        break;
    }
    default:
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    return ca_ele_type;
}

static RtResult<uint64_t> read_customattribute_elem_value(metadata::RtModuleDef* mod, utils::BinaryReader* reader, metadata::RtElementType ele_type)
{
    uint64_t val = 0;

    switch (ele_type)
    {
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::Char:
    case metadata::RtElementType::I1:
    case metadata::RtElementType::U1:
    case metadata::RtElementType::I2:
    case metadata::RtElementType::U2:
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    case metadata::RtElementType::R4:
    case metadata::RtElementType::R8:
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(val, read_customattribute_elem_simple_value(reader, ele_type));
        break;
    }
    case metadata::RtElementType::String:
    {
        size_t s_len = 0;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(std::optional<utils::Span<const char>>, s, read_ser_string(reader));
        if (s)
        {
            auto& data = s.value();
            RtString* str_obj = String::create_string_from_utf8chars(data.data(), static_cast<int32_t>(data.size()));
            val = (uint64_t)str_obj;
        }
        else
        {
            val = 0; // null string
        }
        break;
    }

    case metadata::RtElementType::CASystemType:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<utils::Span<const char>>, opt_type_name, read_ser_string(reader));
        if (!opt_type_name)
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        auto& type_name_span = opt_type_name.value();

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, type_obj,
                                                CustomAttribute::parse_assembly_qualified_type(mod, type_name_span.data(), type_name_span.size(), false));
        val = (uint64_t)type_obj;
        break;
    }

    case metadata::RtElementType::CAObject:
    {
        uint8_t val_type_byte = 0;
        if (!reader->try_read_byte(val_type_byte))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        metadata::RtElementType val_type = static_cast<metadata::RtElementType>(val_type_byte);

        const CorLibTypes& corlib_types = Class::get_corlib_types();

        RtObject* obj = nullptr;
        switch (val_type)
        {
        case metadata::RtElementType::Boolean:
        {
            uint8_t raw_data = 0;
            if (!reader->try_read_byte(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(corlib_types.cls_boolean, &raw_data));
            break;
        }
        case metadata::RtElementType::Char:
        {
            uint16_t raw_data = 0;
            if (!reader->try_read_u16(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(corlib_types.cls_char, &raw_data));
            break;
        }
        case metadata::RtElementType::I1:
        {
            int8_t raw_data = 0;
            if (!reader->try_read_any(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(corlib_types.cls_sbyte, &raw_data));
            break;
        }
        case metadata::RtElementType::U1:
        {
            uint8_t raw_data = 0;
            if (!reader->try_read_byte(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(corlib_types.cls_byte, &raw_data));
            break;
        }
        case metadata::RtElementType::I2:
        {
            int16_t raw_data = 0;
            if (!reader->try_read_any(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(corlib_types.cls_int16, &raw_data));
            break;
        }
        case metadata::RtElementType::U2:
        {
            uint16_t raw_data = 0;
            if (!reader->try_read_u16(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(corlib_types.cls_uint16, &raw_data));
            break;
        }
        case metadata::RtElementType::I4:
        {
            int32_t raw_data = 0;
            if (!reader->try_read_any(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(corlib_types.cls_int32, &raw_data));
            break;
        }
        case metadata::RtElementType::U4:
        {
            uint32_t raw_data = 0;
            if (!reader->try_read_u32(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(corlib_types.cls_uint32, &raw_data));
            break;
        }
        case metadata::RtElementType::I8:
        {
            int64_t raw_data = 0;
            if (!reader->try_read_any(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(corlib_types.cls_int64, &raw_data));
            break;
        }
        case metadata::RtElementType::U8:
        {
            uint64_t raw_data = 0;
            if (!reader->try_read_u64(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(corlib_types.cls_uint64, &raw_data));
            break;
        }
        case metadata::RtElementType::R4:
        {
            float raw_data = 0.0f;
            if (!reader->try_read_f32(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(corlib_types.cls_single, &raw_data));
            break;
        }
        case metadata::RtElementType::R8:
        {
            double raw_data = 0.0;
            if (!reader->try_read_f64(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(corlib_types.cls_double, &raw_data));
            break;
        }
        case metadata::RtElementType::String:
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<utils::Span<const char>>, s, read_ser_string(reader));
            if (s)
            {
                auto& data = s.value();
                obj = String::create_string_from_utf8chars(data.data(), static_cast<int32_t>(data.size()));
            }
            else
            {
                obj = nullptr; // null string
            }
            break;
        }
        case metadata::RtElementType::CAEnum:
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<utils::Span<const char>>, opt_enum_name, read_ser_string(reader));
            if (!opt_enum_name)
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            auto& enum_name_span = opt_enum_name.value();
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, type_obj,
                                                    CustomAttribute::parse_assembly_qualified_type(mod, enum_name_span.data(), enum_name_span.size(), false));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, enum_klass, Class::get_class_from_typesig(type_obj->type_handle));
            if (!Class::is_enum_type(enum_klass))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint64_t, enum_value,
                                                    read_customattribute_elem_simple_value(reader, Class::get_element_type(enum_klass->element_class)));
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, Object::box_object(enum_klass, &enum_value));
            break;
        }
        case metadata::RtElementType::CASystemType:
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<utils::Span<const char>>, opt_type_name, read_ser_string(reader));
            if (!opt_type_name)
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            auto& type_name_span = opt_type_name.value();

            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, type_obj,
                                                    CustomAttribute::parse_assembly_qualified_type(mod, type_name_span.data(), type_name_span.size(), false));
            obj = (RtObject*)type_obj;
            break;
        }
        case metadata::RtElementType::SZArray:
        {
            uint8_t ele_type_byte = 0;
            if (!reader->try_read_byte(ele_type_byte))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            metadata::RtElementType ele_type = static_cast<metadata::RtElementType>(ele_type_byte);
            uint32_t num_elems = 0;
            if (!reader->try_read_u32(num_elems))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            if (num_elems == UINT32_MAX)
            {
                obj = nullptr; // null array
            }
            else
            {
                metadata::RtTypeSig ele_type_sig{};
                ele_type_sig.ele_type = ele_type;
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, ele_klass, Class::get_class_from_typesig(&ele_type_sig));
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, elem_arr, Array::new_szarray_from_ele_klass(ele_klass, static_cast<int32_t>(num_elems)));
                size_t ele_size = Array::get_array_element_size(elem_arr);
                uint8_t* arr_data_ptr = Array::get_array_data_start_as<uint8_t>(elem_arr);
                for (uint32_t i = 0; i < num_elems; ++i)
                {
                    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint64_t, elem_value, read_customattribute_elem_value(mod, reader, ele_type));
                    uint8_t* elem_addr = arr_data_ptr + i * ele_size;
                    std::memcpy(elem_addr, &elem_value, ele_size);
                    // gc::GarbageCollector::write_barrier((RtObject**)elem_addr, (RtObject*)elem_value);
                }
                obj = elem_arr;
            }
            break;
        }
        default:
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }

        val = (uint64_t)obj;
        break;
    }

    default:
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    return val;
}

static RtResult<FixedArg> read_fixed_arg(metadata::RtModuleDef* mod, const metadata::RtTypeSig* param_type, utils::BinaryReader* reader)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtElementType, ele_type, get_custom_attribute_elem_type_from_typesig(param_type));

    FixedArg result{};
    result.ele_type = ele_type;

    if (ele_type != metadata::RtElementType::SZArray)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(result.value, read_customattribute_elem_value(mod, reader, ele_type));
    }
    else
    {
        uint32_t num_elems = 0;
        if (!reader->try_read_u32(num_elems))
            RET_ASSERT_ERR(RtErr::BadImageFormat);

        if (num_elems == UINT32_MAX)
        {
            result.value = 0; // null array
        }
        else
        {
            const metadata::RtTypeSig* ele_type_sig = param_type->data.element_type;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, ele_klass, Class::get_class_from_typesig(ele_type_sig));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, elem_arr, Array::new_szarray_from_ele_klass(ele_klass, static_cast<int32_t>(num_elems)));

            size_t ele_size = Array::get_array_element_size(elem_arr);
            uint8_t* arr_data_ptr = Array::get_array_data_start_as<uint8_t>(elem_arr);

            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtElementType, ele_ele_type, get_custom_attribute_elem_type_from_typesig(ele_type_sig));

            for (uint32_t i = 0; i < num_elems; ++i)
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint64_t, elem_value, read_customattribute_elem_value(mod, reader, ele_ele_type));
                uint8_t* elem_addr = arr_data_ptr + i * ele_size;
                std::memcpy(elem_addr, &elem_value, ele_size);
                // gc::GarbageCollector::write_barrier((RtObject**)elem_addr, (RtObject*)elem_value);
            }

            result.value = (uint64_t)elem_arr;
        }
    }

    RET_OK(result);
}

static RtResult<metadata::RtElementType> read_field_or_prop_type(metadata::RtModuleDef* mod, utils::BinaryReader* reader)
{
    uint8_t ele_type_byte = 0;
    if (!reader->try_read_byte(ele_type_byte))
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    metadata::RtElementType ele_type = static_cast<metadata::RtElementType>(ele_type_byte);

    metadata::RtElementType result = ele_type;

    switch (ele_type)
    {
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::Char:
    case metadata::RtElementType::I1:
    case metadata::RtElementType::U1:
    case metadata::RtElementType::I2:
    case metadata::RtElementType::U2:
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    case metadata::RtElementType::R4:
    case metadata::RtElementType::R8:
    case metadata::RtElementType::String:
    case metadata::RtElementType::CASystemType:
    case metadata::RtElementType::CAObject:
        result = ele_type;
        break;

    case metadata::RtElementType::SZArray:
    {
        RET_ERR_ON_FAIL(read_field_or_prop_type(mod, reader));
        result = metadata::RtElementType::SZArray;
        break;
    }

    case metadata::RtElementType::CAEnum:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<utils::Span<const char>>, opt_enum_name, read_ser_string(reader));
        if (!opt_enum_name)
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        auto& enum_name_span = opt_enum_name.value();
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, type_obj,
                                                CustomAttribute::parse_assembly_qualified_type(mod, enum_name_span.data(), enum_name_span.size(), false));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, enum_klass, Class::get_class_from_typesig(type_obj->type_handle));
        if (!Class::is_enum_type(enum_klass))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        result = Class::get_element_type(enum_klass->element_class);
        break;
    }
    default:
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    RET_OK(result);
}

static RtResult<NamedArgWithoutValue> read_named_arg(metadata::RtModuleDef* mod, utils::BinaryReader* reader)
{
    uint8_t type_tag = 0;
    if (!reader->try_read_byte(type_tag))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    bool is_field;
    if (type_tag == (uint8_t)metadata::RtElementType::CAField)
    {
        is_field = true;
    }
    else if (type_tag == (uint8_t)metadata::RtElementType::CAProperty)
    {
        is_field = false;
    }
    else
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtElementType, field_or_prop_type, read_field_or_prop_type(mod, reader));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<utils::Span<const char>>, name, read_ser_string(reader));
    if (!name)
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    NamedArgWithoutValue result{};
    result.is_field = is_field;
    result.field_or_prop_type = field_or_prop_type;
    result.name = name.value();

    RET_OK(result);
}

// Public API implementations

RtResult<RtReflectionType*> CustomAttribute::parse_assembly_qualified_type(metadata::RtModuleDef* default_mod, const char* assembly_qualified_type_name,
                                                                           size_t name_len, bool ignore_case)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, typeSig,
                                            vm::Type::parse_assembly_qualified_type(default_mod, assembly_qualified_type_name, name_len, ignore_case));
    return Reflection::get_type_reflection_object(typeSig);
}

static RtResult<bool> is_type_match_for_custom_attribute_elem(metadata::RtElementType ca_ele_type, const metadata::RtTypeSig* field_or_property_typesig)
{
    metadata::RtElementType expected_ele_type = field_or_property_typesig->ele_type;
    bool result;
    switch (ca_ele_type)
    {
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::Char:
    case metadata::RtElementType::I1:
    case metadata::RtElementType::U1:
    case metadata::RtElementType::I2:
    case metadata::RtElementType::U2:
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    case metadata::RtElementType::R4:
    case metadata::RtElementType::R8:
    {
        if (expected_ele_type == ca_ele_type)
        {
            result = true;
        }
        else if (expected_ele_type == metadata::RtElementType::ValueType || expected_ele_type == metadata::RtElementType::GenericInst)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, expected_klass, Class::get_class_from_typesig(field_or_property_typesig));
            if (Class::is_enum_type(expected_klass))
            {
                metadata::RtElementType enum_underlying_type = Class::get_element_type(expected_klass->element_class);
                result = (enum_underlying_type == ca_ele_type);
            }
            else
            {
                result = false;
            }
        }
        else
        {
            result = false;
        }
        break;
    }
    case metadata::RtElementType::String:
    {
        result = (expected_ele_type == metadata::RtElementType::String || expected_ele_type == metadata::RtElementType::Object);
        break;
    }
    case metadata::RtElementType::CAObject:
    {
        result = (expected_ele_type == metadata::RtElementType::Object);
        break;
    }
    case metadata::RtElementType::CASystemType:
    {
        result = (expected_ele_type == metadata::RtElementType::Class || expected_ele_type == metadata::RtElementType::Object);
        break;
    }
    case metadata::RtElementType::SZArray:
    {
        result = (expected_ele_type == metadata::RtElementType::SZArray || expected_ele_type == metadata::RtElementType::Object);
        break;
    }
    default:
    {
        result = false;
        break;
    }
    }

    RET_OK(result);
}

RtResult<RtObject*> CustomAttribute::read_custom_attribute(metadata::RtModuleDef* mod, const metadata::RtCustomAttributeRawData* data)
{
    const metadata::RtMethodInfo* ctor_method = data->ctor;
    const metadata::RtClass* klass = ctor_method->parent;
    RET_ERR_ON_FAIL(Class::initialize_all(const_cast<metadata::RtClass*>(klass)));

    const CorLibTypes& types = Class::get_corlib_types();
    if (!Class::has_class_parent_fast(klass, types.cls_attribute))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ca_obj, Object::new_object(klass));

    uint32_t param_count = ctor_method->parameter_count;

    if (data->dataBlobIndex == 0)
    {
        if (param_count != 0)
            RET_ASSERT_ERR(RtErr::BadImageFormat);

        RET_ERR_ON_FAIL(Runtime::invoke_with_run_cctor(ctor_method, ca_obj, nullptr));
    }
    else
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, reader, mod->get_decoded_blob_reader(data->dataBlobIndex));

        uint16_t prolog = 0;
        if (!reader.try_read_u16(prolog))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        if (prolog != 0x0001)
            RET_ASSERT_ERR(RtErr::BadImageFormat);

        if (param_count == 0)
        {
            RET_ERR_ON_FAIL(Runtime::invoke_with_run_cctor(ctor_method, ca_obj, nullptr));
        }
        else
        {
            // TODO: Allocate fixed_arg_buf using ScopeFixedBuffer or similar
            // For now, use dynamic allocation
            int64_t* fixed_arg_buf = (int64_t*)alloca(sizeof(int64_t) * param_count);
            if (fixed_arg_buf == nullptr)
                RET_ERR(RtErr::OutOfMemory);
            const void** invoke_args = (const void**)alloca(sizeof(void*) * param_count);
            if (invoke_args == nullptr)
                RET_ERR(RtErr::OutOfMemory);

            for (uint32_t i = 0; i < param_count; ++i)
            {
                const metadata::RtTypeSig* param_type_sig = ctor_method->parameters[i];
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(FixedArg, fixed_arg, read_fixed_arg(mod, param_type_sig, &reader));
                fixed_arg_buf[i] = static_cast<int64_t>(fixed_arg.value);

                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_val_type, Type::is_value_type(param_type_sig));
                invoke_args[i] = is_val_type ? (const void*)&fixed_arg_buf[i] : (const void*)fixed_arg_buf[i];
            }
            RET_ERR_ON_FAIL(Runtime::invoke_with_run_cctor(ctor_method, ca_obj, invoke_args));
        }

        uint16_t named_arg_count = 0;
        if (!reader.try_read_u16(named_arg_count))
            RET_ASSERT_ERR(RtErr::BadImageFormat);

        for (uint16_t i = 0; i < named_arg_count; ++i)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(NamedArgWithoutValue, named_arg, read_named_arg(mod, &reader));

            if (named_arg.is_field)
            {
                const metadata::RtFieldInfo* field_info =
                    Class::get_field_for_name(klass, named_arg.name.data(), static_cast<uint32_t>(named_arg.name.size()), true);
                if (!field_info)
                    RET_ERR(RtErr::MissingField);

                const metadata::RtTypeSig* field_type_sig = field_info->type_sig;
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(FixedArg, value, read_fixed_arg(mod, field_type_sig, &reader));

                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_type_match, is_type_match_for_custom_attribute_elem(value.ele_type, field_type_sig));
                if (!is_type_match)
                    RET_ASSERT_ERR(RtErr::BadImageFormat);
                RET_ERR_ON_FAIL(Field::set_instance_value(field_info, ca_obj, &value.value));
            }
            else
            {
                const metadata::RtPropertyInfo* property_info =
                    Class::get_property_for_name(klass, named_arg.name.data(), static_cast<uint32_t>(named_arg.name.size()), true);
                if (!property_info)
                    RET_ERR(RtErr::MissingMember);

                const metadata::RtTypeSig* property_type_sig = property_info->property_sig.type_sig;
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(FixedArg, value, read_fixed_arg(mod, property_type_sig, &reader));
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_type_match, is_type_match_for_custom_attribute_elem(value.ele_type, property_type_sig));
                if (!is_type_match)
                    RET_ASSERT_ERR(RtErr::BadImageFormat);

                const metadata::RtMethodInfo* setter = property_info->set_method;
                if (!setter)
                    RET_ERR(RtErr::MissingMethod);

                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_val_type, Type::is_value_type(property_type_sig));
                const void* params[1] = {is_val_type ? &value.value : (const void*)value.value};

                RET_ERR_ON_FAIL(Runtime::invoke_with_run_cctor(setter, ca_obj, params));
            }
        }
    }

    return ca_obj;
}

static RtResult<RtObject*> new_custom_attribute_typed_argument(const metadata::RtMethodInfo* ctor, const metadata::RtTypeSig* param_type, const void* data)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, param_klass, Class::get_class_from_typesig(param_type));
    RET_ERR_ON_FAIL(Class::initialize_all(param_klass));
    const void* invoke_args[2];
    UNWRAP_OR_RET_ERR_ON_FAIL(invoke_args[0], Reflection::get_type_reflection_object(param_type));
    if (Class::is_value_type(param_klass))
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(invoke_args[1], Object::box_object(param_klass, data));
    }
    else
    {
        invoke_args[1] = *(const void**)data;
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, typed_arg_obj, Object::new_object(ctor->parent));
    RET_ERR_ON_FAIL(Runtime::invoke_with_run_cctor(ctor, typed_arg_obj, invoke_args));
    RET_OK(typed_arg_obj);
}

static RtResult<RtObject*> new_custom_attribute_named_argument(const metadata::RtMethodInfo* ctor, RtObject* member_info, RtObject* typed_arg)
{
    const void* invoke_args[2] = {member_info, typed_arg + 1}; // typed_arg + 1 to skip the RtObject header
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, named_arg_obj, Object::new_object(ctor->parent));
    RET_ERR_ON_FAIL(Runtime::invoke_with_run_cctor(ctor, named_arg_obj, invoke_args));
    RET_OK(named_arg_obj);
}

RtResultVoid CustomAttribute::resolve_customattribute_data_arguments(utils::BinaryReader* reader, metadata::RtModuleDef* mod,
                                                                     const metadata::RtMethodInfo* ctor_method, RtArray** typed_arg_arr_ptr,
                                                                     RtArray** named_arg_arr_ptr)
{
    const metadata::RtClass* klass = ctor_method->parent;
    uint16_t prolog = 0;
    if (!reader->try_read_u16(prolog))
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    if (prolog != 0x0001)
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    const auto& corlib_types = Class::get_corlib_types();

    const metadata::RtTypeSig* ctor_param_type_sigs[] = {
        corlib_types.cls_systemtype->by_val,
        corlib_types.cls_object->by_val,
    };
    const metadata::RtMethodInfo* typed_arg_ctor =
        Method::find_matched_method_in_class_by_name_and_signature(corlib_types.cls_customattribute_typed_argument, STR_CTOR, ctor_param_type_sigs, 2);
    if (!typed_arg_ctor)
        RET_ERR(RtErr::MissingMethod);

    const metadata::RtTypeSig* named_arg_param_type_sigs[] = {
        corlib_types.cls_reflection_memberinfo->by_val,
        corlib_types.cls_customattribute_typed_argument->by_val,
    };
    const metadata::RtMethodInfo* named_arg_ctor =
        Method::find_matched_method_in_class_by_name_and_signature(corlib_types.cls_customattribute_named_argument, STR_CTOR, named_arg_param_type_sigs, 2);
    if (!named_arg_ctor)
        RET_ERR(RtErr::MissingMethod);

    uint32_t param_count = ctor_method->parameter_count;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, typed_arg_arr, Array::new_szarray_from_ele_klass(corlib_types.cls_object, (int32_t)param_count));
    *typed_arg_arr_ptr = typed_arg_arr;

    for (uint32_t i = 0; i < param_count; ++i)
    {
        const metadata::RtTypeSig* param_type_sig = ctor_method->parameters[i];
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(FixedArg, fixed_arg, read_fixed_arg(mod, param_type_sig, reader));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, typed_arg_obj,
                                                new_custom_attribute_typed_argument(typed_arg_ctor, param_type_sig, &fixed_arg.value));
        // gc::GarbageCollector::write_barrier((RtObject**)&typed_arg_obj, typed_arg_obj);
        Array::set_array_data_at(typed_arg_arr, static_cast<int32_t>(i), typed_arg_obj);
    }

    uint16_t named_arg_count = 0;
    if (!reader->try_read_u16(named_arg_count))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, named_arg_arr, Array::new_szarray_from_ele_klass(corlib_types.cls_object, (int32_t)named_arg_count));
    *named_arg_arr_ptr = named_arg_arr;

    for (uint16_t i = 0; i < named_arg_count; ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(NamedArgWithoutValue, named_arg, read_named_arg(mod, reader));

        RtObject* typed_arg_obj = nullptr;
        RtObject* member_info_obj = nullptr;
        if (named_arg.is_field)
        {
            const metadata::RtFieldInfo* field_info =
                Class::get_field_for_name(klass, named_arg.name.data(), static_cast<uint32_t>(named_arg.name.size()), true);
            if (!field_info)
                RET_ERR(RtErr::MissingField);

            const metadata::RtTypeSig* field_type_sig = field_info->type_sig;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(FixedArg, value, read_fixed_arg(mod, field_type_sig, reader));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_type_match, is_type_match_for_custom_attribute_elem(value.ele_type, field_type_sig));
            if (!is_type_match)
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionField*, ref_field, Reflection::get_field_reflection_object(field_info, field_info->parent));
            member_info_obj = (RtObject*)ref_field;
            UNWRAP_OR_RET_ERR_ON_FAIL(typed_arg_obj, new_custom_attribute_typed_argument(typed_arg_ctor, field_type_sig, &value.value));
        }
        else
        {
            const metadata::RtPropertyInfo* property_info =
                Class::get_property_for_name(klass, named_arg.name.data(), static_cast<uint32_t>(named_arg.name.size()), true);
            if (!property_info)
                RET_ERR(RtErr::MissingMember);
            const metadata::RtTypeSig* property_type_sig = property_info->property_sig.type_sig;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(FixedArg, value, read_fixed_arg(mod, property_type_sig, reader));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_type_match, is_type_match_for_custom_attribute_elem(value.ele_type, property_type_sig));
            if (!is_type_match)
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionProperty*, prop_info_obj,
                                                    Reflection::get_property_reflection_object(property_info, property_info->parent));
            member_info_obj = (RtObject*)prop_info_obj;
            UNWRAP_OR_RET_ERR_ON_FAIL(typed_arg_obj, new_custom_attribute_typed_argument(typed_arg_ctor, property_type_sig, &value.value));
        }
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, named_arg_obj, new_custom_attribute_named_argument(named_arg_ctor, member_info_obj, typed_arg_obj));
        // gc::GarbageCollector::write_barrier((RtObject**)&named_arg_obj, member_info_obj);
        Array::set_array_data_at(named_arg_arr, i, named_arg_obj);
    }

    RET_VOID_OK();
}

RtResult<bool> CustomAttribute::has_customattribute_on_target(metadata::RtModuleDef* mod, metadata::EncodedTokenId target_token,
                                                              const metadata::RtClass* attr_klass)
{
    if (target_token == 0)
        RET_OK(false);

    RET_ERR_ON_FAIL(Class::initialize_super_types(const_cast<metadata::RtClass*>(attr_klass)));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(metadata::RtCustomAttributeRidRange, rid_range, mod->get_custom_attribute_rid_range(target_token));

    for (uint32_t i = 0; i < rid_range.count; ++i)
    {
        uint32_t ca_rid = rid_range.start_rid + i;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtCustomAttributeRawData, raw_data, mod->get_custom_attribute_raw_data(ca_rid));

        const metadata::RtClass* data_klass = raw_data.ctor->parent;
        RET_ERR_ON_FAIL(Class::initialize_super_types(const_cast<metadata::RtClass*>(data_klass)));

        if (Class::has_class_parent_fast(data_klass, attr_klass))
            RET_OK(true);
    }

    RET_OK(false);
}

RtResult<bool> CustomAttribute::has_customattribute_on_field(const metadata::RtFieldInfo* field, const metadata::RtClass* attr_klass)
{
    metadata::RtModuleDef* mod = field->parent->image;
    return has_customattribute_on_target(mod, field->token, attr_klass);
}

RtResult<bool> CustomAttribute::has_customattribute_on_method(const metadata::RtMethodInfo* method, const metadata::RtClass* customattribute_klass)
{
    metadata::RtModuleDef* mod = method->parent->image;
    return has_customattribute_on_target(mod, method->token, customattribute_klass);
}

RtResult<bool> CustomAttribute::has_customattribute_on_class(const metadata::RtClass* klass, const metadata::RtClass* customattribute_klass)
{
    metadata::RtModuleDef* mod = klass->image;
    return has_customattribute_on_target(mod, klass->token, customattribute_klass);
}

RtResult<bool> CustomAttribute::has_customattribute_on_property(const metadata::RtPropertyInfo* property, const metadata::RtClass* customattribute_klass)
{
    metadata::RtModuleDef* mod = property->parent->image;
    return has_customattribute_on_target(mod, property->token, customattribute_klass);
}

RtResult<bool> CustomAttribute::has_customattribute_on_event(const metadata::RtEventInfo* event, const metadata::RtClass* customattribute_klass)
{
    metadata::RtModuleDef* mod = event->parent->image;
    return has_customattribute_on_target(mod, event->token, customattribute_klass);
}

RtResult<bool> CustomAttribute::has_customattribute_on_parameter(RtReflectionParameter* parameter, const metadata::RtClass* customattribute_klass)
{
    auto ref_method = reinterpret_cast<RtReflectionMethod*>(parameter->member);
    const metadata::RtMethodInfo* method = ref_method->method;
    metadata::RtModuleDef* mod = method->parent->image;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<uint32_t>, opt_param_token, Method::get_parameter_token(method, parameter->index));
    if (!opt_param_token)
        RET_OK(false);
    uint32_t param_token = opt_param_token.value();
    return has_customattribute_on_target(mod, param_token, customattribute_klass);
}

RtResult<bool> CustomAttribute::has_customattribute_on_assembly(metadata::RtModuleDef* mod, const metadata::RtClass* customattribute_klass)
{
    uint32_t ass_token = mod->get_assembly_token();
    return has_customattribute_on_target(mod, ass_token, customattribute_klass);
}

static RtResult<CustomAttributeProvider> get_token_of_customattribute_provider(RtObject* obj)
{
    if (obj == nullptr)
        RET_ERR(RtErr::NullReference);

    const CorLibTypes& corlib_types = Class::get_corlib_types();
    const metadata::RtClass* obj_klass = obj->klass;

    CustomAttributeProvider provider{};

    if (obj_klass == corlib_types.cls_runtimetype)
    {
        RtReflectionType* type_obj = reinterpret_cast<RtReflectionType*>(obj);
        const metadata::RtTypeSig* type_sig = type_obj->type_handle;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, target_klass, Class::get_class_from_typesig(type_sig));
        metadata::RtModuleDef* mod = target_klass->image;
        provider.mod = mod;
        provider.token = target_klass->token;
    }
    else if (obj_klass == corlib_types.cls_reflection_method || obj_klass == corlib_types.cls_reflection_constructor)
    {
        RtReflectionMethod* method_obj = reinterpret_cast<RtReflectionMethod*>(obj);
        const metadata::RtMethodInfo* method = method_obj->method;
        metadata::RtModuleDef* mod = method->parent->image;
        provider.mod = mod;
        provider.token = method->token;
    }
    else if (obj_klass == corlib_types.cls_reflection_field)
    {
        RtReflectionField* field_obj = reinterpret_cast<RtReflectionField*>(obj);
        const metadata::RtFieldInfo* field = field_obj->field;
        metadata::RtModuleDef* mod = field->parent->image;
        provider.mod = mod;
        provider.token = field->token;
    }
    else if (obj_klass == corlib_types.cls_reflection_property)
    {
        RtReflectionProperty* prop_obj = reinterpret_cast<RtReflectionProperty*>(obj);
        const metadata::RtPropertyInfo* property = prop_obj->property;
        metadata::RtModuleDef* mod = property->parent->image;
        provider.mod = mod;
        provider.token = property->token;
    }
    else if (obj_klass == corlib_types.cls_reflection_event)
    {
        RtReflectionEventInfo* event_obj = reinterpret_cast<RtReflectionEventInfo*>(obj);
        const metadata::RtEventInfo* event = event_obj->event;
        metadata::RtModuleDef* mod = event->parent->image;
        provider.mod = mod;
        provider.token = event->token;
    }
    else if (obj_klass == corlib_types.cls_reflection_parameter)
    {
        RtReflectionParameter* param_obj = reinterpret_cast<RtReflectionParameter*>(obj);
        RtReflectionMethod* ref_method = reinterpret_cast<RtReflectionMethod*>(param_obj->member);
        const metadata::RtMethodInfo* method = ref_method->method;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<uint32_t>, opt_param_token, Method::get_parameter_token(method, param_obj->index));
        uint32_t param_token = opt_param_token.value_or(0);
        metadata::RtModuleDef* mod = method->parent->image;
        provider.mod = mod;
        provider.token = param_token;
    }
    else if (obj_klass == corlib_types.cls_reflection_assembly)
    {
        RtReflectionAssembly* assembly_obj = reinterpret_cast<RtReflectionAssembly*>(obj);
        metadata::RtAssembly* assembly = assembly_obj->assembly;
        metadata::RtModuleDef* mod = assembly->mod;
        provider.mod = mod;
        provider.token = mod->get_assembly_token();
    }
    else
    {
        metadata::RtModuleDef* mod = obj_klass->image;
        provider.mod = mod;
        provider.token = obj_klass->token;
    }

    RET_OK(provider);
}

RtResult<bool> CustomAttribute::has_attribute(RtObject* obj, const metadata::RtClass* attr_klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(CustomAttributeProvider, provider, get_token_of_customattribute_provider(obj));
    return has_customattribute_on_target(provider.mod, provider.token, attr_klass);
}

RtResult<RtArray*> CustomAttribute::get_customattributes_on_target_token(metadata::RtModuleDef* mod, metadata::EncodedTokenId target_token,
                                                                         const metadata::RtClass* attr_klass)
{
    const CorLibTypes& types = Class::get_corlib_types();

    if (target_token == 0)
    {
        return Array::new_empty_szarray_by_ele_klass(types.cls_attribute);
    }

    if (attr_klass)
    {
        RET_ERR_ON_FAIL(Class::initialize_super_types(const_cast<metadata::RtClass*>(attr_klass)));
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(metadata::RtCustomAttributeRidRange, rid_range, mod->get_custom_attribute_rid_range(target_token));

    utils::Vector<RtObject*> ca_buf;
    ca_buf.reserve(rid_range.count);

    for (uint32_t i = 0; i < rid_range.count; ++i)
    {
        uint32_t ca_rid = rid_range.start_rid + i;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtCustomAttributeRawData, raw_data, mod->get_custom_attribute_raw_data(ca_rid));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ca, read_custom_attribute(mod, &raw_data));

        if (attr_klass && !Object::is_inst(ca, attr_klass))
        {
            continue;
        }

        ca_buf.push_back(ca);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, ca_arr, Array::new_szarray_from_ele_klass(types.cls_attribute, (int32_t)ca_buf.size()));

    for (size_t i = 0; i < ca_buf.size(); ++i)
    {
        // gc::GarbageCollector::write_barrier((RtObject**)Array::get_array_data_start_as<RtObject*>(ca_arr) + i, ca_buf[i]);
        Array::set_array_data_at<RtObject*>(ca_arr, (int32_t)i, ca_buf[i]);
    }

    return ca_arr;
}

RtResult<RtArray*> CustomAttribute::get_customattributes_on_target_object(RtObject* obj, const metadata::RtClass* attr_klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(CustomAttributeProvider, provider, get_token_of_customattribute_provider(obj));
    return get_customattributes_on_target_token(provider.mod, provider.token, attr_klass);
}

RtResult<RtArray*> CustomAttribute::get_customattributes_data_on_target(RtObject* obj)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(CustomAttributeProvider, provider, get_token_of_customattribute_provider(obj));
    return get_customattributes_data_on_target_token(provider.mod, provider.token);
}

static const metadata::RtMethodInfo* s_customattribute_data_ctor = nullptr;

RtResult<const metadata::RtMethodInfo*> get_customattribute_data_ctor()
{
    if (s_customattribute_data_ctor == nullptr)
    {
        const CorLibTypes& corlib_types = Class::get_corlib_types();
        const metadata::RtTypeSig* param_type_sigs[] = {
            corlib_types.cls_runtimetype->by_val,
            corlib_types.cls_array->by_val,
        };
        const metadata::RtMethodInfo* ctor = Method::find_matched_method_in_class_by_name_and_param_count(corlib_types.cls_customattributedata, STR_CTOR, 4);
        if (!ctor)
            RET_ERR(RtErr::MissingMethod);
        s_customattribute_data_ctor = ctor;
    }
    RET_OK(s_customattribute_data_ctor);
}

RtResult<RtArray*> CustomAttribute::get_customattributes_data_on_target_token(metadata::RtModuleDef* mod, metadata::EncodedTokenId target_token)
{
    const CorLibTypes& types = Class::get_corlib_types();

    if (target_token == 0)
    {
        return Array::new_empty_szarray_by_ele_klass(types.cls_customattributedata);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(metadata::RtCustomAttributeRidRange, rid_range, mod->get_custom_attribute_rid_range(target_token));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(const metadata::RtMethodInfo*, ca_data_ctor, get_customattribute_data_ctor());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(RtArray*, ca_data_arr, Array::new_szarray_from_ele_klass(types.cls_customattributedata, (int32_t)rid_range.count));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionAssembly*, assembly_obj, Reflection::get_assembly_reflection_object(mod->get_assembly()));
    for (uint32_t i = 0; i < rid_range.count; ++i)
    {
        uint32_t ca_rid = rid_range.start_rid + i;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtCustomAttributeRawData, raw_data, mod->get_custom_attribute_raw_data(ca_rid));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ca, read_custom_attribute(mod, &raw_data));

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, reader, mod->get_decoded_blob_reader(raw_data.dataBlobIndex));
        RtArray* typed_arg_arr = nullptr;
        RtArray* named_arg_arr = nullptr;
        RET_ERR_ON_FAIL(resolve_customattribute_data_arguments(&reader, mod, raw_data.ctor, &typed_arg_arr, &named_arg_arr));

        const void* ctor_args[4];
        UNWRAP_OR_RET_ERR_ON_FAIL(ctor_args[0], Reflection::get_method_reflection_object(raw_data.ctor, raw_data.ctor->parent));
        const void* data_ptr = reader.data();
        size_t data_len = reader.length();
        ctor_args[1] = assembly_obj;
        ctor_args[2] = &data_ptr;
        ctor_args[3] = &data_len;

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ca_data_obj, Object::new_object(ca_data_ctor->parent));
        RET_ERR_ON_FAIL(Runtime::invoke_with_run_cctor(ca_data_ctor, ca_data_obj, ctor_args));

        // gc::GarbageCollector::write_barrier((RtObject**)Array::get_array_data_start_as<RtObject*>(ca_data_arr) + i, ca_data_obj);
        Array::set_array_data_at<RtObject*>(ca_data_arr, (int32_t)i, ca_data_obj);
    }
    RET_OK(ca_data_arr);
}

static const metadata::RtMethodInfo* s_marshal_as_ctor = nullptr;
static const metadata::RtFieldInfo* s_marshal_as_size_const_field = nullptr;
static const metadata::RtFieldInfo* s_marshal_as_array_sub_type_field = nullptr;
static const metadata::RtFieldInfo* s_marshal_as_size_param_index_field = nullptr;

static void init_marshal_as_fields()
{
    if (s_marshal_as_ctor)
    {
        return;
    }
    const CorLibTypes& corlib_types = Class::get_corlib_types();
    const metadata::RtClass* marshal_as_klass = corlib_types.cls_marshal_as;
    const metadata::RtTypeSig* param_type_sigs[] = {
        corlib_types.cls_int16->by_val,
    };
    s_marshal_as_ctor = Method::find_matched_method_in_class_by_name_and_signature(marshal_as_klass, STR_CTOR, param_type_sigs, 1);
    assert(s_marshal_as_ctor);
    s_marshal_as_size_const_field = Class::get_field_for_name(marshal_as_klass, "SizeConst", false);
    assert(s_marshal_as_size_const_field);
    s_marshal_as_array_sub_type_field = Class::get_field_for_name(marshal_as_klass, "ArraySubType", false);
    assert(s_marshal_as_array_sub_type_field);
    s_marshal_as_size_param_index_field = Class::get_field_for_name(marshal_as_klass, "SizeParamIndex", false);
    assert(s_marshal_as_size_param_index_field);
}

RtResult<RtCustomAttribute*> CustomAttribute::get_marshal_info(const metadata::RtFieldInfo* field)
{
    if (!vm::Field::has_field_marshal(field))
    {
        RET_OK(nullptr);
    }

    metadata::RtModuleDef* mod = field->parent->image;
    const metadata::CliImage& cli_image = mod->get_cli_image();
    uint32_t field_rid = metadata::RtToken::decode_rid(field->token);
    uint32_t fieldmarshal_parent = metadata::RtMetadata::encode_has_field_marshal_coded_index(metadata::TableType::Field, field_rid);
    std::optional<uint32_t> marshal_rid = cli_image.find_row_of_owner(metadata::TableType::FieldMarshal, 0, fieldmarshal_parent);
    if (!marshal_rid)
    {
        // impossible for valid dll file.
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    std::optional<metadata::RowFieldMarshal> marshal_row = cli_image.read_field_marshal(marshal_rid.value());
    assert(marshal_row.has_value());

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, reader, mod->get_decoded_blob_reader(marshal_row->native_type));
    uint8_t native_type = 0;
    if (!reader.try_read_byte(native_type))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    init_marshal_as_fields();
    const CorLibTypes& corlib_types = Class::get_corlib_types();

    assert(sizeof(metadata::RtMarshalNativeType) == sizeof(int32_t));
    metadata::RtMarshalNativeType native_intrinsic_type = static_cast<metadata::RtMarshalNativeType>(native_type);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, marshal_as_obj, Object::new_object(corlib_types.cls_marshal_as));
    const void* ctor_args[1] = {&native_intrinsic_type};
    RET_ERR_ON_FAIL(Runtime::invoke_with_run_cctor(s_marshal_as_ctor, marshal_as_obj, ctor_args));

    metadata::RtMarshalNativeType ele_type = metadata::RtMarshalNativeType::Max;
    uint32_t param_num = 0;
    uint32_t num_elems = 0;
    switch (native_intrinsic_type)
    {
    case metadata::RtMarshalNativeType::Array:
    {
        uint8_t ele_type_byte = 0;
        if (!reader.try_read_byte(ele_type_byte))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        ele_type = static_cast<metadata::RtMarshalNativeType>(ele_type_byte);
        vm::Field::set_instance_value(s_marshal_as_array_sub_type_field, marshal_as_obj, &ele_type);
        if (reader.not_empty())
        {
            if (!reader.try_read_compressed_uint32(param_num))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            vm::Field::set_instance_value(s_marshal_as_size_param_index_field, marshal_as_obj, &param_num);
        }
        if (reader.not_empty())
        {
            if (!reader.try_read_compressed_uint32(num_elems))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            vm::Field::set_instance_value(s_marshal_as_size_const_field, marshal_as_obj, &num_elems);
        }
        break;
    }
    default:
        break;
    }
    if (reader.not_empty())
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RET_OK((RtCustomAttribute*)marshal_as_obj);
}

} // namespace vm
} // namespace leanclr

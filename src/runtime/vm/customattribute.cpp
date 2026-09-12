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
#include "marshal.h"
#include "rt_string.h"
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

static RtResult<metadata::RtClass*> get_class_from_reflection_type(RtReflectionType* type_obj)
{
    return Reflection::get_class_from_reflection_type_object(type_obj);
}

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
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(corlib_types.cls_boolean, &raw_data, "customattribute::read_customattribute_elem_value"));
            break;
        }
        case metadata::RtElementType::Char:
        {
            uint16_t raw_data = 0;
            if (!reader->try_read_u16(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(corlib_types.cls_char, &raw_data, "customattribute::read_customattribute_elem_value"));
            break;
        }
        case metadata::RtElementType::I1:
        {
            int8_t raw_data = 0;
            if (!reader->try_read_any(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(corlib_types.cls_sbyte, &raw_data, "customattribute::read_customattribute_elem_value"));
            break;
        }
        case metadata::RtElementType::U1:
        {
            uint8_t raw_data = 0;
            if (!reader->try_read_byte(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(corlib_types.cls_byte, &raw_data, "customattribute::read_customattribute_elem_value"));
            break;
        }
        case metadata::RtElementType::I2:
        {
            int16_t raw_data = 0;
            if (!reader->try_read_any(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(corlib_types.cls_int16, &raw_data, "customattribute::read_customattribute_elem_value"));
            break;
        }
        case metadata::RtElementType::U2:
        {
            uint16_t raw_data = 0;
            if (!reader->try_read_u16(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(corlib_types.cls_uint16, &raw_data, "customattribute::read_customattribute_elem_value"));
            break;
        }
        case metadata::RtElementType::I4:
        {
            int32_t raw_data = 0;
            if (!reader->try_read_any(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(corlib_types.cls_int32, &raw_data, "customattribute::read_customattribute_elem_value"));
            break;
        }
        case metadata::RtElementType::U4:
        {
            uint32_t raw_data = 0;
            if (!reader->try_read_u32(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(corlib_types.cls_uint32, &raw_data, "customattribute::read_customattribute_elem_value"));
            break;
        }
        case metadata::RtElementType::I8:
        {
            int64_t raw_data = 0;
            if (!reader->try_read_any(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(corlib_types.cls_int64, &raw_data, "customattribute::read_customattribute_elem_value"));
            break;
        }
        case metadata::RtElementType::U8:
        {
            uint64_t raw_data = 0;
            if (!reader->try_read_u64(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(corlib_types.cls_uint64, &raw_data, "customattribute::read_customattribute_elem_value"));
            break;
        }
        case metadata::RtElementType::R4:
        {
            float raw_data = 0.0f;
            if (!reader->try_read_f32(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(corlib_types.cls_single, &raw_data, "customattribute::read_customattribute_elem_value"));
            break;
        }
        case metadata::RtElementType::R8:
        {
            double raw_data = 0.0;
            if (!reader->try_read_f64(raw_data))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(corlib_types.cls_double, &raw_data, "customattribute::read_customattribute_elem_value"));
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
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, enum_klass, get_class_from_reflection_type(type_obj));
            if (!Class::is_enum_type(enum_klass))
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint64_t, enum_value,
                                                    read_customattribute_elem_simple_value(reader, Class::get_element_type(enum_klass->element_class)));
            UNWRAP_OR_RET_ERR_ON_FAIL(obj, LEANCLR_BOX_OBJECT_INTERNAL(enum_klass, &enum_value, "customattribute::read_customattribute_elem_value"));
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
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, elem_arr, LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(ele_klass, static_cast<int32_t>(num_elems), "customattribute::read_customattribute_elem_value"));
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
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, elem_arr, LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(ele_klass, static_cast<int32_t>(num_elems), "customattribute::read_customattribute_elem_value"));

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
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, enum_klass, get_class_from_reflection_type(type_obj));
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

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ca_obj, LEANCLR_NEWOBJ_INTERNAL(klass, "CustomAttribute::read_custom_attribute"));

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
        UNWRAP_OR_RET_ERR_ON_FAIL(invoke_args[1], LEANCLR_BOX_OBJECT_INTERNAL(param_klass, data, "CustomAttribute::new_custom_attribute_typed_argument"));
    }
    else
    {
        invoke_args[1] = *(const void**)data;
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, typed_arg_obj, LEANCLR_NEWOBJ_INTERNAL(ctor->parent, "CustomAttribute::new_custom_attribute_typed_argument"));
    RET_ERR_ON_FAIL(Runtime::invoke_with_run_cctor(ctor, typed_arg_obj, invoke_args));
    RET_OK(typed_arg_obj);
}

static RtResult<RtObject*> new_custom_attribute_named_argument(const metadata::RtMethodInfo* ctor, RtObject* member_info, RtObject* typed_arg)
{
    const void* invoke_args[2] = {member_info, typed_arg + 1}; // typed_arg + 1 to skip the RtObject header
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, named_arg_obj, LEANCLR_NEWOBJ_INTERNAL(ctor->parent, "CustomAttribute::new_custom_attribute_typed_argument"));
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
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, typed_arg_arr, LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(corlib_types.cls_object, (int32_t)param_count, "CustomAttribute::resolve_customattribute_data_arguments"));
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

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, named_arg_arr, LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(corlib_types.cls_object, (int32_t)named_arg_count, "CustomAttribute::resolve_customattribute_data_arguments"));
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
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method,
                                            Reflection::get_parameter_method_info_from_reflection_object(parameter));
    metadata::RtModuleDef* mod = method->parent->image;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<uint32_t>, opt_param_token,
                                            Reflection::get_parameter_token_from_reflection_object(parameter));
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
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, provider_obj, Reflection::normalize_coreclr_reflection_object(obj));
    const metadata::RtClass* obj_klass = provider_obj->klass;

    CustomAttributeProvider provider{};

    if (obj_klass == corlib_types.cls_runtimetype)
    {
        RtReflectionType* type_obj = reinterpret_cast<RtReflectionType*>(provider_obj);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, target_klass,
                                                Reflection::get_class_from_reflection_type_object(type_obj));
        metadata::RtModuleDef* mod = target_klass->image;
        provider.mod = mod;
        provider.token = target_klass->token;
    }
    else if (obj_klass == corlib_types.cls_reflection_method || obj_klass == corlib_types.cls_reflection_constructor)
    {
        RtReflectionMethod* method_obj = reinterpret_cast<RtReflectionMethod*>(provider_obj);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, Reflection::get_method_info_from_reflection_object(method_obj));
        metadata::RtModuleDef* mod = method->parent->image;
        provider.mod = mod;
        provider.token = method->token;
    }
    else if (obj_klass == corlib_types.cls_reflection_field)
    {
        RtReflectionField* field_obj = reinterpret_cast<RtReflectionField*>(provider_obj);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field, Reflection::get_field_info_from_reflection_object(field_obj));
        metadata::RtModuleDef* mod = field->parent->image;
        provider.mod = mod;
        provider.token = field->token;
    }
    else if (obj_klass == corlib_types.cls_reflection_property)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtPropertyInfo*, property,
                                                Reflection::get_property_info_from_reflection_object(
                                                    reinterpret_cast<RtReflectionProperty*>(provider_obj)));
        metadata::RtModuleDef* mod = property->parent->image;
        provider.mod = mod;
        provider.token = property->token;
    }
    else if (obj_klass == corlib_types.cls_reflection_event)
    {
        RtReflectionEventInfo* event_obj = reinterpret_cast<RtReflectionEventInfo*>(provider_obj);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtEventInfo*, event,
                                                Reflection::get_event_info_from_reflection_object(event_obj));
        metadata::RtModuleDef* mod = event->parent->image;
        provider.mod = mod;
        provider.token = event->token;
    }
    else if (obj_klass == corlib_types.cls_reflection_parameter)
    {
        RtReflectionParameter* param_obj = reinterpret_cast<RtReflectionParameter*>(provider_obj);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method,
                                                Reflection::get_parameter_method_info_from_reflection_object(param_obj));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<uint32_t>, opt_param_token,
                                                Reflection::get_parameter_token_from_reflection_object(param_obj));
        uint32_t param_token = opt_param_token.value_or(0);
        metadata::RtModuleDef* mod = method->parent->image;
        provider.mod = mod;
        provider.token = param_token;
    }
    else if (obj_klass == corlib_types.cls_reflection_assembly)
    {
        RtReflectionAssembly* assembly_obj = reinterpret_cast<RtReflectionAssembly*>(provider_obj);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtAssembly*, assembly,
                                                Reflection::get_assembly_from_reflection_object(assembly_obj));
        metadata::RtModuleDef* mod = assembly->mod;
        provider.mod = mod;
        provider.token = mod->get_assembly_token();
    }
    else if (obj_klass == corlib_types.cls_reflection_module)
    {
        RtReflectionModule* module_obj = reinterpret_cast<RtReflectionModule*>(provider_obj);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtModuleDef*, mod,
                                                Reflection::get_module_from_reflection_object(module_obj));
        provider.mod = mod;
        provider.token = mod->get_module_token();
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
        return LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS_INTERNAL(types.cls_attribute, "CustomAttribute::get_customattributes_on_target_token");
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

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, ca_arr, LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(types.cls_attribute, (int32_t)ca_buf.size(), "CustomAttribute::get_customattributes_on_target_token"));

    for (size_t i = 0; i < ca_buf.size(); ++i)
    {
        // gc::GarbageCollector::write_barrier((RtObject**)Array::get_array_data_start_as<RtObject*>(ca_arr) + i, ca_buf[i]);
        Array::set_array_data_at<RtObject*>(ca_arr, (int32_t)i, ca_buf[i]);
    }

    return ca_arr;
}

static bool has_struct_layout_pseudo_attribute(const metadata::RtClass* klass)
{
    uint32_t layout_flags = klass->flags & (static_cast<uint32_t>(metadata::RtTypeAttribute::SequentialLayout) |
                                            static_cast<uint32_t>(metadata::RtTypeAttribute::ExplicitLayout));
    uint32_t string_format = klass->flags & static_cast<uint32_t>(metadata::RtTypeAttribute::StringFormatMask);
    return layout_flags != 0 || string_format != 0 || klass->image->get_class_layout_data(klass->token).has_value();
}

static RtResult<RtObject*> create_struct_layout_pseudo_attribute(const metadata::RtClass* klass)
{
    metadata::RtModuleDef* corlib = Class::get_corlib_types().cls_object->image;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, attr_klass,
                                            corlib->get_class_by_name("System.Runtime.InteropServices.StructLayoutAttribute", false, true));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, layout_kind_klass,
                                            corlib->get_class_by_name("System.Runtime.InteropServices.LayoutKind", false, true));

    const metadata::RtTypeSig* ctor_params[] = {layout_kind_klass->by_val};
    const metadata::RtMethodInfo* ctor =
        Method::find_matched_method_in_class_by_name_and_signature(attr_klass, STR_CTOR, ctor_params, 1);
    if (ctor == nullptr)
    {
        RET_ERR(RtErr::MissingMethod);
    }

    int32_t layout_kind = 3; // LayoutKind.Auto
    if ((klass->flags & static_cast<uint32_t>(metadata::RtTypeAttribute::SequentialLayout)) != 0)
    {
        layout_kind = 0; // LayoutKind.Sequential
    }
    else if ((klass->flags & static_cast<uint32_t>(metadata::RtTypeAttribute::ExplicitLayout)) != 0)
    {
        layout_kind = 2; // LayoutKind.Explicit
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, attr_obj,
                                            LEANCLR_NEWOBJ_INTERNAL(attr_klass, "CustomAttribute::create_struct_layout_pseudo_attribute"));
    const void* ctor_args[] = {&layout_kind};
    RET_ERR_ON_FAIL(Runtime::invoke_with_run_cctor(ctor, attr_obj, ctor_args));

    int32_t pack = 0;
    int32_t size = 0;
    auto layout_data = klass->image->get_class_layout_data(klass->token);
    if (layout_data.has_value())
    {
        pack = static_cast<int32_t>(layout_data->packing);
        size = static_cast<int32_t>(layout_data->size);
    }

    int32_t char_set = 2; // CharSet.Ansi
    uint32_t string_format = klass->flags & static_cast<uint32_t>(metadata::RtTypeAttribute::StringFormatMask);
    if (string_format == static_cast<uint32_t>(metadata::RtTypeAttribute::UnicodeClass))
    {
        char_set = 3; // CharSet.Unicode
    }
    else if (string_format == static_cast<uint32_t>(metadata::RtTypeAttribute::AutoClass))
    {
        char_set = 4; // CharSet.Auto
    }

    const metadata::RtFieldInfo* pack_field = Class::get_field_for_name(attr_klass, "Pack", true);
    const metadata::RtFieldInfo* size_field = Class::get_field_for_name(attr_klass, "Size", true);
    const metadata::RtFieldInfo* char_set_field = Class::get_field_for_name(attr_klass, "CharSet", true);
    if (pack_field == nullptr || size_field == nullptr || char_set_field == nullptr)
    {
        RET_ERR(RtErr::MissingField);
    }
    RET_ERR_ON_FAIL(Field::set_instance_value(pack_field, attr_obj, &pack));
    RET_ERR_ON_FAIL(Field::set_instance_value(size_field, attr_obj, &size));
    RET_ERR_ON_FAIL(Field::set_instance_value(char_set_field, attr_obj, &char_set));
    RET_OK(attr_obj);
}

static RtResult<RtArray*> append_custom_attribute(RtArray* normal_attrs, RtObject* pseudo_attr)
{
    const CorLibTypes& types = Class::get_corlib_types();
    int32_t normal_count = normal_attrs != nullptr ? Array::get_array_length(normal_attrs) : 0;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, result,
                                            LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(types.cls_attribute, normal_count + 1,
                                                                                       "CustomAttribute::append_custom_attribute"));
    for (int32_t i = 0; i < normal_count; ++i)
    {
        Array::set_array_data_at<RtObject*>(result, i, Array::get_array_data_at<RtObject*>(normal_attrs, i));
    }
    Array::set_array_data_at<RtObject*>(result, normal_count, pseudo_attr);
    RET_OK(result);
}

RtResult<RtArray*> CustomAttribute::get_customattributes_on_class_with_pseudo(const metadata::RtClass* klass,
                                                                              const metadata::RtClass* attr_klass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, normal_attrs,
                                            get_customattributes_on_target_token(klass->image, klass->token, attr_klass));
    if (attr_klass == nullptr || !has_struct_layout_pseudo_attribute(klass))
    {
        RET_OK(normal_attrs);
    }

    metadata::RtModuleDef* corlib = Class::get_corlib_types().cls_object->image;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, struct_layout_klass,
                                            corlib->get_class_by_name("System.Runtime.InteropServices.StructLayoutAttribute", false, true));
    RET_ERR_ON_FAIL(Class::initialize_super_types(const_cast<metadata::RtClass*>(struct_layout_klass)));
    RET_ERR_ON_FAIL(Class::initialize_super_types(const_cast<metadata::RtClass*>(attr_klass)));
    if (!Class::has_class_parent_fast(struct_layout_klass, attr_klass))
    {
        RET_OK(normal_attrs);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, pseudo_attr, create_struct_layout_pseudo_attribute(klass));
    return append_custom_attribute(normal_attrs, pseudo_attr);
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
static const metadata::RtClass* s_runtime_customattribute_data_class = nullptr;
static const metadata::RtClass* s_metadata_token_class = nullptr;
static const metadata::RtClass* s_const_array_class = nullptr;

struct RuntimeMetadataConstArray
{
    int32_t length;
    intptr_t data;
};

RtResult<const metadata::RtMethodInfo*> get_customattribute_data_ctor()
{
    if (s_customattribute_data_ctor == nullptr)
    {
        const CorLibTypes& corlib_types = Class::get_corlib_types();
        metadata::RtModuleDef* corlib = corlib_types.cls_customattributedata->image;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, runtime_customattribute_data_class,
                                                corlib->get_class_by_name("System.Reflection.RuntimeCustomAttributeData", false, true));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, metadata_token_class,
                                                corlib->get_class_by_name("System.Reflection.MetadataToken", false, true));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, const_array_class,
                                                corlib->get_class_by_name("System.Reflection.ConstArray", false, true));

        RET_ERR_ON_FAIL(Class::initialize_all(runtime_customattribute_data_class));
        RET_ERR_ON_FAIL(Class::initialize_all(metadata_token_class));
        RET_ERR_ON_FAIL(Class::initialize_all(const_array_class));

        const metadata::RtTypeSig* param_type_sigs[] = {
            corlib_types.cls_reflection_module->by_val,
            metadata_token_class->by_val,
            const_array_class->by_ref,
        };
        const metadata::RtMethodInfo* ctor =
            Method::find_matched_method_in_class_by_name_and_signature(runtime_customattribute_data_class, STR_CTOR, param_type_sigs, 3);
        if (!ctor)
            RET_ERR(RtErr::MissingMethod);
        s_runtime_customattribute_data_class = runtime_customattribute_data_class;
        s_metadata_token_class = metadata_token_class;
        s_const_array_class = const_array_class;
        s_customattribute_data_ctor = ctor;
    }
    RET_OK(s_customattribute_data_ctor);
}

RtResult<RtArray*> CustomAttribute::get_customattributes_data_on_target_token(metadata::RtModuleDef* mod, metadata::EncodedTokenId target_token)
{
    const CorLibTypes& types = Class::get_corlib_types();

    if (target_token == 0)
    {
        return LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS_INTERNAL(types.cls_customattributedata, "CustomAttribute::get_customattributes_data_on_target_token");
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(metadata::RtCustomAttributeRidRange, rid_range, mod->get_custom_attribute_rid_range(target_token));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(const metadata::RtMethodInfo*, ca_data_ctor, get_customattribute_data_ctor());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(RtArray*, ca_data_arr, LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(types.cls_customattributedata, (int32_t)rid_range.count, "CustomAttribute::get_customattributes_data_on_target_token"));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionModule*, module_obj, Reflection::get_module_reflection_object(mod));
    for (uint32_t i = 0; i < rid_range.count; ++i)
    {
        uint32_t ca_rid = rid_range.start_rid + i;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtCustomAttributeRawData, raw_data, mod->get_custom_attribute_raw_data(ca_rid));

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, reader, mod->get_decoded_blob_reader(raw_data.dataBlobIndex));
        RuntimeMetadataConstArray blob;
        blob.length = static_cast<int32_t>(reader.length());
        blob.data = reinterpret_cast<intptr_t>(reader.data());

        int32_t ctor_token = static_cast<int32_t>(raw_data.ctor_token);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ctor_token_obj,
                                                LEANCLR_BOX_OBJECT_INTERNAL(s_metadata_token_class, &ctor_token,
                                                                            "CustomAttribute::get_customattributes_data_on_target_token"));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, blob_obj,
                                                LEANCLR_BOX_OBJECT_INTERNAL(s_const_array_class, &blob,
                                                                            "CustomAttribute::get_customattributes_data_on_target_token"));

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ca_data_obj, LEANCLR_NEWOBJ_INTERNAL(ca_data_ctor->parent, "CustomAttribute::get_customattributes_data_on_target_token"));
        RtObject* ctor_args[3] = {
            reinterpret_cast<RtObject*>(module_obj),
            ctor_token_obj,
            blob_obj,
        };
        RET_ERR_ON_FAIL(Runtime::invoke_object_arguments_with_run_cctor(ca_data_ctor, ca_data_obj, ctor_args, 3));

        // gc::GarbageCollector::write_barrier((RtObject**)Array::get_array_data_start_as<RtObject*>(ca_data_arr) + i, ca_data_obj);
        Array::set_array_data_at<RtObject*>(ca_data_arr, (int32_t)i, ca_data_obj);
    }
    RET_OK(ca_data_arr);
}

struct MarshalAsMetadatas
{
    const metadata::RtFieldInfo* marshal_cookie;
    const metadata::RtFieldInfo* marshal_type;
    const metadata::RtFieldInfo* marshal_type_ref;
    const metadata::RtFieldInfo* safe_array_user_defined_sub_type;
    const metadata::RtFieldInfo* array_sub_type;
    const metadata::RtFieldInfo* safe_array_sub_type;
    const metadata::RtFieldInfo* size_const;
    const metadata::RtFieldInfo* iid_parameter_index;
    const metadata::RtFieldInfo* size_param_index;
    const metadata::RtMethodInfo* ctor_int16;
};

static MarshalAsMetadatas s_marshal_as_metadatas;

static void init_marshal_as_fields()
{
    if (s_marshal_as_metadatas.ctor_int16)
    {
        return;
    }
    const CorLibTypes& corlib_types = Class::get_corlib_types();
    const metadata::RtClass* marshal_as_klass = corlib_types.cls_marshal_as;
    const metadata::RtTypeSig* param_type_sigs[] = {
        corlib_types.cls_int16->by_val,
    };
    s_marshal_as_metadatas.ctor_int16 = Method::find_matched_method_in_class_by_name_and_signature(marshal_as_klass, STR_CTOR, param_type_sigs, 1);
    assert(s_marshal_as_metadatas.ctor_int16);
    for (uint16_t i = 0; i < marshal_as_klass->field_count; ++i)
    {
        const metadata::RtFieldInfo* field = marshal_as_klass->fields + i;
        if (!Field::is_instance(field))
        {
            continue;
        }
        if (strcmp(field->name, "MarshalCookie") == 0)
        {
            s_marshal_as_metadatas.marshal_cookie = field;
        }
        else if (strcmp(field->name, "MarshalType") == 0)
        {
            s_marshal_as_metadatas.marshal_type = field;
        }
        else if (strcmp(field->name, "MarshalTypeRef") == 0)
        {
            s_marshal_as_metadatas.marshal_type_ref = field;
        }
        else if (strcmp(field->name, "SafeArrayUserDefinedSubType") == 0)
        {
            s_marshal_as_metadatas.safe_array_user_defined_sub_type = field;
        }
        else if (strcmp(field->name, "ArraySubType") == 0)
        {
            s_marshal_as_metadatas.array_sub_type = field;
        }
        else if (strcmp(field->name, "SafeArraySubType") == 0)
        {
            s_marshal_as_metadatas.safe_array_sub_type = field;
        }
        else if (strcmp(field->name, "SizeConst") == 0)
        {
            s_marshal_as_metadatas.size_const = field;
        }
        else if (strcmp(field->name, "SizeParamIndex") == 0)
        {
            s_marshal_as_metadatas.size_param_index = field;
        }
        else if (strcmp(field->name, "IidParameterIndex") == 0)
        {
            s_marshal_as_metadatas.iid_parameter_index = field;
        }
    }
    assert(s_marshal_as_metadatas.marshal_cookie);
    assert(s_marshal_as_metadatas.marshal_type);
    assert(s_marshal_as_metadatas.marshal_type_ref);
    assert(s_marshal_as_metadatas.safe_array_user_defined_sub_type);
    assert(s_marshal_as_metadatas.array_sub_type);
    assert(s_marshal_as_metadatas.safe_array_sub_type);
    assert(s_marshal_as_metadatas.size_const);
    assert(s_marshal_as_metadatas.size_param_index);
    assert(s_marshal_as_metadatas.iid_parameter_index);
}

static RtString* create_utf8_span_string(const metadata::RtMarshalUtf8Span& span)
{
    if (span.is_invalid())
    {
        return nullptr;
    }
    return String::create_string_from_utf8chars(span.data, static_cast<int32_t>(span.length));
}

RtResult<RtCustomAttribute*> CustomAttribute::get_marshal_info(const metadata::RtFieldInfo* field)
{
    if (!vm::Field::has_field_marshal(field))
    {
        RET_OK(nullptr);
    }

    metadata::RtMarshalSpec spec{};
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, has_marshal, Marshal::get_marshal_spec(field, spec));
    assert(has_marshal);

    init_marshal_as_fields();

    const CorLibTypes& corlib_types = Class::get_corlib_types();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, marshal_as_obj, LEANCLR_NEWOBJ_INTERNAL(corlib_types.cls_marshal_as, "CustomAttribute::get_marshal_info"));
    const void* ctor_args[1] = {&spec.native_type};
    RET_ERR_ON_FAIL(Runtime::invoke_with_run_cctor(s_marshal_as_metadatas.ctor_int16, marshal_as_obj, ctor_args));

    metadata::RtMarshalNativeType ele_type = metadata::RtMarshalNativeType::Max;
    uint32_t param_num = 0;
    uint32_t num_elems = 0;
    switch (spec.native_type)
    {
        case metadata::RtMarshalNativeType::FixedSysString:
        {
            auto& fixed_sys_string_data = spec.fixed_sys_string;
            vm::Field::set_instance_value(s_marshal_as_metadatas.size_const, marshal_as_obj, &fixed_sys_string_data.size);
            break;
        }
        case metadata::RtMarshalNativeType::SafeArray:
        {
            auto& safe_array_data = spec.safe_array;
            vm::Field::set_instance_value(s_marshal_as_metadatas.safe_array_user_defined_sub_type, marshal_as_obj, &safe_array_data.variant_type);
            vm::Field::set_instance_value(s_marshal_as_metadatas.safe_array_sub_type, marshal_as_obj, &safe_array_data.udt);
            break;
        }
        case metadata::RtMarshalNativeType::FixedArray:
        {
            auto& fixed_array_data = spec.fixed_array;
            vm::Field::set_instance_value(s_marshal_as_metadatas.size_const, marshal_as_obj, &fixed_array_data.size);
            vm::Field::set_instance_value(s_marshal_as_metadatas.array_sub_type, marshal_as_obj, &fixed_array_data.array_element_type);
            break;
        }
        case metadata::RtMarshalNativeType::Array:
        {
            auto& arr_data = spec.array;
            vm::Field::set_instance_value(s_marshal_as_metadatas.array_sub_type, marshal_as_obj, &arr_data.array_element_type);
            vm::Field::set_instance_value(s_marshal_as_metadatas.size_param_index, marshal_as_obj, &arr_data.param_index);
            vm::Field::set_instance_value(s_marshal_as_metadatas.size_const, marshal_as_obj, &arr_data.element_count);
            break;
        }
        case metadata::RtMarshalNativeType::CustomMarshaler:
        {
            auto& custom_marshaler_data = spec.custom_marshaler;
            if (custom_marshaler_data.guid.is_valid())
            {
                RtString* guid_str = create_utf8_span_string(custom_marshaler_data.guid);
                vm::Field::set_instance_value(s_marshal_as_metadatas.marshal_cookie, marshal_as_obj, &guid_str);
            }
            if (custom_marshaler_data.cookie.is_valid())
            {
                RtString* cookie_str = create_utf8_span_string(custom_marshaler_data.cookie);
                vm::Field::set_instance_value(s_marshal_as_metadatas.marshal_cookie, marshal_as_obj, &cookie_str);
            }
            if (custom_marshaler_data.type_name.is_valid())
            {
                RtString* type_name_str = create_utf8_span_string(custom_marshaler_data.type_name);
                vm::Field::set_instance_value(s_marshal_as_metadatas.marshal_type, marshal_as_obj, &type_name_str);
            }
            if (custom_marshaler_data.custom_marshaler_type)
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, type_obj, Reflection::get_type_reflection_object(custom_marshaler_data.custom_marshaler_type));
                vm::Field::set_instance_value(s_marshal_as_metadatas.marshal_type_ref, marshal_as_obj, &type_obj);
            }
            break;
        }
        case metadata::RtMarshalNativeType::IUnknown:
        case metadata::RtMarshalNativeType::IDispatch:
        case metadata::RtMarshalNativeType::IntF:
        {
            vm::Field::set_instance_value(s_marshal_as_metadatas.iid_parameter_index, marshal_as_obj, &spec.interface.iid_param_index);
            break;
        }
        default: break;
    }
    RET_OK((RtCustomAttribute*)marshal_as_obj);
}

} // namespace vm
} // namespace leanclr

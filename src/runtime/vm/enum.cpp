#include "enum.h"
#include "rt_array.h"
#include "class.h"
#include "field.h"
#include "object.h"
#include "rt_string.h"

namespace leanclr
{
namespace vm
{

int32_t Enum::get_enum_long_hash_code(int64_t value)
{
    int32_t low = static_cast<int32_t>(value);
    int32_t high = static_cast<int32_t>(value >> 32);
    return low ^ high;
}

static bool is_enum_item_field(const metadata::RtFieldInfo* field)
{
    return Field::is_static_literal(field);
}

RtResult<std::tuple<bool, RtArray*, RtArray*>> Enum::get_enum_values_and_names(metadata::RtClass* klass)
{
    assert(Class::is_enum_type(klass));
    RET_ERR_ON_FAIL(Class::initialize_all(klass));

    // Count enum items
    int32_t enum_item_count = 0;
    if (klass->field_count != 0)
    {
        for (uint16_t i = 0; i < klass->field_count; i++)
        {
            if (is_enum_item_field(&klass->fields[i]))
            {
                enum_item_count++;
            }
        }
    }

    // Create arrays for values and names
    const auto& corlib_types = Class::get_corlib_types();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, values, Array::new_szarray_from_ele_klass(corlib_types.cls_uint64, enum_item_count));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, names, Array::new_szarray_from_ele_klass(corlib_types.cls_string, enum_item_count));

    // Fill arrays
    int32_t index = 0;
    bool sorted = true;
    uint64_t last_value = 0;
    metadata::RtElementType enum_item_type = Class::get_enum_element_type(klass);

    for (uint16_t i = 0; i < klass->field_count; i++)
    {
        const metadata::RtFieldInfo* field = &klass->fields[i];
        if (!is_enum_item_field(field))
        {
            continue;
        }

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const void*, rva_data, Field::get_field_const_data(field));
        if (rva_data == nullptr)
        {
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }

        uint64_t value = 0;
        switch (enum_item_type)
        {
        case metadata::RtElementType::Boolean:
        case metadata::RtElementType::I1:
            value = static_cast<uint64_t>(static_cast<int8_t>(*static_cast<const int8_t*>(rva_data)));
            break;
        case metadata::RtElementType::U1:
            value = static_cast<uint64_t>(*static_cast<const uint8_t*>(rva_data));
            break;
        case metadata::RtElementType::I2:
            value = static_cast<uint64_t>(static_cast<int16_t>(*static_cast<const int16_t*>(rva_data)));
            break;
        case metadata::RtElementType::Char:
        case metadata::RtElementType::U2:
            value = static_cast<uint64_t>(*static_cast<const uint16_t*>(rva_data));
            break;
        case metadata::RtElementType::I4:
            value = static_cast<uint64_t>(static_cast<int32_t>(*static_cast<const int32_t*>(rva_data)));
            break;
        case metadata::RtElementType::U4:
            value = static_cast<uint64_t>(*static_cast<const uint32_t*>(rva_data));
            break;
        case metadata::RtElementType::I8:
            value = static_cast<uint64_t>(*static_cast<const int64_t*>(rva_data));
            break;
        case metadata::RtElementType::U8:
            value = *static_cast<const uint64_t*>(rva_data);
            break;
        default:
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }

        if (index > 0)
        {
            if (value < last_value)
            {
                sorted = false;
            }
        }
        last_value = value;

        Array::set_array_data_at<uint64_t>(values, index, value);
        Array::set_array_data_at<RtString*>(names, index, String::create_string_from_utf8cstr(field->name));
        index++;
    }

    assert(index == enum_item_count);
    RET_OK(std::make_tuple(sorted, values, names));
}

RtResult<uint64_t> Enum::get_boxed_enum_data_as_unsigned_and_extended_to_u64(RtObject* obj)
{
    const metadata::RtClass* klass = obj->klass;
    assert(Class::is_enum_type(klass));

    const void* data_ptr = Object::get_boxed_enum_data_ptr(obj);
    metadata::RtElementType enum_item_type = Class::get_enum_element_type(klass);

    uint64_t value = 0;
    switch (enum_item_type)
    {
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::I1:
    case metadata::RtElementType::U1:
        value = *static_cast<const uint8_t*>(data_ptr);
        break;
    case metadata::RtElementType::I2:
    case metadata::RtElementType::Char:
    case metadata::RtElementType::U2:
        value = *static_cast<const uint16_t*>(data_ptr);
        break;
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
        value = *static_cast<const uint32_t*>(data_ptr);
        break;
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
        value = *static_cast<const uint64_t*>(data_ptr);
        break;
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }

    RET_OK(value);
}

RtResult<int32_t> Enum::get_hash_code(RtObject* obj)
{
    const metadata::RtClass* klass = obj->klass;
    assert(Class::is_enum_type(klass));

    const void* data_ptr = Object::get_boxed_enum_data_ptr(obj);
    metadata::RtElementType enum_item_type = Class::get_enum_element_type(klass);

    int32_t result = 0;
    switch (enum_item_type)
    {
    case metadata::RtElementType::Boolean:
        result = *static_cast<const uint8_t*>(data_ptr);
        break;
    case metadata::RtElementType::Char:
        result = static_cast<int32_t>(*static_cast<const uint16_t*>(data_ptr));
        break;
    case metadata::RtElementType::I1:
        result = static_cast<int32_t>(*static_cast<const int8_t*>(data_ptr));
        break;
    case metadata::RtElementType::U1:
        result = static_cast<int32_t>(*static_cast<const uint8_t*>(data_ptr));
        break;
    case metadata::RtElementType::I2:
        // it's unnecessary to cast to uint16_t, we just make implementation same to il2cpp.
        result = static_cast<int32_t>(*static_cast<const uint16_t*>(data_ptr));
        break;
    case metadata::RtElementType::U2:
        result = static_cast<int32_t>(*static_cast<const uint16_t*>(data_ptr));
        break;
    case metadata::RtElementType::I4:
        result = *static_cast<const int32_t*>(data_ptr);
        break;
    case metadata::RtElementType::U4:
        result = static_cast<int32_t>(*static_cast<const uint32_t*>(data_ptr));
        break;
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    {
        const int32_t* two_parts = static_cast<const int32_t*>(data_ptr);
        result = two_parts[0] ^ two_parts[1];
        break;
    }
    default:
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }

    RET_OK(result);
}

} // namespace vm
} // namespace leanclr

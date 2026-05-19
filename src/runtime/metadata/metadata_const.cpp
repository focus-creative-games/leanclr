#include "metadata_const.h"
#include "module_def.h"
#include "vm/class.h"
#include "vm/object.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace metadata
{

RtResult<vm::RtObject*> MetadataConst::decode_const_object(RtModuleDef* mod, EncodedTokenId token, const RtTypeSig* typeSig)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(TypedConstRawData, raw_data, mod->get_const_or_default_value(token));
    utils::BinaryReader reader(raw_data.data, raw_data.size);

    const void* dataPtr = reader.data();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, fieldClass, vm::Class::get_class_from_typesig(typeSig));
    RtElementType const_type = raw_data.type;
    bool is_default_value = const_type == RtElementType::Class;
    switch (typeSig->ele_type)
    {
    case RtElementType::Boolean:
    case RtElementType::Char:
    case RtElementType::I1:
    case RtElementType::U1:
    case RtElementType::I2:
    case RtElementType::U2:
    case RtElementType::I4:
    case RtElementType::U4:
    case RtElementType::I8:
    case RtElementType::U8:
    case RtElementType::R4:
    case RtElementType::R8:
    case RtElementType::I:
    case RtElementType::U:
    case RtElementType::Ptr:
    case RtElementType::FnPtr:
    {
        if (is_default_value)
        {
            return LEANCLR_NEWOBJ_INTERNAL(fieldClass, "MetadataConst::decode_const_object");
        }
        if (raw_data.type != typeSig->ele_type)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        return LEANCLR_BOX_OBJECT_INTERNAL(fieldClass, dataPtr, "MetadataConst::decode_const_object");
    }
    case RtElementType::String:
    {
        if (is_default_value)
        {
            return nullptr;
        }
        if (raw_data.type != typeSig->ele_type)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }

        if (reader.length() % 2 != 0)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        RET_OK(vm::String::create_string_from_utf16chars(reinterpret_cast<const Utf16Char*>(dataPtr), static_cast<int32_t>(reader.length() / 2)));
    }
    case RtElementType::Object:
    case RtElementType::Array:
    case RtElementType::SZArray:
    case RtElementType::Class:
    {
        if (is_default_value)
        {
            return nullptr;
        }
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    case RtElementType::ValueType:
    {
        if (is_default_value)
        {
            return LEANCLR_NEWOBJ_INTERNAL(fieldClass, "MetadataConst::decode_const_object");
        }
        if (vm::Class::is_enum_type(fieldClass))
        {
            if (raw_data.type != fieldClass->element_class->by_val->ele_type)
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            return LEANCLR_BOX_OBJECT_INTERNAL(fieldClass, dataPtr, "MetadataConst::decode_const_object");
        }
        else
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
    }
    case RtElementType::GenericInst:
    {
        if (vm::Class::is_reference_type(fieldClass))
        {
            if (is_default_value)
            {
                return nullptr;
            }
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        const RtClass* element_class = fieldClass->element_class;
        if (vm::Class::is_nullable_type(fieldClass))
        {
            if (is_default_value)
            {
                return nullptr;
            }
            if (raw_data.type != element_class->by_val->ele_type)
            {
                // when element_type is enum, the raw data can also be encoded as the underlying integral type, so we need to check it specially here
                if (!vm::Class::is_enum_type(element_class) || raw_data.type != element_class->element_class->by_val->ele_type)
                {
                    RET_ASSERT_ERR(RtErr::BadImageFormat);
                }
            }
            return LEANCLR_BOX_OBJECT_INTERNAL(element_class, dataPtr, "MetadataConst::decode_const_object");
        }
        if (is_default_value)
        {
            return LEANCLR_NEWOBJ_INTERNAL(fieldClass, "MetadataConst::decode_const_object");
        }
        if (vm::Class::is_enum_type(fieldClass) && raw_data.type == element_class->by_val->ele_type)
        {
            return LEANCLR_BOX_OBJECT_INTERNAL(fieldClass, dataPtr, "MetadataConst::decode_const_object");
        }
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    default:
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    }
}

} // namespace metadata
} // namespace leanclr

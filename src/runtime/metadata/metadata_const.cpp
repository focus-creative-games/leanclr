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
    auto retReader = mod->get_const_or_default_value(token);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, reader, retReader);

    const void* dataPtr = reader.data();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, fieldClass, vm::Class::get_class_from_typesig(typeSig));
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
    {
        return vm::Object::box_object(fieldClass, dataPtr);
    }
    case RtElementType::String:
    {
        if (reader.length() % 2 != 0)
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        RET_OK(vm::String::create_string_from_utf16chars(reinterpret_cast<const Utf16Char*>(dataPtr), static_cast<int32_t>(reader.length() / 2)));
    }
    case RtElementType::Class:
    {
        return nullptr;
    }
    case RtElementType::ValueType:
    {
        if (vm::Class::is_enum_type(fieldClass))
        {
            return vm::Object::box_object(fieldClass, dataPtr);
        }
        else
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
    }
    case RtElementType::GenericInst:
    {
        if (vm::Class::is_enum_type(fieldClass))
        {
            return vm::Object::box_object(fieldClass, dataPtr);
        }
        else if (vm::Class::is_reference_type(fieldClass))
        {
            return nullptr;
        }
        else
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
    }
    default:
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    }
}

} // namespace metadata
} // namespace leanclr

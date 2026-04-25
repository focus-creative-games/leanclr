#include "interp_defs.h"

#include "vm/class.h"
#include "vm/generic_class.h"

namespace leanclr
{
namespace interp
{
RtEvalStackDataType InterpDefs::get_eval_stack_data_type_by_reduce_type(metadata::RtArgOrLocOrFieldReduceType reduce_type)
{
    using namespace metadata;
    switch (reduce_type)
    {
    case RtArgOrLocOrFieldReduceType::Unspecific:
    case RtArgOrLocOrFieldReduceType::Void:
    {
        assert(false && "Unspecific or Void type has no eval stack data type");
        return RtEvalStackDataType::Other;
    }
    case RtArgOrLocOrFieldReduceType::I1:
    case RtArgOrLocOrFieldReduceType::U1:
    case RtArgOrLocOrFieldReduceType::I2:
    case RtArgOrLocOrFieldReduceType::U2:
    case RtArgOrLocOrFieldReduceType::I4:
        return RtEvalStackDataType::I4;
    case RtArgOrLocOrFieldReduceType::I8:
        return RtEvalStackDataType::I8;
    case RtArgOrLocOrFieldReduceType::R4:
        return RtEvalStackDataType::R4;
    case RtArgOrLocOrFieldReduceType::R8:
        return RtEvalStackDataType::R8;
    case RtArgOrLocOrFieldReduceType::Ref:
    case RtArgOrLocOrFieldReduceType::I:
        return RtEvalStackDataType::RefOrPtr;
    case RtArgOrLocOrFieldReduceType::Other:
        return RtEvalStackDataType::Other;
    default:
        assert(false && "Not implemented reduce type");
        return RtEvalStackDataType::Other;
    }
}

RtResult<ReduceTypeAndSize> InterpDefs::get_reduce_type_and_size_by_typesig(const metadata::RtTypeSig* typeSig)
{
    ReduceTypeAndSize result{};
    if (typeSig->by_ref)
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::Ref;
        result.byte_size = PTR_SIZE;
        RET_OK(result);
    }
    switch (typeSig->ele_type)
    {
    case metadata::RtElementType::Void:
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::Void;
        result.byte_size = 0;
        break;
    }
    case metadata::RtElementType::I1:
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::I1;
        result.byte_size = 1;
        break;
    }
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::U1:
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::U1;
        result.byte_size = 1;
        break;
    }
    case metadata::RtElementType::I2:
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::I2;
        result.byte_size = 2;
        break;
    }
    case metadata::RtElementType::Char:
    case metadata::RtElementType::U2:
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::U2;
        result.byte_size = 2;
        break;
    }
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::I4;
        result.byte_size = 4;
        break;
    }
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::I8;
        result.byte_size = 8;
        break;
    }
    case metadata::RtElementType::R4:
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::R4;
        result.byte_size = 4;
        break;
    }
    case metadata::RtElementType::R8:
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::R8;
        result.byte_size = 8;
        break;
    }
    case metadata::RtElementType::I:
    case metadata::RtElementType::U:
    case metadata::RtElementType::Ptr:
    case metadata::RtElementType::FnPtr:
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::I;
        result.byte_size = PTR_SIZE;
        break;
    }
    case metadata::RtElementType::Object:
    case metadata::RtElementType::String:
    case metadata::RtElementType::Class:
    case metadata::RtElementType::SZArray:
    case metadata::RtElementType::Array:
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::Ref;
        result.byte_size = PTR_SIZE;
        break;
    }
    case metadata::RtElementType::TypedByRef:
    {
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::Other;
        result.byte_size = vm::RT_TYPED_REFERENCE_SIZE;
        break;
    }
    case metadata::RtElementType::ValueType:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, cls, vm::Class::get_class_from_typesig(typeSig));
        if (vm::Class::is_enum_type(cls))
        {
            return get_reduce_type_and_size_by_typesig(cls->element_class->by_val);
        }
        RET_ERR_ON_FAIL(vm::Class::initialize_fields(cls));
        result.byte_size = vm::Class::get_instance_size_without_object_header(cls);
        result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::Other;
        break;
    }
    case metadata::RtElementType::GenericInst:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, baseClass, vm::GenericClass::get_base_class(typeSig->data.generic_class));
        if (vm::Class::is_reference_type(baseClass))
        {
            result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::Ref;
            result.byte_size = PTR_SIZE;
        }
        else
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, inflatedClass, vm::Class::get_class_from_typesig(typeSig));
            if (vm::Class::is_enum_type(inflatedClass))
            {
                return get_reduce_type_and_size_by_typesig(inflatedClass->element_class->by_val);
            }
            RET_ERR_ON_FAIL(vm::Class::initialize_fields(inflatedClass));
            result.byte_size = vm::Class::get_instance_size_without_object_header(inflatedClass);
            result.reduce_type = metadata::RtArgOrLocOrFieldReduceType::Other;
        }
        break;
    }
    default:
        assert(false && "Not implemented type sig ele type");
        RETURN_NOT_IMPLEMENTED_ERROR();
    }
    RET_OK(result);
}

size_t InterpDefs::get_stack_object_size_by_byte_size(size_t byte_size)
{
    assert(byte_size <= UINT16_MAX * sizeof(RtStackObject) && "byte_size too large for stack object");
    return (byte_size + 7) / 8;
}
} // namespace interp
} // namespace leanclr

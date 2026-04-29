#include "layout.h"
#include "vm/class.h"
#include "utils/mem_op.h"
#include "vm/generic_class.h"
#include "metadata/module_def.h"

namespace leanclr
{
namespace metadata
{
using namespace leanclr::utils;

// Get field size and alignment information
RtResult<SizeAndAlignment> Layout::get_field_size_and_alignment(RtTypeSig* typeSig)
{
    if (typeSig->by_ref)
    {
        SizeAndAlignment result = {static_cast<uint32_t>(leanclr::PTR_SIZE), static_cast<uint32_t>(leanclr::PTR_SIZE)};
        RET_OK(result);
    }

    SizeAndAlignment result;

    switch (typeSig->ele_type)
    {
    case RtElementType::Boolean:
        result = {1, 1};
        break;
    case RtElementType::Char:
        result = {2, 2};
        break;
    case RtElementType::I1:
    case RtElementType::U1:
        result = {1, 1};
        break;
    case RtElementType::I2:
    case RtElementType::U2:
        result = {2, 2};
        break;
    case RtElementType::I4:
    case RtElementType::U4:
    case RtElementType::R4:
        result = {4, 4};
        break;
    case RtElementType::I8:
    case RtElementType::U8:
    case RtElementType::R8:
        result = {8, 8};
        break;
    case RtElementType::I:
    case RtElementType::U:
        result = {static_cast<uint32_t>(leanclr::PTR_SIZE), static_cast<uint32_t>(leanclr::PTR_SIZE)};
        break;
    case RtElementType::String:
    case RtElementType::Class:
    case RtElementType::Object:
    case RtElementType::Array:
    case RtElementType::SZArray:
    case RtElementType::Var:
    case RtElementType::MVar:
    case RtElementType::Ptr:
    case RtElementType::FnPtr:
        result = {static_cast<uint32_t>(leanclr::PTR_SIZE), static_cast<uint32_t>(leanclr::PTR_SIZE)};
        break;
    case RtElementType::ValueType:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, field_class, vm::Class::get_class_from_typesig(typeSig));
        RET_ERR_ON_FAIL(vm::Class::initialize_fields(field_class));
        result = {field_class->instance_size_without_header, field_class->alignment};
        break;
    }
    case RtElementType::GenericInst:
    {
        const RtGenericClass* generic_class = typeSig->data.generic_class;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, baseClass, vm::GenericClass::get_base_class(generic_class));
        if (vm::Class::is_reference_type(baseClass))
        {
            result = {static_cast<uint32_t>(leanclr::PTR_SIZE), static_cast<uint32_t>(leanclr::PTR_SIZE)};
            RET_OK(result);
        }
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, fieldClass, vm::Class::get_class_from_typesig(typeSig));
        RET_ERR_ON_FAIL(vm::Class::initialize_fields(fieldClass));
        result = {fieldClass->instance_size_without_header, fieldClass->alignment};
        break;
    }
    case RtElementType::TypedByRef:
        result = {vm::RT_TYPED_REFERENCE_SIZE, static_cast<uint32_t>(PTR_ALIGN)};
        break;
    default:
        RETURN_NOT_IMPLEMENTED_ERROR();
    }

    RET_OK(result);
}

// Compute layout for fields with sequential layout
RtResult<SizeAndAlignment> Layout::compute_layout(utils::Vector<const RtFieldInfo*>& fields, int32_t first_field_index_of_cur_klass, uint8_t packing)
{
    uint32_t offset = 0;
    uint8_t max_alignment = 1;

    int32_t field_index = 0;
    for (const RtFieldInfo* field : fields)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(SizeAndAlignment, field_info, get_field_size_and_alignment(const_cast<RtTypeSig*>(field->type_sig)));

        uint8_t effective_alignment =
            (packing != 0) ? std::min(static_cast<uint8_t>(field_info.alignment), packing) : static_cast<uint8_t>(field_info.alignment);

        offset = static_cast<uint32_t>(MemOp::align_up(offset, effective_alignment));

        if (field_index >= first_field_index_of_cur_klass)
        {
            const_cast<RtFieldInfo*>(field)->offset = offset;
        }
        else
        {
            assert(field->offset == offset && "Field offset mismatch for inherited field with sequential layout");
        }

        offset += field_info.size;
        max_alignment = std::max(max_alignment, effective_alignment);
        ++field_index;
    }

    offset = static_cast<uint32_t>(MemOp::align_up(offset, max_alignment));

    SizeAndAlignment result = {offset, max_alignment};
    RET_OK(result);
}

// Compute layout for fields with explicit layout
RtResult<SizeAndAlignment> Layout::compute_explicit_layout(RtModuleDef* mod, utils::Vector<const RtFieldInfo*>& fields, uint8_t packing)
{
    uint32_t max_size = 0;
    uint8_t max_alignment = 1;

    for (const RtFieldInfo* field : fields)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(SizeAndAlignment, field_info, get_field_size_and_alignment(const_cast<RtTypeSig*>(field->type_sig)));

        uint8_t effective_alignment =
            (packing != 0) ? std::min(static_cast<uint8_t>(field_info.alignment), packing) : static_cast<uint8_t>(field_info.alignment);

        auto field_offset_opt = mod->get_field_offset(field->token);
        if (!field_offset_opt.has_value())
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }

        uint32_t offset = field_offset_opt.value();
        const_cast<RtFieldInfo*>(field)->offset = offset;

        max_size = std::max(max_size, offset + field_info.size);
        max_alignment = std::max(max_alignment, effective_alignment);
    }
    max_size = static_cast<uint32_t>(MemOp::align_up(max_size, max_alignment));

    SizeAndAlignment result = {max_size, max_alignment};
    RET_OK(result);
}

} // namespace metadata
} // namespace leanclr

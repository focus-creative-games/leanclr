#include "metadata_compare.h"
#include "vm/method.h"

namespace leanclr
{
namespace metadata
{

// Type signature comparison (from metadata_compare.rs)
bool MetadataCompare::is_typesig_equal_ignore_attrs(const RtTypeSig* a, const RtTypeSig* b, bool compareGenericParamByIndex)
{
    if (a->ele_type != b->ele_type)
        return false;
    if ((a->by_ref != 0) != (b->by_ref != 0))
        return false;

    switch (a->ele_type)
    {
    case RtElementType::ValueType:
    case RtElementType::Class:
        return a->data.type_def_gid == b->data.type_def_gid;

    case RtElementType::Ptr:
    case RtElementType::SZArray:
        return is_typesig_equal_ignore_attrs(a->data.element_type, b->data.element_type, compareGenericParamByIndex);

    case RtElementType::Array:
    {
        const RtArrayType* ar = a->data.array_type;
        const RtArrayType* br = b->data.array_type;
        return ar->rank == br->rank && is_typesig_equal_ignore_attrs(ar->ele_type, br->ele_type, compareGenericParamByIndex);
    }

    case RtElementType::GenericInst:
    {
        const RtGenericClass* gia = a->data.generic_class;
        const RtGenericClass* gib = b->data.generic_class;
        if (gia->base_type_def_gid != gib->base_type_def_gid)
            return false;
        if (gia->class_inst == gib->class_inst)
            return true;
        if (!compareGenericParamByIndex)
            return false;
        for (uint8_t i = 0; i < gia->class_inst->generic_arg_count; ++i)
        {
            const RtTypeSig* at = gia->class_inst->generic_args[i];
            const RtTypeSig* bt = gib->class_inst->generic_args[i];
            if (!is_typesig_equal_ignore_attrs(at, bt, compareGenericParamByIndex))
                return false;
        }
        return true;
    }

    case RtElementType::Var:
    case RtElementType::MVar:
        if (compareGenericParamByIndex)
            return a->data.generic_param->index == b->data.generic_param->index;
        else
            return a->data.generic_param == b->data.generic_param;

    default:
        return true;
    }
}

bool MetadataCompare::is_typesigs_equal_ignore_attrs(const RtTypeSig* const* sigs1, const RtTypeSig* const* sigs2, size_t count,
                                                     bool compareGenericParamByIndex)
{
    for (size_t i = 0; i < count; ++i)
    {
        if (!is_typesig_equal_ignore_attrs(sigs1[i], sigs2[i], compareGenericParamByIndex))
            return false;
    }
    return true;
}

bool MetadataCompare::is_method_signature_equal(const RtMethodInfo* a, const RtMethodInfo* b, bool compareName, bool compareGenericParamByIndex)
{
    if (compareName && std::strcmp(a->name, b->name) != 0)
        return false;
    if (a->parameter_count != b->parameter_count)
        return false;

    if (!is_typesig_equal_ignore_attrs(a->return_type, b->return_type, compareGenericParamByIndex))
    {
        return false;
    }
    if (!is_typesigs_equal_ignore_attrs(a->parameters, b->parameters, a->parameter_count, compareGenericParamByIndex))
    {
        return false;
    }
    if (vm::Method::get_generic_param_count(a) != vm::Method::get_generic_param_count(b))
        return false;
    return true;
}

bool MetadataCompare::is_method_signature_equal(const RtMethodSig* a, const RtMethodSig* b)
{
    if (a->flags != b->flags)
        return false;
    if (a->generic_param_count != b->generic_param_count)
        return false;
    if (!is_typesig_equal_ignore_attrs(a->return_type, b->return_type, false))
        return false;
    if (!is_typesigs_equal_ignore_attrs(a->params.data(), b->params.data(), a->params.size(), false))
        return false;
    return true;
}
} // namespace metadata
} // namespace leanclr

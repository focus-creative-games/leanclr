#include "metadata_hash.h"
#include "utils/hash_util.h"

namespace leanclr
{
namespace metadata
{
size_t MetadataHash::hash_type_sig_ignore_attrs(const RtTypeSig* a)
{
    size_t h = (size_t)a->ele_type;
    h = utils::HashUtil::combine_hash(h, a->by_ref);
    switch (a->ele_type)
    {
    case RtElementType::Class:
    case RtElementType::ValueType:
    {
        h = utils::HashUtil::combine_hash(h, (size_t)(a->data.type_def_gid));
        break;
    }
    case RtElementType::Ptr:
    case RtElementType::SZArray:
    {
        h = utils::HashUtil::combine_hash(h, hash_type_sig_ignore_attrs(a->data.element_type));
        break;
    }
    case RtElementType::Array:
    {
        auto arrType = a->data.array_type;
        h = utils::HashUtil::combine_hash(h, (size_t)(arrType->rank));
        h = utils::HashUtil::combine_hash(h, hash_type_sig_ignore_attrs(arrType->ele_type));
        break;
    }
    case RtElementType::GenericInst:
    {
        const RtGenericClass* gc = a->data.generic_class;
        h = utils::HashUtil::combine_hash(h, (size_t)(gc->base_type_def_gid));
        h = utils::HashUtil::combine_hash(h, (size_t)(gc->class_inst));
        break;
    }
    case RtElementType::Var:
    case RtElementType::MVar:
    {
        h = utils::HashUtil::combine_hash(h, (size_t)(a->data.generic_param));
        break;
    }
    default:
        break;
    }
    return h;
}
} // namespace metadata
} // namespace leanclr

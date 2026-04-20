#pragma once

#include "rt_metadata.h"
#include "utils/hash_util.h"

// Type signature hashing (from metadata_hash.rs)
// Forward declaration for RtTypeSig
namespace leanclr
{
namespace metadata
{

class MetadataHash
{
  public:
    static size_t hash_type_sig_ignore_attrs(const RtTypeSig* a);
};

struct TypeSigIgnoreAttrsHasher
{
    size_t operator()(const RtTypeSig* key) const noexcept
    {
        return MetadataHash::hash_type_sig_ignore_attrs(key);
    }
};

struct GenericInstHash
{
    std::size_t operator()(const RtGenericInst* gi) const
    {
        std::size_t h = (size_t)(gi->generic_arg_count);
        for (uint8_t i = 0; i < gi->generic_arg_count; ++i)
        {
            h = utils::HashUtil::combine_hash(h, MetadataHash::hash_type_sig_ignore_attrs(gi->generic_args[i]));
        }
        return h;
    }
};

struct GenericClassHash
{
    std::size_t operator()(const RtGenericClass* key) const
    {
        std::size_t h = (size_t)(key->base_type_def_gid);
        h = utils::HashUtil::combine_hash(h, (size_t)(key->class_inst));
        return h;
    }
};

struct GenericMethodHash
{
    std::size_t operator()(const RtGenericMethod* key) const
    {
        std::size_t h = std::hash<uint32_t>()(key->base_method_gid);
        h = utils::HashUtil::combine_hash(h, (size_t)key->generic_context.class_inst);
        h = utils::HashUtil::combine_hash(h, (size_t)key->generic_context.method_inst);
        return h;
    }
};

struct MethodSigHash
{
    std::size_t operator()(const RtMethodSig* key) const
    {
        size_t h = (size_t)key->flags;
        h = utils::HashUtil::combine_hash(h, (size_t)key->generic_param_count);
        h = utils::HashUtil::combine_hash(h, MetadataHash::hash_type_sig_ignore_attrs(key->return_type));
        for (size_t i = 0; i < key->params.size(); ++i)
        {
            h = utils::HashUtil::combine_hash(h, MetadataHash::hash_type_sig_ignore_attrs(key->params[i]));
        }
        return h;
    }
};

} // namespace metadata
} // namespace leanclr

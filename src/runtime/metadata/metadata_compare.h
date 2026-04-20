#pragma once

#include "rt_metadata.h"

namespace leanclr
{
namespace metadata
{
class MetadataCompare
{
  public:
    static bool is_typesig_equal_ignore_attrs(const RtTypeSig* a, const RtTypeSig* b, bool compareGenericParamByIndex);
    static bool is_typesigs_equal_ignore_attrs(const RtTypeSig* const* sigs1, const RtTypeSig* const* sigs2, size_t count, bool compareGenericParamByIndex);
    static bool is_method_signature_equal(const RtMethodInfo* a, const RtMethodInfo* b, bool compareName, bool compareGenericParamByIndex);
    static bool is_method_signature_equal(const RtMethodSig* a, const RtMethodSig* b);
};

struct TypeSigIgnoreAttrsEqual
{
    bool operator()(const RtTypeSig* a, const RtTypeSig* b) const noexcept
    {
        return MetadataCompare::is_typesig_equal_ignore_attrs(a, b, false);
    }
};

struct GenericInstCompare
{
    bool operator()(const RtGenericInst* a, const RtGenericInst* b) const
    {
        if (a->generic_arg_count != b->generic_arg_count)
            return false;
        return MetadataCompare::is_typesigs_equal_ignore_attrs(a->generic_args, b->generic_args, a->generic_arg_count, false);
    }
};

struct GenericClassCompare
{
    bool operator()(const RtGenericClass* a, const RtGenericClass* b) const
    {
        if (a->base_type_def_gid != b->base_type_def_gid)
            return false;
        return a->class_inst == b->class_inst;
    }
};

struct GenericMethodCompare
{
    bool operator()(const RtGenericMethod* a, const RtGenericMethod* b) const
    {
        return a->base_method_gid == b->base_method_gid && a->generic_context.class_inst == b->generic_context.class_inst &&
               a->generic_context.method_inst == b->generic_context.method_inst;
    }
};

struct MethodSigCompare
{
    bool operator()(const RtMethodSig* a, const RtMethodSig* b) const
    {
        return a->flags == b->flags && a->generic_param_count == b->generic_param_count &&
               MetadataCompare::is_typesig_equal_ignore_attrs(a->return_type, b->return_type, false) &&
               MetadataCompare::is_typesigs_equal_ignore_attrs(a->params.data(), b->params.data(), a->params.size(), false);
    }
};

} // namespace metadata
} // namespace leanclr

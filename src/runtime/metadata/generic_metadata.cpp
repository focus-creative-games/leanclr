#include "generic_metadata.h"
#include "metadata_cache.h"
#include "vm/class.h"
#include "alloc/metadata_allocation.h"
#include <cstring>

namespace leanclr
{
namespace metadata
{
using namespace leanclr::core;
using namespace leanclr::alloc;

// Forward declaration for static helpers
static RtResult<const RtTypeSig*> inflate_typesig_impl(const RtTypeSig* typesig, const RtGenericContext* genericContext);
static RtResult<const RtTypeSig*> apply_generic_param_attributes(const RtTypeSig* genericParamTypeSig, const RtTypeSig* inflatedTypeSig);

// Helper: Apply generic parameter attributes to inflated type
static RtResult<const RtTypeSig*> apply_generic_param_attributes(const RtTypeSig* genericParamTypeSig, const RtTypeSig* inflatedTypeSig)
{
    bool generic_by_ref = genericParamTypeSig->by_ref;
    bool inflated_by_ref = inflatedTypeSig->by_ref;
    uint8_t generic_field_or_param_attrs = genericParamTypeSig->field_or_param_attrs;
    uint8_t inflated_field_or_param_attrs = inflatedTypeSig->field_or_param_attrs;

    if (inflated_field_or_param_attrs == generic_field_or_param_attrs && inflated_by_ref == generic_by_ref)
    {
        RET_OK(inflatedTypeSig);
    }

    RtTypeSig new_sig = *inflatedTypeSig;
    new_sig.field_or_param_attrs = generic_field_or_param_attrs;
    new_sig.by_ref = generic_by_ref ? 1 : 0;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, pooled_sig, MetadataCache::get_pooled_typesig(new_sig));
    RET_OK(pooled_sig);
}

// Helper: Inflate generic instance with generic arguments
RtResult<const RtGenericInst*> GenericMetadata::inflate_generic_inst(const RtGenericInst* genericInst, const RtGenericContext* genericContext)
{
    uint8_t generic_arg_count = genericInst->generic_arg_count;
    assert(generic_arg_count > 0 && generic_arg_count <= RT_MAX_GENERIC_PARAM_COUNT);
    const RtTypeSig* temp_inflated_generic_args[RT_MAX_GENERIC_PARAM_COUNT];

    bool any_changed = false;
    for (size_t i = 0; i < generic_arg_count; ++i)
    {
        const RtTypeSig* old_arg = genericInst->generic_args[i];
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, inflated_arg, inflate_typesig_impl(old_arg, genericContext));
        if (inflated_arg != old_arg)
        {
            any_changed = true;
        }
        temp_inflated_generic_args[i] = inflated_arg;
    }

    if (!any_changed)
    {
        RET_OK(genericInst);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtGenericInst*, new_inst,
                                            MetadataCache::get_pooled_generic_inst(temp_inflated_generic_args, generic_arg_count));
    RET_OK(new_inst);
}

// Main implementation of type signature inflation
static RtResult<const RtTypeSig*> inflate_typesig_impl(const RtTypeSig* typesig, const RtGenericContext* genericContext)
{
    const RtTypeSig* result = nullptr;

    switch (typesig->ele_type)
    {
    case RtElementType::Array:
    {
        const RtArrayType* old_array_type = typesig->data.array_type;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, new_sub_type_sig, inflate_typesig_impl(old_array_type->ele_type, genericContext));

        if (new_sub_type_sig == old_array_type->ele_type)
        {
            result = typesig;
        }
        else
        {
            if (old_array_type->is_canonized())
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
                    const RtTypeSig*, cache_type_sig,
                    MetadataCache::get_pooled_array_typesig_by_element_typesig(new_sub_type_sig, old_array_type->rank, typesig->by_ref));

                if (typesig->is_canonized())
                {
                    result = cache_type_sig;
                }
                else
                {
                    auto newTs = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
                    *newTs = *typesig;
                    newTs->data.array_type = cache_type_sig->data.array_type;
                    result = newTs;
                }
            }
            else
            {
                auto newArr = alloc::MetadataAllocation::malloc_any_zeroed<RtArrayType>();
                *newArr = *old_array_type;
                newArr->ele_type = new_sub_type_sig;
                RtTypeSig* newTs = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
                *newTs = *typesig;
                newTs->data.array_type = newArr;
                result = newTs;
            }
        }
        break;
    }

    case RtElementType::SZArray:
    {
        const RtTypeSig* element_type = typesig->data.element_type;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, new_sub_type_sig, inflate_typesig_impl(element_type, genericContext));

        if (typesig->is_canonized())
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, cache_type_sig,
                                                    MetadataCache::get_pooled_szarray_typesig_by_element_typesig(new_sub_type_sig, typesig->by_ref));
            result = cache_type_sig;
        }
        else
        {
            auto new_ts = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
            *new_ts = *typesig;
            new_ts->data.element_type = new_sub_type_sig;
            result = new_ts;
        }
        break;
    }

    case RtElementType::GenericInst:
    {
        const RtGenericClass* old_generic_class = typesig->data.generic_class;
        const RtGenericInst* old_generic_inst = old_generic_class->class_inst;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtGenericInst*, new_generic_inst,
                                                GenericMetadata::inflate_generic_inst(old_generic_inst, genericContext));

        if (old_generic_inst == new_generic_inst)
        {
            result = typesig;
        }
        else
        {
            const RtGenericClass* generic_class = MetadataCache::get_pooled_generic_class(old_generic_class->base_type_def_gid, new_generic_inst);

            if (typesig->is_canonized())
            {
                if (typesig->by_ref)
                {
                    result = &generic_class->by_ref_type_sig;
                }
                else
                {
                    result = &generic_class->by_val_type_sig;
                }
            }
            else
            {
                auto new_ts = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
                *new_ts = *typesig;
                new_ts->data.generic_class = generic_class;
                result = new_ts;
            }
        }
        break;
    }

    case RtElementType::Ptr:
    {
        const RtTypeSig* element_type = typesig->data.element_type;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, new_sub_type_sig, inflate_typesig_impl(element_type, genericContext));

        if (new_sub_type_sig == element_type)
        {
            result = typesig;
        }
        else
        {
            auto new_ts = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
            *new_ts = *typesig;
            new_ts->data.element_type = new_sub_type_sig;
            result = new_ts;
        }
        break;
    }

    case RtElementType::Var:
    {
        assert(genericContext != nullptr);
        assert(genericContext->class_inst != nullptr);
        uint16_t param_index = typesig->data.generic_param->index;
        const RtTypeSig* new_sub_type_sig = genericContext->class_inst->generic_args[param_index];
        UNWRAP_OR_RET_ERR_ON_FAIL(result, apply_generic_param_attributes(typesig, new_sub_type_sig));
        break;
    }

    case RtElementType::MVar:
    {
        if (genericContext->method_inst == nullptr)
        {
            result = typesig;
        }
        else
        {
            uint16_t param_index = typesig->data.generic_param->index;
            const RtTypeSig* new_sub_type_sig = genericContext->method_inst->generic_args[param_index];
            UNWRAP_OR_RET_ERR_ON_FAIL(result, apply_generic_param_attributes(typesig, new_sub_type_sig));
        }
        break;
    }

    case RtElementType::FnPtr:
    {
        const RtMethodSig* old_method_sig = typesig->data.method_sig;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodSig*, new_method_sig, GenericMetadata::inflate_method_sig(old_method_sig, genericContext));
        if (old_method_sig == new_method_sig)
        {
            result = typesig;
        }
        else
        {
            auto new_ts = alloc::MetadataAllocation::malloc_any_zeroed<RtTypeSig>();
            *new_ts = *typesig;
            new_ts->data.method_sig = new_method_sig;
            result = new_ts;
        }
        break;
    }

    default:
    {
        result = typesig;
        break;
    }
    }

    RET_OK(result);
}

RtResult<const RtTypeSig*> GenericMetadata::inflate_typesig(const RtTypeSig* typesig, const RtGenericContext* genericContext)
{
    return inflate_typesig_impl(typesig, genericContext);
}

RtResultVoid GenericMetadata::inflate_typesigs(utils::Vector<const RtTypeSig*> typesigs, const RtGenericContext* genericContext)
{
    for (size_t i = 0; i < typesigs.size(); ++i)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(typesigs[i], inflate_typesig_impl(typesigs[i], genericContext));
    }
    RET_VOID_OK();
}

RtResult<RtClass*> GenericMetadata::inflate_class(const RtClass* klass, const RtGenericContext* genericContext)
{
    if (!genericContext)
    {
        RET_OK(const_cast<RtClass*>(klass));
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, inflatedTypeSig, inflate_typesig(klass->by_val, genericContext));
    return vm::Class::get_class_from_typesig(inflatedTypeSig);
}

RtResult<const RtMethodSig*> GenericMetadata::inflate_method_sig(const RtMethodSig* methodSig, const RtGenericContext* genericContext)
{
    bool any_changed = false;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, inflated_return_type, inflate_typesig(methodSig->return_type, genericContext));
    if (inflated_return_type != methodSig->return_type)
    {
        any_changed = true;
    }
    utils::Vector<const RtTypeSig*> inflated_params(methodSig->params.size());
    for (size_t i = 0; i < methodSig->params.size(); ++i)
    {
        const RtTypeSig* old_param = methodSig->params[i];
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, inflated_param, inflate_typesig(old_param, genericContext));
        if (inflated_param != old_param)
        {
            any_changed = true;
        }
        inflated_params[i] = inflated_param;
    }
    if (!any_changed)
    {
        RET_OK(methodSig);
    }
    RtMethodSig* new_method_sig = new (alloc::MetadataAllocation::malloc_any_zeroed<RtMethodSig>())
        RtMethodSig{methodSig->flags, methodSig->generic_param_count, inflated_return_type, std::move(inflated_params)};
    RET_OK(new_method_sig);
}

} // namespace metadata
} // namespace leanclr

#include "managed_layouts.h"
#include <algorithm>
#include <cstring>
#include "core/stl_compat.h"

#include "vm/method.h"

#include "vm/class.h"
#include "vm/runtime.h"
#include "vm/type.h"
#include "vm/generic_method.h"
#include "vm/customattribute.h"
#include "vm/rt_string.h"
#include "vm/object.h"
#include "vm/rt_array.h"
#include "vm/reflection.h"
#include "metadata/generic_metadata.h"
#include "metadata/metadata_cache.h"
#include "metadata/metadata_compare.h"
#include "metadata/module_def.h"
#include "interp/interp_defs.h"

namespace leanclr
{
namespace vm
{
using namespace metadata;
using namespace core;
using namespace utils;

static RtResult<RtReflectionExceptionHandlingClause*>
create_reflection_exceptionhandlingclause(RtModuleDef* mod, const metadata::RtGenericContainerContext& gcc, const metadata::RtGenericContext* gc,
                                          RtReflectionMethodBody* method_body, const metadata::RtExceptionClause& clause)
{
    metadata::RtClass* cls_exceptionhandlingclause = Class::get_corlib_types().cls_reflection_exceptionhandlingclause;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, clause_obj_base,
                                            LEANCLR_NEWOBJ_INTERNAL(cls_exceptionhandlingclause, "Method::create_reflection_exceptionhandlingclause"));
    RtReflectionExceptionHandlingClause* clause_obj = static_cast<RtReflectionExceptionHandlingClause*>(clause_obj_base);

    clause_obj->method_body = method_body;
    if (clause.flags == metadata::RtILExceptionClauseType::Exception)
    {
        if (clause.class_token_or_filter_offset != 0)
        {
            metadata::RtToken token = metadata::RtToken::decode(clause.class_token_or_filter_offset);
            RET_ERR_ON_FAIL(mod->get_class_by_type_def_ref_spec_token(token, gcc, gc));
            clause_obj->catch_metadata_token = static_cast<int32_t>(clause.class_token_or_filter_offset);
        }
    }
    else if (clause.flags == metadata::RtILExceptionClauseType::Filter)
    {
        clause_obj->filter_offset = static_cast<int32_t>(clause.class_token_or_filter_offset);
    }

    clause_obj->flags = static_cast<int32_t>(clause.flags);
    clause_obj->try_offset = static_cast<int32_t>(clause.try_offset);
    clause_obj->try_length = static_cast<int32_t>(clause.try_length);
    clause_obj->handler_offset = static_cast<int32_t>(clause.handler_offset);
    clause_obj->handler_length = static_cast<int32_t>(clause.handler_length);

    RET_OK(clause_obj);
}

static RtResult<RtReflectionLocalVariableInfo*> create_reflection_localvariableinfo(const metadata::RtTypeSig* local_var_type_sig, uint16_t position)
{
    metadata::RtClass* cls_localvarinfo = Class::get_corlib_types().cls_reflection_localvariableinfo;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, var_obj_base, LEANCLR_NEWOBJ_INTERNAL(cls_localvarinfo, "Method::create_reflection_localvariableinfo"));
    RtReflectionLocalVariableInfo* var_obj = static_cast<RtReflectionLocalVariableInfo*>(var_obj_base);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, type_obj, Reflection::get_type_reflection_object(local_var_type_sig));
    var_obj->type_ = type_obj;
    var_obj->position = position;
    var_obj->is_pinned = local_var_type_sig->pinned != 0;

    RET_OK(var_obj);
}

RtResult<RtReflectionMethodBody*> Method::create_reflection_method_body(const RtMethodInfo* method)
{
    RtModuleDef* mod = method->parent->image;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<metadata::RtMethodBody>, raw_method_body_opt, get_method_body(method));

    if (!raw_method_body_opt.has_value())
    {
        RET_OK(nullptr);
    }

    const metadata::RtMethodBody& raw_method_body = raw_method_body_opt.value();
    const vm::CorLibTypes& corlib_types = Class::get_corlib_types();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, method_body_obj_base,
                                            LEANCLR_NEWOBJ_INTERNAL(corlib_types.cls_reflection_methodbody, "Method::create_reflection_method_body"));
    RtReflectionMethodBody* method_body_obj = static_cast<RtReflectionMethodBody*>(method_body_obj_base);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, code_arr,
                                            LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(corlib_types.cls_byte, static_cast<int32_t>(raw_method_body.code_size),
                                                                                        "Method::create_reflection_method_body"));
    if (raw_method_body.code_size > 0)
    {
        std::memcpy(Array::get_array_data_start_as<uint8_t>(code_arr), raw_method_body.code, raw_method_body.code_size);
    }
    method_body_obj->codes = code_arr;

    // Setup generic context
    metadata::RtGenericContainerContext generic_container_context{method->parent->generic_container, method->generic_container};

    const metadata::RtGenericContext* generic_context = nullptr;
    if (method->generic_method != nullptr)
    {
        generic_context = &method->generic_method->generic_context;
    }

    // Create exception clauses array
    const utils::Vector<metadata::RtExceptionClause>& raw_exception_clauses = raw_method_body.exception_clauses;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, clause_arr,
                                            LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(corlib_types.cls_reflection_exceptionhandlingclause,
                                                                                        static_cast<int32_t>(raw_exception_clauses.size()),
                                                                                        "Method::create_reflection_method_body"));
    for (size_t i = 0; i < raw_exception_clauses.size(); ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
            RtReflectionExceptionHandlingClause*, ex_clause_obj,
            create_reflection_exceptionhandlingclause(mod, generic_container_context, generic_context, method_body_obj, raw_exception_clauses[i]));
        Array::set_array_data_at<RtObject*>(clause_arr, static_cast<int32_t>(i), ex_clause_obj);
    }
    method_body_obj->clauses = clause_arr;

    // Create local variables array
    RtArray* locals_arr = nullptr;
    if (raw_method_body.local_var_sig_token != 0)
    {
        utils::Vector<const metadata::RtTypeSig*> local_var_sigs;
        RET_ERR_ON_FAIL(mod->read_local_var_sig(raw_method_body.local_var_sig_token, generic_container_context, generic_context, local_var_sigs));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, localvar_arr,
                                                LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(corlib_types.cls_reflection_localvariableinfo,
                                                                                            static_cast<int32_t>(local_var_sigs.size()),
                                                                                            "Method::create_reflection_method_body"));
        for (size_t i = 0; i < local_var_sigs.size(); ++i)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionLocalVariableInfo*, local_var_obj,
                                                    create_reflection_localvariableinfo(local_var_sigs[i], static_cast<uint16_t>(i)));
            Array::set_array_data_at<RtObject*>(localvar_arr, static_cast<int32_t>(i), local_var_obj);
        }
        locals_arr = localvar_arr;
    }
    else
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
            RtArray*, empty_locals_arr,
            LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS_INTERNAL(corlib_types.cls_reflection_localvariableinfo, "Method::create_reflection_method_body"));
        locals_arr = empty_locals_arr;
    }
    method_body_obj->locals = locals_arr;

    // Set other fields
    method_body_obj->init_locals = (raw_method_body.flags & static_cast<uint16_t>(metadata::RtILMethodFormat::InitLocals)) != 0;
    method_body_obj->sig_token = static_cast<int32_t>(raw_method_body.local_var_sig_token);
    method_body_obj->max_stack = static_cast<int32_t>(raw_method_body.max_stack);

    RET_OK(method_body_obj);
}

} // namespace vm
} // namespace leanclr

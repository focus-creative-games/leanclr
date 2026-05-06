#include <algorithm>
#include <cstring>
#include "core/stl_compat.h"

#include "method.h"

#include "class.h"
#include "runtime.h"
#include "type.h"
#include "generic_method.h"
#include "customattribute.h"
#include "rt_string.h"
#include "object.h"
#include "rt_array.h"
#include "reflection.h"
#include "metadata/generic_metadata.h"
#include "metadata/metadata_cache.h"
#include "metadata/metadata_compare.h"
#include "metadata/module_def.h"
#include "interp/interp_defs.h"

namespace leanclr
{
namespace vm
{
using namespace leanclr::metadata;
using namespace leanclr::core;
using namespace leanclr::utils;

// Helper: compare method signatures (optionally including name)
static bool is_method_signature_equal(const RtMethodInfo* a, const RtMethodInfo* b, bool compareName, bool compareGenericParamByIndex)
{
    if (compareName && std::strcmp(a->name, b->name) != 0)
        return false;

    if (a->parameter_count != b->parameter_count)
        return false;

    if (!MetadataCompare::is_typesig_equal_ignore_attrs(a->return_type, b->return_type, compareGenericParamByIndex))
        return false;

    return MetadataCompare::is_typesigs_equal_ignore_attrs(a->parameters, b->parameters, a->parameter_count, compareGenericParamByIndex);
}

RtResult<const RtMethodInfo*> Method::get_method_by_method_def_gid(uint32_t method_def_gid)
{
    uint32_t module_id = RtMetadata::decode_module_id_from_gid(method_def_gid);
    uint32_t method_rid = RtMetadata::decode_rid_from_gid(method_def_gid);

    RtModuleDef* module = RtModuleDef::get_module_by_id(module_id);
    if (!module)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, method, module->get_method_by_rid(method_rid));
    RET_OK(method);
}

uint32_t Method::get_method_def_gid(const metadata::RtMethodInfo* method)
{
    return RtMetadata::encode_gid_by_rid(*method->parent->image, RtToken::decode_rid(method->token));
}

const RtVirtualInvokeData* Method::get_vtable_method_invoke_data(const RtClass* klass, size_t method_index)
{
    return klass->vtable + method_index;
}

RtManagedMethodPointer Method::get_vtable_method_ptr(const RtClass* klass, size_t method_index)
{
    const RtVirtualInvokeData* data = get_vtable_method_invoke_data(klass, method_index);
    return data->method_impl->method_ptr;
}

const RtMethodInfo* Method::get_vtable_method(const RtClass* klass, size_t method_index)
{
    const RtVirtualInvokeData* data = get_vtable_method_invoke_data(klass, method_index);
    return data->method;
}

RtResult<const RtVirtualInvokeData*> Method::get_interface_method_invoke_data(const RtClass* klass, const RtClass* interface_klass, size_t slot)
{
    const RtInterfaceOffset* offsets = klass->interface_vtable_offsets;
    if (!Class::is_generic_inst(interface_klass))
    {
        for (size_t i = 0; i < klass->interface_vtable_offset_count; ++i)
        {
            const RtInterfaceOffset& off = offsets[i];
            if (off.interface == interface_klass)
            {
                size_t vtable_index = static_cast<size_t>(off.offset) + slot;
                RET_OK(klass->vtable + vtable_index);
            }
        }
    }
    else
    {
        // FIXME: same to il2cpp, but it seems that il2cpp doesn't consider the case where the same generic interface is implemented multiple times with
        // different generic arguments, which is legal in C#. We may need to add more checks here to find the correct interface vtable offset.
        for (int32_t i = klass->interface_vtable_offset_count; i > 0; --i)
        {
            const RtInterfaceOffset& off = offsets[i - 1];
            const RtClass* implemented_interface = off.interface;
            if (implemented_interface != interface_klass)
            {
                if (!Class::is_generic_inst(implemented_interface))
                {
                    continue;
                }
                if (implemented_interface->by_val->data.generic_class->base_type_def_gid != interface_klass->by_val->data.generic_class->base_type_def_gid)
                {
                    continue;
                }
                if (!Class::is_assignable_from_generic_parameter_convariant(implemented_interface, interface_klass, klass))
                {
                    continue;
                }
            }
            size_t vtable_index = static_cast<size_t>(off.offset) + slot;
            RET_OK(klass->vtable + vtable_index);
        }
    }
    RET_ERR(RtErr::MethodAccess);
}

RtResult<const RtMethodInfo*> Method::get_virtual_method_impl(RtObject* obj, const RtMethodInfo* virtual_method)
{
    const RtClass* klass = obj->klass;
    return get_virtual_method_impl_on_klass(klass, virtual_method);
}

RtResult<const RtMethodInfo*> Method::get_virtual_method_impl_on_klass(const RtClass* klass, const RtMethodInfo* virtual_method)
{
    const RtClass* declaring_klass = virtual_method->parent;
    if (declaring_klass == klass || !is_virtual(virtual_method))
    {
        RET_OK(virtual_method);
    }

    size_t slot = virtual_method->slot;
    const RtMethodInfo* actual_method = nullptr;
    if (!Class::is_interface(declaring_klass))
    {
        actual_method = klass->vtable[slot].method_impl;
    }
    else
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtVirtualInvokeData*, invoke_data, get_interface_method_invoke_data(klass, declaring_klass, slot));
        actual_method = invoke_data->method_impl;
    }
    if (actual_method->generic_container)
    {
        const metadata::RtGenericInst* method_inst = virtual_method->generic_method->generic_context.method_inst;
        const metadata::RtGenericMethod* actual_generic_method = actual_method->generic_method;
        if (actual_generic_method)
        {
            const metadata::RtGenericInst* class_inst = actual_generic_method->generic_context.class_inst;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method_def_base,
                                                    get_method_by_method_def_gid(actual_generic_method->base_method_gid));
            return GenericMethod::get_method(method_def_base, class_inst, method_inst);
        }
        else
        {
            return GenericMethod::get_method(actual_method, nullptr, method_inst);
        }
    }

    RET_OK(actual_method);
}

const RtMethodInfo* Method::find_matched_method_in_class(const RtClass* klass, const RtMethodInfo* to_match_method)
{
    const RtMethodInfo** methods = klass->methods;
    for (size_t i = 0; i < klass->method_count; ++i)
    {
        const RtMethodInfo* method = methods[i];
        if (is_method_signature_equal(method, to_match_method, true, false))
        {
            return method;
        }
    }
    return nullptr;
}

const RtMethodInfo* Method::find_matched_method_in_class_by_name_and_signature(const RtClass* klass, const char* name, const RtTypeSig* const* param_type_sigs,
                                                                               size_t param_count)
{
    const RtMethodInfo** methods = klass->methods;
    for (size_t i = 0; i < klass->method_count; ++i)
    {
        const RtMethodInfo* method = methods[i];
        if (std::strcmp(method->name, name) == 0 && method->parameter_count == param_count &&
            MetadataCompare::is_typesigs_equal_ignore_attrs(method->parameters, param_type_sigs, param_count, false))
        {
            return method;
        }
    }
    return nullptr;
}

const RtMethodInfo* Method::find_matched_method_in_class_by_name(const RtClass* klass, const char* name)
{
    const RtMethodInfo** methods = klass->methods;
    for (size_t i = 0; i < klass->method_count; ++i)
    {
        const RtMethodInfo* method = methods[i];
        if (std::strcmp(method->name, name) == 0)
        {
            return method;
        }
    }
    return nullptr;
}

const RtMethodInfo* Method::find_matched_method_in_class_by_name_and_param_count(const RtClass* klass, const char* name, size_t parameter_count)
{
    const RtMethodInfo** methods = klass->methods;
    for (size_t i = 0; i < klass->method_count; ++i)
    {
        const RtMethodInfo* method = methods[i];
        if (std::strcmp(method->name, name) == 0 && method->parameter_count == parameter_count)
        {
            return method;
        }
    }
    return nullptr;
}

bool Method::is_virtual(const RtMethodInfo* method)
{
    return (method->flags & static_cast<uint16_t>(RtMethodAttribute::Virtual)) != 0;
}

bool Method::is_devirtualed(const metadata::RtMethodInfo* method)
{
    return !vm::Method::is_virtual(method) || vm::Method::is_sealed(method) || vm::Class::is_sealed(method->parent);
}

bool Method::is_abstract(const RtMethodInfo* method)
{
    return (method->flags & static_cast<uint16_t>(RtMethodAttribute::Abstract)) != 0;
}

bool Method::is_instance(const RtMethodInfo* method)
{
    return (method->flags & static_cast<uint16_t>(RtMethodAttribute::Static)) == 0;
}

bool Method::is_sealed(const RtMethodInfo* method)
{
    return (method->flags & static_cast<uint16_t>(RtMethodAttribute::Final)) != 0;
}

bool Method::is_new_slot(const RtMethodInfo* method)
{
    return (method->flags & static_cast<uint16_t>(RtMethodAttribute::NewSlot)) != 0;
}

bool Method::is_static(const RtMethodInfo* method)
{
    return (method->flags & static_cast<uint16_t>(RtMethodAttribute::Static)) != 0;
}

bool Method::is_void_return(const RtMethodInfo* method)
{
    return method->return_type->ele_type == RtElementType::Void;
}

bool Method::is_pinvoke(const RtMethodInfo* method)
{
    return (method->flags & static_cast<uint16_t>(RtMethodAttribute::PinvokeImpl)) != 0;
}

bool Method::is_internal_call(const RtMethodInfo* method)
{
    return (method->iflags & static_cast<uint16_t>(RtMethodImplAttribute::InternalCall)) != 0;
}

bool Method::is_runtime_implemented(const RtMethodInfo* method)
{
    return (method->iflags & static_cast<uint16_t>(RtMethodImplAttribute::Runtime)) != 0;
}

bool Method::is_public(const RtMethodInfo* method)
{
    return (method->flags & static_cast<uint16_t>(RtMethodAttribute::MemberAccessMask)) == static_cast<uint16_t>(RtMethodAttribute::Public);
}

bool Method::is_private(const RtMethodInfo* method)
{
    return (method->flags & static_cast<uint16_t>(RtMethodAttribute::MemberAccessMask)) == static_cast<uint16_t>(RtMethodAttribute::Private);
}

bool Method::is_runtime_special_method(const RtMethodInfo* method)
{
    return (method->flags & static_cast<uint16_t>(RtMethodAttribute::RtSpecialName)) != 0;
}

bool Method::is_ctor_or_cctor(const RtMethodInfo* method)
{
    const char* name = method->name;
    return is_runtime_special_method(method) && (std::strcmp(name, ".ctor") == 0 || std::strcmp(name, ".cctor") == 0);
}

bool Method::is_ctor(const RtMethodInfo* method)
{
    const char* name = method->name;
    return is_runtime_special_method(method) && (std::strcmp(name, ".ctor") == 0);
}

RtMethodImplAttribute Method::get_code_type(const RtMethodInfo* method)
{
    return static_cast<RtMethodImplAttribute>(method->iflags & RT_CODE_TYPE_MASK);
}

bool Method::has_method_body(const RtMethodInfo* method)
{
    RtModuleDef* mod = method->parent->image;
    uint32_t methodRid = metadata::RtToken::decode_rid(method->token);
    auto optMethod = mod->get_cli_image().read_method(methodRid);
    return optMethod.has_value() && optMethod->rva != 0;
}

bool Method::has_this(const metadata::RtMethodSig* method_sig)
{
    return method_sig->flags & static_cast<uint16_t>(metadata::RtSigType::HasThis);
}

size_t Method::get_param_count_include_this(const RtMethodInfo* method)
{
    return static_cast<size_t>(method->parameter_count) + (is_instance(method) ? 1 : 0);
}

size_t Method::get_param_count_exclude_this(const RtMethodInfo* method)
{
    return method->parameter_count;
}

uint8_t Method::get_generic_param_count(const RtMethodInfo* method)
{
    if (method->generic_container)
    {
        return method->generic_container->generic_param_count;
    }
    if (method->generic_method && method->generic_method->generic_context.method_inst)
    {
        return method->generic_method->generic_context.method_inst->generic_arg_count;
    }
    return 0;
}

bool Method::contains_not_instantiated_generic_param(const RtMethodInfo* method)
{
    if (method->generic_container)
    {
        return true;
    }
    const RtClass* klass = method->parent;
    if (klass->generic_container || Type::contains_generic_param(method->return_type))
    {
        return true;
    }
    if (method->generic_method)
    {
        auto gc = method->generic_method->generic_context;
        return (gc.class_inst && Type::contains_not_instantiated_generic_param_in_generic_inst(gc.class_inst)) ||
               (gc.method_inst && Type::contains_not_instantiated_generic_param_in_generic_inst(gc.method_inst));
    }
    return false;
}

size_t Method::get_total_arg_stack_object_size(const metadata::RtMethodInfo* method)
{
    return method->total_arg_stack_object_size;
}

size_t Method::get_return_value_stack_object_size(const metadata::RtMethodInfo* method)
{
    return method->ret_stack_object_size;
}

RtResult<bool> Method::is_intrinsic(const RtMethodInfo* method)
{
    metadata::RtClass* intrinsic_attribute_klass = Class::get_corlib_types().cls_intrinsic;
    return CustomAttribute::has_customattribute_on_method(method, intrinsic_attribute_klass);
}

RtResultVoid Method::build_method_arg_descs(RtMethodInfo* method)
{
    assert(method->arg_descs == nullptr && "Method arg descs already built");
    if (Method::contains_not_instantiated_generic_param(method))
    {
        RET_VOID_OK();
    }
    RtModuleDef* mod = method->parent->image;
    size_t totalParamCountExcludeThis = get_param_count_exclude_this(method);
    size_t totalStackObjectSize = is_instance(method);
    if (totalParamCountExcludeThis > 0)
    {
        metadata::RtMethodArgDesc* descs = mod->get_mem_pool().calloc_any<metadata::RtMethodArgDesc>(totalParamCountExcludeThis);
        for (size_t i = 0; i < totalParamCountExcludeThis; ++i)
        {
            const metadata::RtTypeSig* paramTypeSig = method->parameters[i];
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, reduceTypeAndSize,
                                                    interp::InterpDefs::get_reduce_type_and_size_by_typesig(paramTypeSig));
            descs[i].reduce_type = reduceTypeAndSize.reduce_type;
            size_t stackObjectSize = interp::InterpDefs::get_stack_object_size_by_byte_size(reduceTypeAndSize.byte_size);
            descs[i].stack_object_size = static_cast<uint16_t>(stackObjectSize);
            totalStackObjectSize += stackObjectSize;
        }
        method->arg_descs = descs;
    }
    method->total_arg_stack_object_size = static_cast<uint16_t>(totalStackObjectSize);
    if (method->return_type->is_void())
    {
        method->ret_stack_object_size = 0;
    }
    else
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, reduceTypeAndSize,
                                                interp::InterpDefs::get_reduce_type_and_size_by_typesig(method->return_type));
        size_t retStackObjectSize = interp::InterpDefs::get_stack_object_size_by_byte_size(reduceTypeAndSize.byte_size);
        method->ret_stack_object_size = static_cast<uint16_t>(retStackObjectSize);
    }

    RET_VOID_OK();
}

size_t Method::get_method_index_in_class(const RtMethodInfo* method)
{
    const RtClass* parent = method->parent;
    const RtMethodInfo** methods = parent->methods;
    if (method->token != 0)
    {
        return static_cast<size_t>(method->token - methods[0]->token);
    }
    for (size_t i = 0; i < parent->method_count; ++i)
    {
        if (methods[i] == method)
        {
            return i;
        }
    }
    return static_cast<size_t>(-1);
}

RtResult<const RtMethodInfo*> Method::inflate_method(const RtMethodInfo* method, const RtGenericContext* gc)
{
    if (!gc)
    {
        RET_OK(method);
    }
    const metadata::RtClass* klass = method->parent;
    if (!vm::Class::is_generic(klass) && !vm::Class::is_generic_inst(klass) && !method->generic_container)
    {
        assert(!klass->generic_container);
        assert(!method->generic_method);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, inflatedKlass, vm::GenericMetadata::inflate_class(klass, gc));
        if (inflatedKlass == klass)
        {
            RET_OK(method);
        }
        else
        {
            size_t methodIndex = get_method_index_in_class(method);
            const RtMethodInfo* inflatedMethod = inflatedKlass->methods[methodIndex];
            RET_OK(inflatedMethod);
        }
    }
    const metadata::RtGenericMethod* genericMethod = method->generic_method;
    const metadata::RtMethodInfo* baseMethod;
    const metadata::RtGenericInst* oldClassInst;
    const metadata::RtGenericInst* oldMethodInst;
    if (genericMethod)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(baseMethod, get_method_by_method_def_gid(genericMethod->base_method_gid));
        oldClassInst = genericMethod->generic_context.class_inst;
        oldMethodInst = genericMethod->generic_context.method_inst;
    }
    else
    {
        baseMethod = method;
        oldClassInst = nullptr;
        oldMethodInst = nullptr;
    }
    const metadata::RtGenericInst* newClassInst;
    if (vm::Class::is_generic(baseMethod->parent))
    {
        if (oldClassInst)
        {
            UNWRAP_OR_RET_ERR_ON_FAIL(newClassInst, metadata::GenericMetadata::inflate_generic_inst(oldClassInst, gc));
        }
        else
        {
            assert(gc->class_inst && "Class generic instance must be provided in generic context");
            newClassInst = gc->class_inst;
        }
    }
    else
    {
        newClassInst = nullptr;
    }

    const metadata::RtGenericInst* newMethodInst;
    if (baseMethod->generic_container)
    {
        if (oldMethodInst)
        {
            UNWRAP_OR_RET_ERR_ON_FAIL(newMethodInst, metadata::GenericMetadata::inflate_generic_inst(oldMethodInst, gc));
        }
        else
        {
            // assert(gc->method_inst && "Method generic instance must be provided in generic context");
            newMethodInst = gc->method_inst;
        }
    }
    else
    {
        newMethodInst = nullptr;
    }
    if (oldClassInst == newClassInst && oldMethodInst == newMethodInst)
    {
        RET_OK(method);
    }
    uint32_t baseMethodGid = get_method_def_gid(baseMethod);
    const metadata::RtGenericMethod* newGenericMethod = MetadataCache::get_pooled_generic_method(baseMethodGid, newClassInst, newMethodInst);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, inflatedMethod,
                                            GenericMethod::get_method_from_pooled_generic_method(newGenericMethod));
    RET_OK(inflatedMethod);
}

RtResult<const RtMethodInfo*> Method::inflate(const RtMethodInfo* method_info, const RtGenericContext* gc)
{
    return inflate_method(method_info, gc);
}

RtResult<std::optional<uint32_t>> Method::get_parameter_token(const RtMethodInfo* method, int32_t index)
{
    if (index < -1)
    {
        RET_ERR(RtErr::Argument);
    }

    RtModuleDef* mod = method->parent->image;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, token, mod->get_parameter_token(method->token, index));
    if (token == 0)
    {
        RET_OK(std::optional<uint32_t>{});
    }
    RET_OK(std::optional<uint32_t>{token});
}

RtResult<RtString*> Method::get_parameter_name_by_token(RtModuleDef* mod, metadata::EncodedTokenId param_token)
{
    uint32_t rid = metadata::RtToken::decode_rid(param_token);
    auto opt_param_row = mod->get_cli_image().read_param(rid);
    if (!opt_param_row)
    {
        RET_OK(nullptr);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, name, mod->get_string(opt_param_row->name));
    RtString* managed_name = String::create_string_from_utf8cstr(name);
    RET_OK(managed_name);
}

RtResult<const char*> Method::get_parameter_c_name_by_token(RtModuleDef* mod, metadata::EncodedTokenId param_token)
{
    uint32_t rid = metadata::RtToken::decode_rid(param_token);
    auto opt_param_row = mod->get_cli_image().read_param(rid);
    if (!opt_param_row)
    {
        RET_OK(nullptr);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, name, mod->get_string(opt_param_row->name));
    RET_OK(name);
}

RtResult<std::optional<RowImplMap>> Method::get_imp_map_info(const RtMethodInfo* method)
{
    RtModuleDef* ass = method->parent->image;
    return ass->read_method_impl_map(method->token);
}

RtResult<std::optional<RtMethodBody>> Method::get_method_body(const RtMethodInfo* method)
{
    RtModuleDef* ass = method->parent->image;
    return ass->read_method_body(method->token);
}

static RtResult<RtReflectionExceptionHandlingClause*> create_reflection_exceptionhandlingclause(RtModuleDef* mod,
                                                                                                const metadata::RtGenericContainerContext& gcc,
                                                                                                const metadata::RtGenericContext* gc,
                                                                                                const metadata::RtExceptionClause& clause)
{
    metadata::RtClass* cls_exceptionhandlingclause = Class::get_corlib_types().cls_reflection_exceptionhandlingclause;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, clause_obj_base, Object::new_object(cls_exceptionhandlingclause));
    RtReflectionExceptionHandlingClause* clause_obj = static_cast<RtReflectionExceptionHandlingClause*>(clause_obj_base);

    if (clause.flags == metadata::RtILExceptionClauseType::Exception)
    {
        if (clause.class_token_or_filter_offset != 0)
        {
            metadata::RtToken token = metadata::RtToken::decode(clause.class_token_or_filter_offset);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, catch_klass, mod->get_class_by_type_def_ref_spec_token(token, gcc, gc));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, catch_type, Reflection::get_klass_reflection_object(catch_klass));
            clause_obj->catch_type = catch_type;
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
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, var_obj_base, Object::new_object(cls_localvarinfo));
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
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, method_body_obj_base, Object::new_object(corlib_types.cls_reflection_methodbody));
    RtReflectionMethodBody* method_body_obj = static_cast<RtReflectionMethodBody*>(method_body_obj_base);

    // Setup generic context
    metadata::RtGenericContainerContext generic_container_context{method->parent->generic_container, method->generic_container};

    const metadata::RtGenericContext* generic_context = nullptr;
    if (method->generic_method != nullptr)
    {
        generic_context = &method->generic_method->generic_context;
    }

    // Create exception clauses array
    const utils::Vector<metadata::RtExceptionClause>& raw_exception_clauses = raw_method_body.exception_clauses;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        RtArray*, clause_arr,
        Array::new_szarray_from_ele_klass(corlib_types.cls_reflection_exceptionhandlingclause, static_cast<int32_t>(raw_exception_clauses.size())));
    for (size_t i = 0; i < raw_exception_clauses.size(); ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
            RtReflectionExceptionHandlingClause*, ex_clause_obj,
            create_reflection_exceptionhandlingclause(mod, generic_container_context, generic_context, raw_exception_clauses[i]));
        Array::set_array_data_at<RtObject*>(clause_arr, static_cast<int32_t>(i), ex_clause_obj);
    }
    method_body_obj->clauses = clause_arr;

    // Create local variables array
    RtArray* locals_arr = nullptr;
    if (raw_method_body.local_var_sig_token != 0)
    {
        utils::Vector<const metadata::RtTypeSig*> local_var_sigs;
        RET_ERR_ON_FAIL(mod->read_local_var_sig(raw_method_body.local_var_sig_token, generic_container_context, generic_context, local_var_sigs));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
            RtArray*, localvar_arr,
            Array::new_szarray_from_ele_klass(corlib_types.cls_reflection_localvariableinfo, static_cast<int32_t>(local_var_sigs.size())));
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
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, empty_locals_arr,
                                                Array::new_empty_szarray_by_ele_klass(corlib_types.cls_reflection_localvariableinfo));
        locals_arr = empty_locals_arr;
    }
    method_body_obj->locals = locals_arr;

    // Set other fields
    method_body_obj->init_locals = (raw_method_body.flags & static_cast<uint16_t>(metadata::RtILMethodFormat::InitLocals)) != 0;
    method_body_obj->sig_token = static_cast<int32_t>(raw_method_body.local_var_sig_token);
    method_body_obj->max_stack = static_cast<int32_t>(raw_method_body.max_stack);

    RET_OK(method_body_obj);
}

RtResultVoid Method::get_parameter_modifiers(const RtMethodInfo* method, int32_t index, bool optional, utils::Vector<RtClass*>& modifiers)
{
    if (index < -1)
    {
        RET_ERR(RtErr::Argument);
    }
    metadata::RtModuleDef* mod = method->parent->image;
    auto optMethodRow = mod->get_cli_image().read_method(metadata::RtToken::decode_rid(method->token));
    assert(optMethodRow.has_value() && "Method row must be present");
    auto retBlobReader = mod->get_decoded_blob_reader(optMethodRow->signature);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(utils::BinaryReader, reader, retBlobReader);
    return mod->read_parameter_modifier(reader, index, optional, RtGenericContainerContext{}, nullptr, modifiers);
}

} // namespace vm
} // namespace leanclr

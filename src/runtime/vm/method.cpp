#include <algorithm>
#include <cstring>
#include "core/stl_compat.h"

#include "method.h"
#include "metadata/native_handle_registry.h"

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

    if (Class::is_generic_inst(interface_klass))
    {
        for (int32_t i = klass->interface_vtable_offset_count; i > 0; --i)
        {
            const RtInterfaceOffset& off = offsets[i - 1];
            const RtClass* implemented_interface = off.interface;
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
            size_t vtable_index = static_cast<size_t>(off.offset) + slot;
            RET_OK(klass->vtable + vtable_index);
        }
    }

    for (size_t i = 0; i < klass->interface_vtable_offset_count; ++i)
    {
        const RtInterfaceOffset& off = offsets[i];
        if (off.interface == interface_klass)
        {
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

#if defined(LEANCLR_BCL_CORECLR_NET10)
RtResult<const RtMethodInfo*> Method::resolve_static_interface_method(RtClass* klass, const RtMethodInfo* declaration)
{
    if (!klass || !declaration || !is_static(declaration) || !Class::is_interface(declaration->parent))
        RET_ERR(RtErr::BadImageFormat);
    RET_ERR_ON_FAIL(Class::initialize_methods(klass));
    const RtClass* definition = klass;
    RtGenericContext context{};
    if (declaration->generic_method)
        context.method_inst = declaration->generic_method->generic_context.method_inst;
    if (Class::is_generic_inst(klass))
    {
        const auto* generic = klass->by_val->data.generic_class;
        UNWRAP_OR_RET_ERR_ON_FAIL(definition, Class::get_class_by_type_def_gid(generic->base_type_def_gid));
        context.class_inst = generic->class_inst;
    }
    const auto& image = definition->image->get_cli_image();
    auto range = image.find_row_range_of_owner_at_sorted_table(TableType::MethodImpl, 0, RtToken::decode_rid(definition->token));
    if (range)
    {
        auto gcc = Class::get_generic_container_context(definition);
        for (uint32_t rid = range->ridBegin; rid < range->ridEnd; ++rid)
        {
            auto row = image.read_method_impl(rid);
            if (!row)
                RET_ERR(RtErr::BadImageFormat);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
                const RtMethodInfo*, declared,
                definition->image->get_method_by_token(RtMetadata::decode_method_def_or_ref_coded_index(row->method_declaration), gcc, &context));
            if (context.method_inst && declared->generic_container)
                UNWRAP_OR_RET_ERR_ON_FAIL(declared, inflate(declared, &context));
            if (declared->parent != declaration->parent || !MetadataCompare::is_method_signature_equal(declared, declaration, true, true))
                continue;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
                const RtMethodInfo*, body,
                definition->image->get_method_by_token(RtMetadata::decode_method_def_or_ref_coded_index(row->method_body), gcc, &context));
            if (context.class_inst || context.method_inst)
                UNWRAP_OR_RET_ERR_ON_FAIL(body, inflate(body, &context));
            if (!is_static(body) || is_abstract(body))
                RET_ERR(RtErr::BadImageFormat);
            RET_OK(body);
        }
    }
    const RtMethodInfo* match = nullptr;
    for (uint16_t i = 0; i < klass->method_count; ++i)
    {
        const auto* candidate = klass->methods[i];
        if (context.method_inst && candidate->generic_container)
        {
            if (candidate->generic_container->generic_param_count != context.method_inst->generic_arg_count)
                continue;
            UNWRAP_OR_RET_ERR_ON_FAIL(candidate, inflate(candidate, &context));
        }
        if (is_static(candidate) && is_public(candidate) && !is_abstract(candidate) &&
            MetadataCompare::is_method_signature_equal(candidate, declaration, true, true))
        {
            if (match)
                RET_ERR(RtErr::BadImageFormat);
            match = candidate;
        }
    }
    if (match)
        RET_OK(match);
    if (klass->parent)
        return resolve_static_interface_method(const_cast<RtClass*>(klass->parent), declaration);
    if (!is_abstract(declaration))
        RET_OK(declaration);
    RET_ERR(RtErr::MissingMethod);
}
#endif

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
#if defined(LEANCLR_BCL_CORECLR_NET10)
    if (!NativeHandleRegistry::insert(method, NativeHandleKind::Method))
        RET_ERR(RtErr::OutOfMemory);
#endif
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
    if (method->token == 0)
    {
        RET_OK(std::optional<uint32_t>{});
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

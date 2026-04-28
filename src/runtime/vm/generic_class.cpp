#include <cstddef>
#include <cassert>

#include "generic_class.h"
#include "class.h"
#include "generic_method.h"
#include "method.h"
#include "metadata/generic_metadata.h"
#include "metadata/metadata_cache.h"
#include "metadata/module_def.h"
#include "alloc/metadata_allocation.h"

namespace leanclr
{
namespace vm
{
using namespace leanclr::metadata;
using namespace leanclr::core;
using namespace leanclr::alloc;

// Helper: get class from not pooled generic class
static RtResult<RtClass*> get_class_from_not_pooled_generic_class(const RtGenericClass* genericClass)
{
    uint32_t module_id = RtMetadata::decode_module_id_from_gid(genericClass->base_type_def_gid);
    uint32_t rid = RtMetadata::decode_rid_from_gid(genericClass->base_type_def_gid);
    RtModuleDef* module = RtModuleDef::get_module_by_id(module_id);
    if (!module)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, result, GenericClass::get_class(genericClass->base_type_def_gid, genericClass->class_inst));
    RET_OK(result);
}

// Helper: Get class from pooled generic class
static RtResult<RtClass*> get_class_from_pooled_generic_class(const RtGenericClass* genericClass)
{
    if (genericClass->cache_klass)
    {
        RET_OK(const_cast<RtClass*>(genericClass->cache_klass));
    }

    RtClass* new_class = MetadataAllocation::malloc_any_zeroed<RtClass>();
    const_cast<RtGenericClass*>(genericClass)->cache_klass = new_class;

    RtGenericContext generic_context{genericClass->class_inst, nullptr};
    uint32_t base_type_def_gid = genericClass->base_type_def_gid;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, base_generic_class, Class::get_class_by_type_def_gid(base_type_def_gid));
    const_cast<RtGenericClass*>(genericClass)->cache_base_klass = base_generic_class;
    RtModuleDef* ass = base_generic_class->image;

    RtClass* parent_class = nullptr;
    if (base_generic_class->parent)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, parent_type_sig,
                                                GenericMetadata::inflate_typesig(base_generic_class->parent->by_val, &generic_context));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, parent_klass, Class::get_class_from_typesig(parent_type_sig));
        parent_class = parent_klass;
    }

    new_class->image = base_generic_class->image;
    new_class->token = base_generic_class->token;
    new_class->parent = parent_class;
    new_class->namespaze = base_generic_class->namespaze;
    new_class->name = base_generic_class->name;

    new_class->by_val = &genericClass->by_val_type_sig;
    new_class->by_ref = &genericClass->by_ref_type_sig;

    new_class->element_class = new_class;
    new_class->cast_class = new_class;

    new_class->flags = base_generic_class->flags;
    new_class->extra_flags = base_generic_class->extra_flags;

    // Handle special cases: Nullable and Enum
    if (base_generic_class == Class::get_corlib_types().cls_nullable)
    {
        new_class->extra_flags |= static_cast<uint32_t>(RtClassExtraAttribute::Nullable);
        const RtTypeSig* ele_type_sig = genericClass->class_inst->generic_args[0];
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, ele_klass, Class::get_class_from_typesig(ele_type_sig));
        new_class->cast_class = ele_klass;
        new_class->element_class = ele_klass;
    }
    else if (Class::is_enum_type(base_generic_class))
    {
        new_class->extra_flags |= static_cast<uint32_t>(RtClassExtraAttribute::Enum);
        new_class->cast_class = base_generic_class->cast_class;
        new_class->element_class = base_generic_class->element_class;
    }

    RET_OK(new_class);
}

RtResult<RtClass*> GenericClass::get_class(uint32_t baseTypeDefGid, const RtGenericInst* classInst)
{
    const RtGenericClass* generic_class = MetadataCache::get_pooled_generic_class(baseTypeDefGid, classInst);
    return get_class_from_pooled_generic_class(generic_class);
}

RtResult<RtClass*> GenericClass::get_base_class(const RtGenericClass* genericClass)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, base_class, Class::get_class_by_type_def_gid(genericClass->base_type_def_gid));
    RET_OK(base_class);
}

RtResultVoid GenericClass::setup_fields(RtClass* klass)
{
    const RtGenericClass* generic_class = klass->by_val->data.generic_class;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, base_generic_class, Class::get_class_by_type_def_gid(generic_class->base_type_def_gid));
    RET_ERR_ON_FAIL(Class::initialize_fields(base_generic_class));

    klass->field_count = base_generic_class->field_count;
    alloc::MemPool& pool = klass->image->get_mem_pool();
    if (base_generic_class->fields)
    {
        size_t field_count = base_generic_class->field_count;
        RtFieldInfo* fields = pool.calloc_any<RtFieldInfo>(field_count);

        RtGenericContext generic_context{generic_class->class_inst, nullptr};
        for (size_t i = 0; i < field_count; ++i)
        {
            const RtFieldInfo* base_field = base_generic_class->fields + i;
            RtFieldInfo* new_field = fields + i;
            new_field->parent = klass;
            new_field->name = base_field->name;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, inflated_type, GenericMetadata::inflate_typesig(base_field->type_sig, &generic_context));
            new_field->type_sig = inflated_type;
            new_field->flags = base_field->flags;
            new_field->offset = base_field->offset;
            new_field->token = base_field->token;
        }
        klass->fields = fields;
    }
    else
    {
        klass->fields = nullptr;
    }
    RET_VOID_OK();
}

RtResultVoid GenericClass::setup_interfaces(RtClass* klass)
{
    const RtGenericClass* generic_class = klass->by_val->data.generic_class;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, base_generic_class, Class::get_class_by_type_def_gid(generic_class->base_type_def_gid));
    RET_ERR_ON_FAIL(Class::initialize_interfaces(base_generic_class));

    klass->interface_count = base_generic_class->interface_count;
    alloc::MemPool& pool = klass->image->get_mem_pool();
    if (base_generic_class->interfaces)
    {
        size_t interface_count = base_generic_class->interface_count;
        const RtClass** interfaces = pool.calloc_any<const RtClass*>(interface_count);

        RtGenericContext generic_context{generic_class->class_inst, nullptr};
        for (size_t i = 0; i < interface_count; ++i)
        {
            const RtClass* base_interface = base_generic_class->interfaces[i];
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, interface_type_sig,
                                                    GenericMetadata::inflate_typesig(base_interface->by_val, &generic_context));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, inflated_interface, Class::get_class_from_typesig(interface_type_sig));
            RET_ERR_ON_FAIL(Class::initialize_all(inflated_interface));
            interfaces[i] = inflated_interface;
        }
        klass->interfaces = interfaces;
    }
    else
    {
        klass->interfaces = nullptr;
    }
    RET_VOID_OK();
}

RtResultVoid GenericClass::setup_methods(RtClass* klass)
{
    const RtGenericClass* generic_class = klass->by_val->data.generic_class;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, base_generic_class, Class::get_class_by_type_def_gid(generic_class->base_type_def_gid));
    RET_ERR_ON_FAIL(Class::initialize_methods(base_generic_class));

    // Propagate method mask extra flags
    klass->extra_flags |= base_generic_class->extra_flags & static_cast<uint32_t>(RtClassExtraAttribute::MethodMask);

    if (base_generic_class->method_count > 0)
    {
        size_t method_count = base_generic_class->method_count;
        alloc::MemPool& pool = klass->image->get_mem_pool();
        const RtMethodInfo** methods = pool.calloc_any<const RtMethodInfo*>(method_count);

        for (size_t i = 0; i < method_count; ++i)
        {
            const RtMethodInfo* base_method = base_generic_class->methods[i];
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, inflated_method,
                                                    GenericMethod::get_method(base_method, generic_class->class_inst, nullptr));
            assert(klass == inflated_method->parent);
            methods[i] = inflated_method;
        }
        klass->methods = methods;
        klass->method_count = base_generic_class->method_count;
    }
    RET_VOID_OK();
}

RtResultVoid GenericClass::setup_properties(RtClass* klass)
{
    const RtGenericClass* generic_class = klass->by_ref->data.generic_class;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, base_generic_class, Class::get_class_by_type_def_gid(generic_class->base_type_def_gid));
    RET_ERR_ON_FAIL(Class::initialize_properties(base_generic_class));

    if (base_generic_class->property_count > 0)
    {
        size_t property_count = base_generic_class->property_count;
        alloc::MemPool& pool = klass->image->get_mem_pool();
        RtPropertyInfo* properties = pool.calloc_any<RtPropertyInfo>(property_count);

        RtGenericContext generic_context{generic_class->class_inst, nullptr};
        for (size_t i = 0; i < property_count; ++i)
        {
            const RtPropertyInfo* base_property = base_generic_class->properties + i;
            RtPropertyInfo* new_property = properties + i;

            new_property->parent = klass;
            new_property->name = base_property->name;
            new_property->token = base_property->token;
            new_property->flags = base_property->flags;

            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, property_type_sig,
                                                    GenericMetadata::inflate_typesig(base_property->property_sig.type_sig, &generic_context));
            new_property->property_sig.type_sig = property_type_sig;
            // new_property->property_sig.flags = base_property->property_sig.flags;
            size_t param_count = base_property->property_sig.params.size();
            if (param_count > 0)
            {
                new_property->property_sig.params.resize(param_count);
                const RtTypeSig** params = new_property->property_sig.params.data();
                for (size_t j = 0; j < param_count; ++j)
                {
                    const RtTypeSig* base_param = base_property->property_sig.params[j];
                    UNWRAP_OR_RET_ERR_ON_FAIL(params[j], GenericMetadata::inflate_typesig(base_param, &generic_context));
                }
            }

            if (base_property->get_method)
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, get_method,
                                                        GenericMethod::get_method(base_property->get_method, generic_class->class_inst, nullptr));
                new_property->get_method = get_method;
            }
            else
            {
                new_property->get_method = nullptr;
            }

            if (base_property->set_method)
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, set_method,
                                                        GenericMethod::get_method(base_property->set_method, generic_class->class_inst, nullptr));
                new_property->set_method = set_method;
            }
            else
            {
                new_property->set_method = nullptr;
            }
        }
        klass->properties = properties;
        klass->property_count = base_generic_class->property_count;
    }
    RET_VOID_OK();
}

RtResultVoid GenericClass::setup_events(RtClass* klass)
{
    const RtGenericClass* generic_class = klass->by_ref->data.generic_class;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, base_generic_class, Class::get_class_by_type_def_gid(generic_class->base_type_def_gid));
    RET_ERR_ON_FAIL(Class::initialize_events(base_generic_class));

    if (base_generic_class->event_count > 0)
    {
        size_t event_count = base_generic_class->event_count;
        alloc::MemPool& pool = klass->image->get_mem_pool();
        RtEventInfo* events = pool.calloc_any<RtEventInfo>(event_count);

        RtGenericContext generic_context{generic_class->class_inst, nullptr};
        for (size_t i = 0; i < event_count; ++i)
        {
            const RtEventInfo* base_event = base_generic_class->events + i;
            RtEventInfo* new_event = events + i;

            new_event->parent = klass;
            new_event->name = base_event->name;
            new_event->token = base_event->token;
            new_event->flags = base_event->flags;

            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, event_type_sig, GenericMetadata::inflate_typesig(base_event->type_sig, &generic_context));
            new_event->type_sig = event_type_sig;

            if (base_event->add_method)
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, add_method,
                                                        GenericMethod::get_method(base_event->add_method, generic_class->class_inst, nullptr));
                new_event->add_method = add_method;
            }
            else
            {
                new_event->add_method = nullptr;
            }

            if (base_event->remove_method)
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, remove_method,
                                                        GenericMethod::get_method(base_event->remove_method, generic_class->class_inst, nullptr));
                new_event->remove_method = remove_method;
            }
            else
            {
                new_event->remove_method = nullptr;
            }

            if (base_event->raise_method)
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, raise_method,
                                                        GenericMethod::get_method(base_event->raise_method, generic_class->class_inst, nullptr));
                new_event->raise_method = raise_method;
            }
            else
            {
                new_event->raise_method = nullptr;
            }
        }
        klass->events = events;
        klass->event_count = base_generic_class->event_count;
    }
    RET_VOID_OK();
}

RtResultVoid GenericClass::setup_vtables(RtClass* klass)
{
    const RtGenericClass* generic_class = klass->by_val->data.generic_class;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, base_generic_class, Class::get_class_by_type_def_gid(generic_class->base_type_def_gid));
    RET_ERR_ON_FAIL(Class::initialize_vtables(base_generic_class));

    // Set method slots
    for (size_t i = 0; i < klass->method_count; ++i)
    {
        const RtMethodInfo* base_method = base_generic_class->methods[i];
        RtMethodInfo* inflated_method = const_cast<RtMethodInfo*>(klass->methods[i]);
        inflated_method->slot = base_method->slot;
    }

    alloc::MemPool& pool = klass->image->get_mem_pool();
    // Setup vtable
    if (base_generic_class->vtable_count > 0)
    {
        size_t vtable_count = base_generic_class->vtable_count;
        RtVirtualInvokeData* vtable = pool.calloc_any<RtVirtualInvokeData>(vtable_count);

        for (size_t i = 0; i < vtable_count; ++i)
        {
            const RtVirtualInvokeData* base_invoke_data = base_generic_class->vtable + i;
            RtGenericContext gc = {generic_class->class_inst, nullptr};
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, inflated_method, Method::inflate(base_invoke_data->method, &gc));

            const RtMethodInfo* inflated_method_impl = nullptr;
            if (base_invoke_data->method_impl)
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, impl, Method::inflate(base_invoke_data->method_impl, &gc));
                inflated_method_impl = impl;
            }

            vtable[i].method = inflated_method;
            vtable[i].method_impl = inflated_method_impl;
        }
        klass->vtable = vtable;
        klass->vtable_count = base_generic_class->vtable_count;
    }

    // Setup interface vtable offsets
    size_t interface_vtable_offset_count = base_generic_class->interface_vtable_offset_count;
    if (interface_vtable_offset_count > 0)
    {
        RtGenericContext generic_context{generic_class->class_inst, nullptr};
        RtInterfaceOffset* new_interface_table_offsets = pool.calloc_any<RtInterfaceOffset>(interface_vtable_offset_count);

        for (size_t i = 0; i < interface_vtable_offset_count; ++i)
        {
            const RtInterfaceOffset* interface_offset = base_generic_class->interface_vtable_offsets + i;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, inflated_interface_type,
                                                    GenericMetadata::inflate_typesig(interface_offset->interface->by_val, &generic_context));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, inflated_interface_klass, Class::get_class_from_typesig(inflated_interface_type));

            new_interface_table_offsets[i].interface = inflated_interface_klass;
            new_interface_table_offsets[i].offset = interface_offset->offset;
        }
        klass->interface_vtable_offsets = new_interface_table_offsets;
        klass->interface_vtable_offset_count = static_cast<uint16_t>(interface_vtable_offset_count);
    }

    RET_VOID_OK();
}

} // namespace vm
} // namespace leanclr

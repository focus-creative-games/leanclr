#include "generic_method.h"
#include "method.h"
#include "class.h"
#include "generic_class.h"
#include "shim.h"
#include "metadata/generic_metadata.h"
#include "metadata/metadata_cache.h"
#include "metadata/module_def.h"
#include "alloc/metadata_allocation.h"
#include "utils/hashmap.h"
#include "alloc/mem_pool.h"

namespace leanclr
{
namespace vm
{
using namespace leanclr::metadata;
using namespace leanclr::core;
using namespace leanclr::alloc;
using namespace leanclr::utils;

// Static map to cache inflated methods
static HashMap<const RtGenericMethod*, const RtMethodInfo*> g_method_map;

RtResult<const RtMethodInfo*> GenericMethod::get_method(const RtMethodInfo* methodDef, const RtGenericInst* classInst, const RtGenericInst* methodInst)
{
    assert(methodDef != nullptr);
    assert(methodDef->generic_method == nullptr);
    assert(classInst != nullptr || methodInst != nullptr);

    bool need_inflate = false;
    const RtClass* parent = methodDef->parent;
    RtGenericInst* class_inst_mut = const_cast<RtGenericInst*>(classInst);

    // Check if class needs inflation
    if (classInst != nullptr)
    {
        if (Class::is_generic_inst(parent))
        {
            need_inflate = true;
        }
        else if (Class::is_generic(parent))
        {
            need_inflate = true;
        }
        else
        {
            class_inst_mut = nullptr;
        }
    }
    else
    {
        assert(!Class::is_generic_inst(parent));
        assert(!Class::is_generic(parent));
    }

    const RtGenericMethod* original_generic_method = methodDef->generic_method;
    if (methodInst != nullptr)
    {
        if (methodDef->generic_container != nullptr)
        {
            need_inflate = true;
        }
        else if (original_generic_method != nullptr)
        {
            assert(original_generic_method->generic_context.method_inst == nullptr);
            need_inflate = true;
        }
    }
    else
    {
        assert(methodDef->generic_container == nullptr || need_inflate);
    }

    if (!need_inflate)
    {
        RET_OK(methodDef);
    }

    RtModuleDef* image = parent->image;

    const RtGenericMethod* generic_method =
        MetadataCache::get_pooled_generic_method(Method::get_method_def_gid(methodDef), class_inst_mut, const_cast<RtGenericInst*>(methodInst));

    return get_method_from_pooled_generic_method(generic_method);
}

RtResult<const RtMethodInfo*> GenericMethod::get_method_from_pooled_generic_method(const RtGenericMethod* genericMethod)
{
    const RtGenericInst* class_inst = genericMethod->generic_context.class_inst;
    const RtGenericInst* method_inst = genericMethod->generic_context.method_inst;
    assert(class_inst || method_inst);
    auto it = g_method_map.find(genericMethod);
    if (it != g_method_map.end())
    {
        RET_OK(it->second);
    }

    uint32_t base_method_gid = genericMethod->base_method_gid;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtMethodInfo*, base_method, Method::get_method_by_method_def_gid(base_method_gid));

    assert(!class_inst || class_inst->generic_arg_count == base_method->parent->generic_container->generic_param_count);
    assert(!method_inst || method_inst->generic_arg_count == base_method->generic_container->generic_param_count);

    alloc::MemPool& pool = base_method->parent->image->get_mem_pool();
    RtMethodInfo* new_method = pool.malloc_any_zeroed<RtMethodInfo>();
    const RtGenericContext& generic_context = genericMethod->generic_context;

    // Determine parent class (inflated if needed)
    const RtClass* parent_klass = nullptr;
    if (generic_context.class_inst == nullptr)
    {
        parent_klass = base_method->parent;
    }
    else
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, inflated_parent,
                                                GenericClass::get_class(Class::get_type_def_gid(base_method->parent), generic_context.class_inst));
        parent_klass = inflated_parent;
    }

    // Copy basic method information
    new_method->token = base_method->token;
    new_method->parent = parent_klass;
    new_method->name = base_method->name;

    // Inflate return type and parameters
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtTypeSig*, inflated_return_type,
                                            GenericMetadata::inflate_typesig(base_method->return_type, const_cast<RtGenericContext*>(&generic_context)));
    new_method->return_type = inflated_return_type;

    if (base_method->parameter_count > 0)
    {
        size_t param_count = base_method->parameter_count;
        const RtTypeSig** params = pool.calloc_any<const RtTypeSig*>(param_count);
        for (size_t i = 0; i < param_count; ++i)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
                const RtTypeSig*, inflated_param,
                GenericMetadata::inflate_typesig(base_method->parameters[i], const_cast<RtGenericContext*>(&generic_context)));
            params[i] = inflated_param;
        }
        new_method->parameters = params;
    }
    else
    {
        new_method->parameters = nullptr;
    }

    new_method->parameter_count = base_method->parameter_count;
    new_method->flags = base_method->flags;
    new_method->iflags = base_method->iflags;
    new_method->slot = base_method->slot;

    // Handle generic container
    if (base_method->generic_container != nullptr && generic_context.method_inst == nullptr)
    {
        new_method->generic_container = base_method->generic_container;
    }
    else
    {
        new_method->generic_container = nullptr;
    }

    new_method->generic_method = genericMethod;

    // Build method argument descriptors
    RET_ERR_ON_FAIL(Method::build_method_arg_descs(new_method));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(InvokeTypeAndMethod, invoker_type_and_method, Shim::get_invoker(new_method));
    new_method->invoke_method_ptr = invoker_type_and_method.invoker;
    new_method->invoker_type = invoker_type_and_method.invoker_type;
    MethodAndVirtualMethod method_and_virtual_method = Shim::get_method_pointer(new_method);
    new_method->method_ptr = method_and_virtual_method.method_ptr;
    new_method->virtual_method_ptr = method_and_virtual_method.virtual_method_ptr;
    g_method_map.insert({genericMethod, new_method});
    RET_OK(new_method);
}

} // namespace vm
} // namespace leanclr

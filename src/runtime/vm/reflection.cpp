#include "reflection.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include "core/stl_compat.h"
#include <vector>

#include "rt_array.h"
#include "assembly.h"
#include "class.h"
#include "method.h"
#include "object.h"
#include "runtime.h"
#include "type.h"
#include "rt_string.h"
#include "rt_exception.h"
#include "parameter.h"
#include "alloc/general_allocation.h"
#include "metadata/metadata_cache.h"
#include "metadata/metadata_compare.h"
#include "metadata/metadata_hash.h"
#include "metadata/module_def.h"
#include "utils/hash_util.h"
#include "utils/hashmap.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace vm
{
namespace
{
struct MethodKey
{
    const metadata::RtMethodInfo* method;
    const metadata::RtClass* klass;
};

struct FieldKey
{
    const metadata::RtFieldInfo* field;
    const metadata::RtClass* klass;
};

struct PropertyKey
{
    const metadata::RtPropertyInfo* property;
    const metadata::RtClass* klass;
};

struct EventKey
{
    metadata::RtEventInfo* event_info;
    const metadata::RtClass* klass;
};

struct MethodKeyHash
{
    size_t operator()(const MethodKey& key) const noexcept
    {
        size_t h = std::hash<const void*>()(key.method);
        return utils::HashUtil::combine_hash(h, std::hash<const metadata::RtClass*>()(key.klass));
    }
};

struct MethodKeyEqual
{
    bool operator()(const MethodKey& a, const MethodKey& b) const noexcept
    {
        return a.method == b.method && a.klass == b.klass;
    }
};

struct FieldKeyHash
{
    size_t operator()(const FieldKey& key) const noexcept
    {
        size_t h = std::hash<const void*>()(key.field);
        return utils::HashUtil::combine_hash(h, std::hash<const metadata::RtClass*>()(key.klass));
    }
};

struct FieldKeyEqual
{
    bool operator()(const FieldKey& a, const FieldKey& b) const noexcept
    {
        return a.field == b.field && a.klass == b.klass;
    }
};

struct PropertyKeyHash
{
    size_t operator()(const PropertyKey& key) const noexcept
    {
        size_t h = std::hash<const void*>()(key.property);
        return utils::HashUtil::combine_hash(h, std::hash<const metadata::RtClass*>()(key.klass));
    }
};

struct PropertyKeyEqual
{
    bool operator()(const PropertyKey& a, const PropertyKey& b) const noexcept
    {
        return a.property == b.property && a.klass == b.klass;
    }
};

struct EventKeyHash
{
    size_t operator()(const EventKey& key) const noexcept
    {
        size_t h = std::hash<metadata::RtEventInfo*>()(key.event_info);
        return utils::HashUtil::combine_hash(h, std::hash<const metadata::RtClass*>()(key.klass));
    }
};

struct EventKeyEqual
{
    bool operator()(const EventKey& a, const EventKey& b) const noexcept
    {
        return a.event_info == b.event_info && a.klass == b.klass;
    }
};

static utils::HashMap<const metadata::RtTypeSig*, RtReflectionType*, metadata::TypeSigIgnoreAttrsHasher, metadata::TypeSigIgnoreAttrsEqual>
    s_class_reflection_type_map;
static utils::HashMap<MethodKey, RtReflectionMethod*, MethodKeyHash, MethodKeyEqual> s_method_reflection_map;
static utils::HashMap<MethodKey, RtArray*, MethodKeyHash, MethodKeyEqual> s_method_params_map;
static utils::HashMap<FieldKey, RtReflectionField*, FieldKeyHash, FieldKeyEqual> s_field_reflection_map;
static utils::HashMap<PropertyKey, RtReflectionProperty*, PropertyKeyHash, PropertyKeyEqual> s_property_reflection_map;
static utils::HashMap<EventKey, RtReflectionEventInfo*, EventKeyHash, EventKeyEqual> s_event_reflection_map;
static utils::HashMap<const metadata::RtAssembly*, RtReflectionAssembly*> s_assembly_reflection_map;
static utils::HashMap<const metadata::RtModuleDef*, RtReflectionModule*> s_module_reflection_map;
static utils::HashMap<const metadata::RtAssembly*, metadata::RtMonoAssemblyName*> s_assembly_name_map;

static RtResult<int32_t> unbox_i32(RtObject* obj, const metadata::RtClass* cls_i32)
{
    if (obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    auto klass = obj->klass;
    if (klass != cls_i32)
    {
        RET_ERR(RtErr::InvalidCast);
    }
    auto data_ptr = reinterpret_cast<const int32_t*>(reinterpret_cast<uint8_t*>(obj) + sizeof(metadata::RtClass*));
    RET_OK(*data_ptr);
}

static RtResult<RtArray*> invoke_new_array(const metadata::RtMethodInfo* method, RtArray* params)
{
    auto klass = method->parent;
    int32_t method_param_count = static_cast<int32_t>(method->parameter_count);
    auto corlib_types = Class::get_corlib_types();
    if (params == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    int32_t argument_count = Array::get_array_length(params);
    if (argument_count <= 0)
    {
        RET_ERR(RtErr::Argument);
    }
    if (argument_count == method_param_count)
    {
        if (argument_count == 1)
        {
            auto length_obj = Array::get_array_data_at<RtObject*>(params, 0);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, length, unbox_i32(length_obj, corlib_types.cls_int32));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, arr_obj, Array::new_szarray_from_array_klass(klass, length));
            RET_OK(arr_obj);
        }
        else
        {
            utils::Vector<int32_t> lengths(static_cast<size_t>(method_param_count));
            for (int32_t i = 0; i < method_param_count; ++i)
            {
                auto length_obj = Array::get_array_data_at<RtObject*>(params, i);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, length, unbox_i32(length_obj, corlib_types.cls_int32));
                lengths[static_cast<size_t>(i)] = length;
            }
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, arr_obj, Array::new_mdarray_from_array_klass(klass, lengths.data(), nullptr));
            RET_OK(arr_obj);
        }
    }
    else if (argument_count == method_param_count * 2)
    {
        utils::Vector<int32_t> lengths(static_cast<size_t>(method_param_count));
        utils::Vector<int32_t> lower_bounds(static_cast<size_t>(method_param_count));
        for (int32_t i = 0; i < method_param_count; ++i)
        {
            auto length_obj = Array::get_array_data_at<RtObject*>(params, i);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, length, unbox_i32(length_obj, corlib_types.cls_int32));
            lengths[static_cast<size_t>(i)] = length;
        }
        for (int32_t i = 0; i < method_param_count; ++i)
        {
            auto lower_bound_obj = Array::get_array_data_at<RtObject*>(params, i + method_param_count);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, lower_bound, unbox_i32(lower_bound_obj, corlib_types.cls_int32));
            lower_bounds[static_cast<size_t>(i)] = lower_bound;
        }
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, arr_obj, Array::new_mdarray_from_array_klass(klass, lengths.data(), lower_bounds.data()));
        RET_OK(arr_obj);
    }
    else
    {
        RET_ERR(RtErr::Argument);
    }
}
} // namespace

RtResult<RtReflectionType*> Reflection::get_type_reflection_object(const metadata::RtTypeSig* type_sig)
{
    auto canon_type_sig = type_sig->to_canonized();

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, pooled_type_sig, metadata::MetadataCache::get_pooled_typesig(canon_type_sig));

    auto it2 = s_class_reflection_type_map.find(pooled_type_sig);
    if (it2 != s_class_reflection_type_map.end())
    {
        RET_OK(it2->second);
    }

    auto runtime_type_klass = Class::get_corlib_types().cls_runtimetype;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, Object::new_object(runtime_type_klass));
    auto ref_obj = reinterpret_cast<RtReflectionType*>(ref_obj_raw);

    s_class_reflection_type_map.emplace(pooled_type_sig, ref_obj);
    ref_obj->type_handle = pooled_type_sig;
    RET_OK(ref_obj);
}

RtResult<RtReflectionType*> Reflection::get_klass_reflection_object(const metadata::RtClass* klass)
{
    return get_type_reflection_object(Class::get_by_val_type_sig(klass));
}

RtResult<RtReflectionMethod*> Reflection::get_method_reflection_object(const metadata::RtMethodInfo* method, const metadata::RtClass* reflection_at_klass)
{
    MethodKey key{method, reflection_at_klass};
    auto found = s_method_reflection_map.find(key);
    if (found != s_method_reflection_map.end())
    {
        RET_OK(found->second);
    }

    auto corlib_types = Class::get_corlib_types();
    auto runtime_method_klass = Method::is_ctor_or_cctor(method) ? corlib_types.cls_reflection_constructor : corlib_types.cls_reflection_method;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, Object::new_object(runtime_method_klass));
    auto ref_obj = reinterpret_cast<RtReflectionMethod*>(ref_obj_raw);
    ref_obj->method = method;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, ref_type, get_klass_reflection_object(reflection_at_klass));
    ref_obj->ref_type = ref_type;
    s_method_reflection_map.emplace(key, ref_obj);
    RET_OK(ref_obj);
}

RtResult<RtArray*> Reflection::get_param_objects(const metadata::RtMethodInfo* method, const metadata::RtClass* reflection_at_klass)
{
    MethodKey key{method, reflection_at_klass};
    auto found = s_method_params_map.find(key);
    if (found != s_method_params_map.end())
    {
        RET_OK(found->second);
    }

    size_t param_count = method->parameter_count;
    auto param_info_klass = Class::get_corlib_types().cls_reflection_parameter;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, param_info_array_obj,
                                            Array::new_szarray_from_ele_klass(param_info_klass, static_cast<int32_t>(param_count)));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionMethod*, ref_member, get_method_reflection_object(method, reflection_at_klass));
    auto ass = method->parent->image;
    for (size_t i = 0; i < param_count; ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, param_obj_base, Object::new_object(param_info_klass));
        auto param_info_obj = reinterpret_cast<RtReflectionParameter*>(param_obj_base);

        const metadata::RtTypeSig* param_type_sig = method->parameters[i];
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, parent_type, get_type_reflection_object(param_type_sig));
        param_info_obj->parent_type = parent_type;
        param_info_obj->member = reinterpret_cast<RtObject*>(ref_member);

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<uint32_t>, param_token_opt, Method::get_parameter_token(method, static_cast<int32_t>(i)));
        if (param_token_opt.has_value())
        {
            metadata::EncodedTokenId param_token = param_token_opt.value();
            auto opt_param = ass->get_cli_image().read_param(metadata::RtToken::decode_rid(param_token));
            param_info_obj->attrs = opt_param->flags;
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtString*, param_name, Method::get_parameter_name_by_token(ass, param_token));
            param_info_obj->name = param_name;
            if (Parameter::has_parameter_attr_optional(param_info_obj->attrs))
            {
                UNWRAP_OR_RET_ERR_ON_FAIL(param_info_obj->default_value, Parameter::get_parameter_default_value_object(ass, param_token, param_type_sig));
            }
        }
        param_info_obj->index = static_cast<int32_t>(i);
        // param_info_obj->attrs = static_cast<uint32_t>(param_type_sig->flags);
        Array::set_array_data_at<RtReflectionParameter*>(param_info_array_obj, static_cast<int32_t>(i), param_info_obj);
    }
    s_method_params_map.emplace(key, param_info_array_obj);
    RET_OK(param_info_array_obj);
}

RtResult<RtReflectionField*> Reflection::get_field_reflection_object(const metadata::RtFieldInfo* field, const metadata::RtClass* reflection_at_klass)
{
    FieldKey key{field, reflection_at_klass};
    auto found = s_field_reflection_map.find(key);
    if (found != s_field_reflection_map.end())
    {
        RET_OK(found->second);
    }

    auto corlib_types = Class::get_corlib_types();
    auto runtime_field_klass = corlib_types.cls_reflection_field;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, Object::new_object(runtime_field_klass));
    auto ref_obj = reinterpret_cast<RtReflectionField*>(ref_obj_raw);
    ref_obj->field = field;
    ref_obj->klass = reflection_at_klass;
    ref_obj->name = String::create_string_from_utf8chars(field->name, static_cast<int32_t>(std::strlen(field->name)));
    ref_obj->attrs = field->flags;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, type_obj, get_type_reflection_object(field->type_sig));
    ref_obj->type_ = type_obj;
    s_field_reflection_map.emplace(key, ref_obj);
    RET_OK(ref_obj);
}

RtResult<RtReflectionProperty*> Reflection::get_property_reflection_object(const metadata::RtPropertyInfo* prop, const metadata::RtClass* reflection_at_klass)
{
    PropertyKey key{prop, reflection_at_klass};
    auto found = s_property_reflection_map.find(key);
    if (found != s_property_reflection_map.end())
    {
        RET_OK(found->second);
    }

    auto corlib_types = Class::get_corlib_types();
    auto runtime_prop_klass = corlib_types.cls_reflection_property;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, Object::new_object(runtime_prop_klass));
    auto ref_obj = reinterpret_cast<RtReflectionProperty*>(ref_obj_raw);
    ref_obj->property = prop;
    ref_obj->klass = reflection_at_klass;
    s_property_reflection_map.emplace(key, ref_obj);
    RET_OK(ref_obj);
}

RtResult<RtReflectionEventInfo*> Reflection::get_event_reflection_object(metadata::RtEventInfo* event_info, const metadata::RtClass* reflection_at_klass)
{
    EventKey key{event_info, reflection_at_klass};
    auto found = s_event_reflection_map.find(key);
    if (found != s_event_reflection_map.end())
    {
        RET_OK(found->second);
    }

    auto corlib_types = Class::get_corlib_types();
    auto runtime_event_klass = corlib_types.cls_reflection_event;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, Object::new_object(runtime_event_klass));
    auto ref_obj = reinterpret_cast<RtReflectionEventInfo*>(ref_obj_raw);
    ref_obj->event = event_info;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, ref_type, get_klass_reflection_object(reflection_at_klass));
    ref_obj->ref_type = ref_type;
    s_event_reflection_map.emplace(key, ref_obj);
    RET_OK(ref_obj);
}

RtResult<RtReflectionAssembly*> Reflection::get_assembly_reflection_object(metadata::RtAssembly* assembly)
{
    auto found = s_assembly_reflection_map.find(assembly);
    if (found != s_assembly_reflection_map.end())
    {
        RET_OK(found->second);
    }

    auto corlib_types = Class::get_corlib_types();
    auto runtime_assembly_klass = corlib_types.cls_reflection_assembly;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, Object::new_object(runtime_assembly_klass));
    auto ref_obj = reinterpret_cast<RtReflectionAssembly*>(ref_obj_raw);
    ref_obj->assembly = assembly;
    s_assembly_reflection_map.emplace(assembly, ref_obj);
    RET_OK(ref_obj);
}

RtResult<metadata::RtMonoAssemblyName*> Reflection::get_assembly_name_object(metadata::RtAssembly* ass)
{
    auto found = s_assembly_name_map.find(ass);
    if (found != s_assembly_name_map.end())
    {
        RET_OK(found->second);
    }

    auto name_obj = alloc::GeneralAllocation::malloc_any_zeroed<metadata::RtMonoAssemblyName>();
    ass->mod->fill_assembly_name(*name_obj);
    s_assembly_name_map.emplace(ass, name_obj);
    RET_OK(name_obj);
}

RtResult<RtReflectionModule*> Reflection::get_module_reflection_object(metadata::RtModuleDef* mod)
{
    auto found = s_module_reflection_map.find(mod);
    if (found != s_module_reflection_map.end())
    {
        RET_OK(found->second);
    }

    auto corlib_types = Class::get_corlib_types();
    auto runtime_module_klass = corlib_types.cls_reflection_module;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, ref_obj_raw, Object::new_object(runtime_module_klass));
    auto ref_obj = reinterpret_cast<RtReflectionModule*>(ref_obj_raw);

    ref_obj->image = mod;
    UNWRAP_OR_RET_ERR_ON_FAIL(ref_obj->assembly, get_assembly_reflection_object(mod->get_assembly()));

    utils::StringBuilder fqname_buf;
    Type::append_assembly_name(fqname_buf, mod->get_assembly_name());

    auto name_no_ext = mod->get_name_no_ext();
    auto name = mod->get_name();
    ref_obj->fqname = String::create_string_from_utf8chars(fqname_buf.as_cstr(), static_cast<int32_t>(fqname_buf.length()));
    ref_obj->name = String::create_string_from_utf8chars(name, static_cast<int32_t>(std::strlen(name)));
    ref_obj->scope_name = String::create_string_from_utf8chars(name_no_ext, static_cast<int32_t>(std::strlen(name_no_ext)));
    ref_obj->token = mod->get_assembly_token();
    s_module_reflection_map.insert({mod, ref_obj});
    RET_OK(ref_obj);
}

RtResult<RtObject*> Reflection::invoke_method(const metadata::RtMethodInfo* method, RtObject* obj, RtArray* params, RtObject** out_ex)
{
    const metadata::RtClass* klass = method->parent;
    size_t method_param_count = method->parameter_count;
    size_t params_count = params == nullptr ? 0 : static_cast<size_t>(Array::get_array_length(params));
    auto& corlib_types = Class::get_corlib_types();
    if (method_param_count != params_count)
    {
        if (out_ex)
        {
            *out_ex = Exception::raise_internal_runtime_exception(corlib_types.cls_target_parameter_count_exception, "Parameter count mismatch");
        }
        RET_OK(nullptr);
    }

    if (Method::is_instance(method))
    {
        if (Method::is_ctor(method))
        {
            if (Class::is_array_or_szarray(klass))
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, arr_obj, invoke_new_array(method, params));
                RET_OK(reinterpret_cast<RtObject*>(arr_obj));
            }
            if (obj == nullptr)
            {
                if (Class::is_abstract(klass) || Class::is_interface(klass))
                {
                    *out_ex =
                        Exception::raise_internal_runtime_exception(corlib_types.cls_target_exception, "Cannot create instance of abstract class or interface");
                    RET_OK(nullptr);
                }
                else
                {
                    if (Class::is_nullable_type(klass))
                    {
                        assert(params_count == 1);
                        RtObject* param_obj = Array::get_array_data_at<RtObject*>(params, 0);
                        metadata::RtClass* ele_klass = Class::get_array_element_class(klass);
                        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const void*, ele_data_ptr, Object::unbox_ex(param_obj, ele_klass));
                        return Object::box_object(ele_klass, ele_data_ptr);
                    }
                }
            }
            else
            {
                if (!Object::is_inst(obj, klass))
                {
                    RET_ERR(RtErr::InvalidCast);
                }
            }
            if (Method::is_virtual(method))
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, virt_method, Method::get_virtual_method_impl(obj, method));
                method = virt_method;
            }
        }
    }

    return Runtime::invoke_array_arguments_with_run_cctor(method, obj, params);
}

} // namespace vm
} // namespace leanclr

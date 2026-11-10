#include "aot_module.h"
#include "utils/hashmap.h"
#include "utils/string_util.h"

namespace leanclr
{
namespace metadata
{

static utils::HashMap<const char*, const RtAotModuleData*, utils::CStrHasher, utils::CStrCompare>& get_s_aotModuleMap()
{
    static utils::HashMap<const char*, const RtAotModuleData*, utils::CStrHasher, utils::CStrCompare> value;
    return value;
}

void AotModule::register_aot_modules(const RtAotModulesData* initData)
{
    for (uint32_t i = 0; i < initData->module_count; i++)
    {
        const RtAotModuleData* moduleData = initData->modules[i];
        get_s_aotModuleMap().insert({moduleData->module_name, moduleData});
    }
}

const RtAotModuleData* AotModule::find_aot_module_by_name(const char* module_name)
{
    auto it = get_s_aotModuleMap().find(module_name);
    return it != get_s_aotModuleMap().end() ? it->second : nullptr;
}

namespace
{
bool matches_type(const RtAotTypeIdentity& identity, const RtTypeSig* type);

bool matches_arguments(const RtAotTypeIdentity* identities, uint32_t count, const RtGenericInst* inst)
{
    if (count != (inst ? inst->generic_arg_count : 0))
        return false;
    for (uint32_t i = 0; i < count; ++i)
        if (!matches_type(identities[i], inst->generic_args[i]))
            return false;
    return true;
}

bool matches_definition(const RtAotTypeIdentity& identity, uint32_t gid)
{
    const auto* module = RtModuleDef::get_module_by_id(RtMetadata::decode_module_id_from_gid(gid));
    return module && identity.module_name && std::strcmp(module->get_name_no_ext(), identity.module_name) == 0 &&
           RtMetadata::decode_rid_from_gid(gid) == RtToken::decode_rid(identity.type_token);
}

bool matches_type(const RtAotTypeIdentity& identity, const RtTypeSig* type)
{
    if (!type || type->is_by_ref() || type->ele_type != identity.element_type)
        return false;
    switch (type->ele_type)
    {
    case RtElementType::Class:
    case RtElementType::ValueType:
        return matches_definition(identity, type->data.type_def_gid);
    case RtElementType::GenericInst:
        return matches_definition(identity, type->data.generic_class->base_type_def_gid) &&
               matches_arguments(identity.arguments, identity.argument_count, type->data.generic_class->class_inst);
    case RtElementType::SZArray:
        return identity.argument_count == 1 && matches_type(identity.arguments[0], type->data.element_type);
    default:
        return identity.argument_count == 0;
    }
}
} // namespace

const RtAotMethodDefData* AotModule::find_aot_method_impl(const RtMethodInfo* method)
{
    if (method->generic_method && !method->generic_container)
    {
        const auto* module_data = method->parent->image->get_aot_module_data();
        if (!module_data)
            return nullptr;
        const auto& context = method->generic_method->generic_context;
        for (uint32_t i = 0; i < module_data->generic_method_entry_count; ++i)
        {
            const auto& entry = module_data->generic_method_entries[i];
            if (entry.implementation.token == method->token && matches_arguments(entry.class_arguments, entry.class_argument_count, context.class_inst) &&
                matches_arguments(entry.method_arguments, entry.method_argument_count, context.method_inst))
                return &entry.implementation;
        }
        return nullptr;
    }
    if (method->generic_method != nullptr || method->generic_container != nullptr || method->parent->generic_container != nullptr)
    {
        return nullptr;
    }
    const RtAotMethodDefData* aotMethodDefData = find_aot_method_def_impl(method->parent->image, method->token);
    if (aotMethodDefData == nullptr)
    {
        return nullptr;
    }
    return aotMethodDefData;
}

const RtAotMethodDefData* AotModule::find_aot_method_def_impl(const RtModuleDef* module, EncodedTokenId token)
{
    const RtAotModuleData* aotModuleData = module->get_aot_module_data();
    if (aotModuleData == nullptr || aotModuleData->method_def_entry_count == 0)
    {
        return nullptr;
    }
    const RtAotMethodDefData* entries = aotModuleData->method_def_entries;
    uint32_t count = aotModuleData->method_def_entry_count;
    uint32_t left = 0;
    uint32_t right = count;
    while (left < right)
    {
        uint32_t mid = left + (right - left) / 2;
        EncodedTokenId cur_token = entries[mid].token;
        if (cur_token < token)
        {
            left = mid + 1;
        }
        else if (cur_token > token)
        {
            right = mid;
        }
        else
        {
            return &entries[mid];
        }
    }
    return nullptr;
}

const RtAotMethodMonoPInvokeCallbackData* AotModule::find_mono_pinvoke_callback_method(const RtModuleDef* module, EncodedTokenId token)
{
    const RtAotModuleData* aotModuleData = module->get_aot_module_data();
    if (aotModuleData == nullptr || aotModuleData->mono_pinvoke_callback_entry_count == 0)
    {
        return nullptr;
    }
    const RtAotMethodMonoPInvokeCallbackData* entries = aotModuleData->mono_pinvoke_callback_entries;
    uint32_t count = aotModuleData->mono_pinvoke_callback_entry_count;
    uint32_t left = 0;
    uint32_t right = count;
    while (left < right)
    {
        uint32_t mid = left + (right - left) / 2;
        EncodedTokenId cur_token = entries[mid].token;
        if (cur_token < token)
        {
            left = mid + 1;
        }
        else if (cur_token > token)
        {
            right = mid;
        }
        else
        {
            return &entries[mid];
        }
    }
    return nullptr;
}

} // namespace metadata
} // namespace leanclr

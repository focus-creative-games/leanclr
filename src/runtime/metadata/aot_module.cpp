#include "aot_module.h"
#include "utils/hashmap.h"
#include "utils/string_util.h"

namespace leanclr
{
namespace metadata
{

utils::HashMap<const char*, const RtAotModuleData*, utils::CStrHasher, utils::CStrCompare> s_aotModuleMap;

void AotModule::register_aot_modules(const RtAotModulesData* initData)
{
    for (uint32_t i = 0; i < initData->module_count; i++)
    {
        const RtAotModuleData* moduleData = initData->modules[i];
        s_aotModuleMap.insert({moduleData->module_name, moduleData});
    }
}

const RtAotModuleData* AotModule::find_aot_module_by_name(const char* module_name)
{
    auto it = s_aotModuleMap.find(module_name);
    return it != s_aotModuleMap.end() ? it->second : nullptr;
}

std::optional<RtAotMethodImplData> AotModule::find_aot_method_impl(const RtMethodInfo* method)
{
    if (method->generic_method != nullptr || method->generic_container != nullptr || method->parent->generic_container != nullptr)
    {
        return std::nullopt;
    }
    const RtAotMethodDefData* aotMethodDefData = find_aot_method_def_impl(method->parent->image, method->token);
    if (aotMethodDefData == nullptr)
    {
        return std::nullopt;
    }
    return RtAotMethodImplData{aotMethodDefData->method_ptr, aotMethodDefData->virtual_method_ptr, aotMethodDefData->invoke_method_ptr};
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

} // namespace metadata
} // namespace leanclr
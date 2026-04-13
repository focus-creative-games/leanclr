#include "internal_calls.h"
#include "method.h"
#include "class.h"
#include "metadata/module_def.h"
#include "utils/string_builder.h"
#include "utils/string_util.h"
#include "metadata/metadata_name.h"
#include "icalls/internal_call_stubs.h"

namespace leanclr
{
namespace vm
{

// Static maps for internal call functions
static utils::HashMap<const char*, Il2CppMethodPointer, utils::CStrHasher, utils::CStrCompare> g_il2cppInternalCallMap;
static utils::HashMap<const char*, InternalCallRegistry, utils::CStrHasher, utils::CStrCompare> g_pinvoke_map;
static utils::HashMap<const char*, InternalCallInvoker, utils::CStrHasher, utils::CStrCompare> g_newobjInternalCallMap;
static utils::Vector<InternalCallInvoker> g_internalCallInvokerIdList;
static utils::HashMap<InternalCallInvoker, uint16_t> g_internalCallInvokerIdMap;

void InternalCalls::register_lite_internal_call(const char* name, Il2CppMethodPointer func)
{
    assert(g_il2cppInternalCallMap.find(name) == g_il2cppInternalCallMap.end() && "IL2CPP internal call already registered");
    g_il2cppInternalCallMap[name] = func;
}

// Get IL2CPP internal call by name
Il2CppMethodPointer InternalCalls::get_lite_internal_call(const char* name)
{
    auto it = g_il2cppInternalCallMap.find(name);
    if (it != g_il2cppInternalCallMap.end())
        return it->second;

    // Fallback: trim parameter list and retry with "Type::Method" key.
    // Example:
    //   Unity.Burst.LowLevel.BurstCompilerService::GetOrCreateSharedMemory(UnityEngine.Hash128&,System.UInt32,System.UInt32)
    // -> Unity.Burst.LowLevel.BurstCompilerService::GetOrCreateSharedMemory
    const char* params_start = std::strchr(name, '(');
    if (params_start != nullptr && params_start > name)
    {
        utils::StringBuilder short_name;
        short_name.append_cstr(reinterpret_cast<const uint8_t*>(name), static_cast<size_t>(params_start - name));
        short_name.sure_null_terminator_but_not_append();

        it = g_il2cppInternalCallMap.find(short_name.as_cstr());
        if (it != g_il2cppInternalCallMap.end())
            return it->second;
    }

    return nullptr;
}

// Register an internal call function by name
void InternalCalls::register_internal_call(const char* name, InternalCallFunction func, InternalCallInvoker invoker)
{
    assert(g_pinvoke_map.find(name) == g_pinvoke_map.end() && "Internal call already registered");
    g_pinvoke_map[name] = InternalCallRegistry{func, invoker};
}

// Get internal call by name
const InternalCallRegistry* InternalCalls::get_internal_call(const char* name)
{
    auto it = g_pinvoke_map.find(name);
    if (it != g_pinvoke_map.end())
        return &it->second;
    return nullptr;
}

// Get internal call by method info (builds full method name with params)
RtResult<const InternalCallRegistry*> InternalCalls::get_internal_call_by_method(const metadata::RtMethodInfo* method)
{
    // Try with full method name (including parameters)
    utils::StringBuilder sb;
    {
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_with_params(sb, method));
        auto it = g_pinvoke_map.find(sb.as_cstr());
        if (it != g_pinvoke_map.end())
            RET_OK(&it->second);
    }

    // Try with method name without parameters
    {
        sb.clear();
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method));
        auto it = g_pinvoke_map.find(sb.as_cstr());
        if (it != g_pinvoke_map.end())
            RET_OK(&it->second);
    }

    RET_OK(nullptr);
}

// Register newobj internal call
void InternalCalls::register_newobj_internal_call(const char* name, InternalCallInvoker invoker)
{
    assert(g_newobjInternalCallMap.find(name) == g_newobjInternalCallMap.end() && "Newobj internal call already registered");
    g_newobjInternalCallMap[name] = invoker;
}

// Get newobj internal call by name
InternalCallInvoker InternalCalls::get_newobj_internal_call(const char* name)
{
    auto it = g_newobjInternalCallMap.find(name);
    if (it != g_newobjInternalCallMap.end())
        return it->second;
    return nullptr;
}

// Get newobj internal call by method info
RtResult<InternalCallInvoker> InternalCalls::get_newobj_internal_call_by_method(const metadata::RtMethodInfo* method)
{
    utils::StringBuilder sb;
    // Try with full method name (including parameters)
    {
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_with_params(sb, method));
        const char* fullName = sb.as_cstr();
        auto it = g_newobjInternalCallMap.find(fullName);
        if (it != g_newobjInternalCallMap.end())
            RET_OK(it->second);
    }

    // Try with method name without parameters
    {
        sb.clear();
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method));
        const char* nameOnly = sb.as_cstr();
        auto it = g_newobjInternalCallMap.find(nameOnly);
        if (it != g_newobjInternalCallMap.end())
            RET_OK(it->second);
    }

    RET_OK((InternalCallInvoker) nullptr);
}

// Get ID for an internal call invoker (with registration if needed)
uint16_t InternalCalls::register_internal_call_invoker_id(InternalCallInvoker invoker)
{
    auto it = g_internalCallInvokerIdMap.find(invoker);
    if (it != g_internalCallInvokerIdMap.end())
        return it->second;

    uint16_t id = static_cast<uint16_t>(g_internalCallInvokerIdList.size());
    if (id == UINT16_MAX)
    {
        // Error: too many internal calls registered
        assert(false && "Too many internal calls registered");
        return 0;
    }

    g_internalCallInvokerIdList.push_back(invoker);
    g_internalCallInvokerIdMap[invoker] = id;
    return id;
}

// Get internal call invoker by ID
InternalCallInvoker InternalCalls::get_internal_call_invoker_by_id_unchecked(uint16_t id)
{
    assert(id < g_internalCallInvokerIdList.size() && "Invalid internal call invoker id");
    return g_internalCallInvokerIdList[id];
}

// Initialize internal calls system
void InternalCalls::initialize()
{
    utils::Vector<vm::InternalCallEntry> entries;
    icalls::InternalCallStubs::get_internal_call_entries(entries);
    for (auto& entry : entries)
        register_internal_call(entry.name, entry.func, entry.invoker);

    utils::Vector<vm::NewobjInternalCallEntry> newobjEntries;
    icalls::InternalCallStubs::get_newobj_internal_call_entries(newobjEntries);
    for (auto& entry : newobjEntries)
        register_newobj_internal_call(entry.name, entry.invoker);
}

} // namespace vm
} // namespace leanclr

#include "intrinsics.h"
#include "method.h"
#include "class.h"
#include "metadata/module_def.h"
#include "utils/string_builder.h"
#include "utils/string_util.h"
#include "metadata/metadata_name.h"
#include "intrinsics/intrinsic_stubs.h"

namespace leanclr
{
namespace vm
{

// Static maps for intrinsic functions
static utils::HashMap<const char*, IntrinsicRegistry, utils::CStrHasher, utils::CStrCompare> g_intrinsicMap;
static utils::HashMap<const char*, IntrinsicInvoker, utils::CStrHasher, utils::CStrCompare> g_newobjIntrinsicMap;
static utils::Vector<IntrinsicInvoker> g_intrinsicInvokerIdList;
static utils::HashMap<IntrinsicInvoker, uint16_t> g_intrinsicInvokerIdMap;

// Register an intrinsic function by name
void Intrinsics::register_intrinsic(const char* name, IntrinsicFunction func, IntrinsicInvoker invoker)
{
    assert(g_intrinsicMap.find(name) == g_intrinsicMap.end() && "Intrinsic already registered");
    g_intrinsicMap[name] = IntrinsicRegistry{func, invoker};
}

// Get intrinsic by name and length
const IntrinsicRegistry* Intrinsics::get_intrinsic(const char* name)
{
    auto it = g_intrinsicMap.find(name);
    if (it != g_intrinsicMap.end())
        return &it->second;
    return nullptr;
}

// Get intrinsic by method info (builds full method name with params)
RtResult<const IntrinsicRegistry*> Intrinsics::get_intrinsic_by_method(const metadata::RtMethodInfo* method)
{
    utils::StringBuilder sb;

    {
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_with_params(sb, method));
        auto it = g_intrinsicMap.find(sb.as_cstr());
        if (it != g_intrinsicMap.end())
            RET_OK(&it->second);
    }

    {
        sb.clear();
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method));
        auto it = g_intrinsicMap.find(sb.as_cstr());
        if (it != g_intrinsicMap.end())
            RET_OK(&it->second);
    }
    RET_OK(nullptr);
}

// Register newobj intrinsic
void Intrinsics::register_newobj_intrinsic(const char* name, IntrinsicInvoker invoker)
{
    assert(g_newobjIntrinsicMap.find(name) == g_newobjIntrinsicMap.end() && "Newobj intrinsic already registered");
    g_newobjIntrinsicMap[name] = invoker;
}

// Get newobj intrinsic by name and length
IntrinsicInvoker Intrinsics::get_newobj_intrinsic(const char* name)
{
    auto it = g_newobjIntrinsicMap.find(name);
    if (it != g_newobjIntrinsicMap.end())
        return it->second;
    return nullptr;
}

// Get newobj intrinsic by method info
RtResult<IntrinsicInvoker> Intrinsics::get_newobj_intrinsic_by_method(const metadata::RtMethodInfo* method)
{
    utils::StringBuilder sb;

    {
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_with_params(sb, method));
        auto it = g_newobjIntrinsicMap.find(sb.as_cstr());
        if (it != g_newobjIntrinsicMap.end())
            RET_OK(it->second);
    }

    {
        sb.clear();
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method));
        auto it = g_newobjIntrinsicMap.find(sb.as_cstr());
        if (it != g_newobjIntrinsicMap.end())
            RET_OK(it->second);
    }
    RET_OK((IntrinsicInvoker) nullptr);
}

// Get ID for an intrinsic invoker (with registration if needed)
uint16_t Intrinsics::register_intrinsic_invoker_id(IntrinsicInvoker invoker)
{
    auto it = g_intrinsicInvokerIdMap.find(invoker);
    if (it != g_intrinsicInvokerIdMap.end())
        return it->second;

    uint16_t id = static_cast<uint16_t>(g_intrinsicInvokerIdList.size());
    if (id == UINT16_MAX)
    {
        // Error: too many intrinsics registered
        // In production code, should return error or throw exception
        return 0;
    }

    g_intrinsicInvokerIdList.push_back(invoker);
    g_intrinsicInvokerIdMap[invoker] = id;
    return id;
}

// Get intrinsic invoker by ID
IntrinsicInvoker Intrinsics::get_intrinsic_invoker_by_id_unchecked(uint16_t id)
{
    assert(id < g_intrinsicInvokerIdList.size() && "Invalid intrinsic invoker id");
    return g_intrinsicInvokerIdList[id];
}

// Initialize intrinsics system
// This would call into intrinsic stubs to register all intrinsics
void Intrinsics::initialize()
{
    utils::Vector<vm::IntrinsicEntry> intrinsicEntries;
    intrinsics::IntrinsicStubs::get_intrinsic_entries(intrinsicEntries);
    for (auto entry : intrinsicEntries)
        register_intrinsic(entry.name, entry.func, entry.invoker);
    utils::Vector<vm::NewobjIntrinsicEntry> newobjIntrinsicEntries;
    intrinsics::IntrinsicStubs::get_newobj_intrinsic_entries(newobjIntrinsicEntries);
    for (auto entry : newobjIntrinsicEntries)
        register_newobj_intrinsic(entry.name, entry.invoker);
}

} // namespace vm
} // namespace leanclr

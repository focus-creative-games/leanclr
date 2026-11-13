#include "host_invokers.h"
#include "intrinsics.h"
#include "method.h"
#include "class.h"
#include "metadata/module_def.h"
#include "utils/string_builder.h"
#include "utils/string_util.h"
#include "metadata/metadata_name.h"
#include "bcl/adapter.h"

namespace leanclr
{
namespace vm
{

// Static maps for intrinsic functions
static utils::HashMap<const char*, IntrinsicRegistry, utils::CStrHasher, utils::CStrCompare>& get_g_intrinsicMap()
{
    static utils::HashMap<const char*, IntrinsicRegistry, utils::CStrHasher, utils::CStrCompare> value;
    return value;
}

static utils::HashMap<const char*, IntrinsicInvoker, utils::CStrHasher, utils::CStrCompare>& get_g_newobjIntrinsicMap()
{
    static utils::HashMap<const char*, IntrinsicInvoker, utils::CStrHasher, utils::CStrCompare> value;
    return value;
}

static utils::Vector<IntrinsicInvoker> g_intrinsicInvokerIdList;

static utils::HashMap<IntrinsicInvoker, uint16_t>& get_g_intrinsicInvokerIdMap()
{
    static utils::HashMap<IntrinsicInvoker, uint16_t> value;
    return value;
}

// Register an intrinsic function by name
void Intrinsics::register_intrinsic(const char* name, IntrinsicFunction func, IntrinsicInvoker invoker)
{
    assert(get_g_intrinsicMap().find(name) == get_g_intrinsicMap().end() && "Intrinsic already registered");
    get_g_intrinsicMap()[name] = IntrinsicRegistry{func, invoker};
}

// Get intrinsic by name and length
const IntrinsicRegistry* Intrinsics::get_intrinsic(const char* name)
{
    auto it = get_g_intrinsicMap().find(name);
    if (it != get_g_intrinsicMap().end())
        return &it->second;
    return nullptr;
}

// Get intrinsic by method info (builds full method name with params)
RtResult<const IntrinsicRegistry*> Intrinsics::get_intrinsic_by_method(const metadata::RtMethodInfo* method)
{
    utils::Utf8StringBuilder sb;

    {
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_with_params(sb, method, metadata::TypeNameFormat::InternalName));
        auto it = get_g_intrinsicMap().find(sb.get_const_chars());
        if (it != get_g_intrinsicMap().end())
        {
            RET_ERR_ON_FAIL(HostInvokers::bind(method, it->first, it->second.invoker));
            RET_OK(&it->second);
        }
    }

    {
        sb.clear();
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method, metadata::TypeNameFormat::InternalName));
        auto it = get_g_intrinsicMap().find(sb.get_const_chars());
        if (it != get_g_intrinsicMap().end())
        {
            RET_ERR_ON_FAIL(HostInvokers::bind(method, it->first, it->second.invoker));
            RET_OK(&it->second);
        }
    }
    RET_OK(bcl::Adapter::resolve_intrinsic(method));
}

// Register newobj intrinsic
void Intrinsics::register_newobj_intrinsic(const char* name, IntrinsicInvoker invoker)
{
    assert(get_g_newobjIntrinsicMap().find(name) == get_g_newobjIntrinsicMap().end() && "Newobj intrinsic already registered");
    get_g_newobjIntrinsicMap()[name] = invoker;
}

// Get newobj intrinsic by name and length
IntrinsicInvoker Intrinsics::get_newobj_intrinsic(const char* name)
{
    auto it = get_g_newobjIntrinsicMap().find(name);
    if (it != get_g_newobjIntrinsicMap().end())
        return it->second;
    return nullptr;
}

// Get newobj intrinsic by method info
RtResult<IntrinsicInvoker> Intrinsics::get_newobj_intrinsic_by_method(const metadata::RtMethodInfo* method)
{
    utils::Utf8StringBuilder sb;

    {
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_with_params(sb, method, metadata::TypeNameFormat::InternalName));
        auto it = get_g_newobjIntrinsicMap().find(sb.get_const_chars());
        if (it != get_g_newobjIntrinsicMap().end())
        {
            RET_ERR_ON_FAIL(HostInvokers::bind(method, it->first, it->second));
            RET_OK(it->second);
        }
    }

    {
        sb.clear();
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method, metadata::TypeNameFormat::InternalName));
        auto it = get_g_newobjIntrinsicMap().find(sb.get_const_chars());
        if (it != get_g_newobjIntrinsicMap().end())
        {
            RET_ERR_ON_FAIL(HostInvokers::bind(method, it->first, it->second));
            RET_OK(it->second);
        }
    }
    RET_OK((IntrinsicInvoker) nullptr);
}

// Get ID for an intrinsic invoker (with registration if needed)
uint16_t Intrinsics::register_intrinsic_invoker_id(IntrinsicInvoker invoker)
{
    auto it = get_g_intrinsicInvokerIdMap().find(invoker);
    if (it != get_g_intrinsicInvokerIdMap().end())
        return it->second;

    uint16_t id = static_cast<uint16_t>(g_intrinsicInvokerIdList.size());
    if (id == UINT16_MAX)
    {
        // Error: too many intrinsics registered
        // In production code, should return error or throw exception
        return 0;
    }

    g_intrinsicInvokerIdList.push_back(invoker);
    get_g_intrinsicInvokerIdMap()[invoker] = id;
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
    bcl::Adapter::register_intrinsics();
}

} // namespace vm
} // namespace leanclr

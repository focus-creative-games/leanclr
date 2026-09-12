#include "intrinsics.h"
#include "method.h"
#include "class.h"
#include "metadata/module_def.h"
#include "utils/string_builder.h"
#include "utils/string_util.h"
#include "metadata/metadata_name.h"
#include "intrinsics/intrinsic_stubs.h"
#include "const_strs.h"

#include <cstring>

namespace leanclr
{
namespace vm
{

// Static maps for intrinsic functions
static utils::HashMap<const char*, IntrinsicRegistry, utils::CStrHasher, utils::CStrCompare> g_intrinsicMap;
static utils::HashMap<const char*, IntrinsicInvoker, utils::CStrHasher, utils::CStrCompare> g_newobjIntrinsicMap;
static utils::Vector<IntrinsicInvoker> g_intrinsicInvokerIdList;
static utils::HashMap<IntrinsicInvoker, uint16_t> g_intrinsicInvokerIdMap;

static bool is_coreclr_corlib_method(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr || method->parent == nullptr || method->parent->image == nullptr)
    {
        return false;
    }

    return method->parent->image->is_corlib() && std::strcmp(method->parent->image->get_name_no_ext(), STR_SYSTEM_PRIVATE_CORELIB_NAME) == 0;
}

static bool is_generic_intrinsic_shape_fallback_allowed(const metadata::RtMethodInfo* method) noexcept
{
    if (!is_coreclr_corlib_method(method))
    {
        return true;
    }

    return Method::get_generic_param_count(method) > 0 || Class::is_generic_inst(method->parent);
}

static RtResultVoid get_runtime_intrinsics_is_supported_false_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)params;
    interp::EvalStackOp::set_return(ret, 0);
    RET_VOID_OK();
}

static bool is_runtime_intrinsics_is_supported(const metadata::RtMethodInfo* method) noexcept
{
    constexpr const char* runtime_intrinsics_namespace = "System.Runtime.Intrinsics";
    constexpr size_t runtime_intrinsics_namespace_len = sizeof("System.Runtime.Intrinsics") - 1;

    if (method->parameter_count != 0 || method->return_type->ele_type != metadata::RtElementType::Boolean ||
        std::strcmp(method->name, "get_IsSupported") != 0)
    {
        return false;
    }

    const metadata::RtClass* klass = method->parent;
    while (klass->declaring_class != nullptr)
    {
        klass = klass->declaring_class;
    }

    return std::strncmp(klass->namespaze, runtime_intrinsics_namespace, runtime_intrinsics_namespace_len) == 0;
}

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

static RtResultVoid append_open_declaring_type_method_name(utils::Utf8StringBuilder& sb, const metadata::RtMethodInfo* method)
{
    RET_ERR_ON_FAIL(metadata::MetadataName::append_klass_full_name_without_generic_params(
        sb, method->parent, metadata::TypeNameFormat::InternalName));
    sb.append_cstr("::");
    sb.append_cstr(method->name);

    uint16_t generic_param_count = Method::get_generic_param_count(method);
    if (generic_param_count > 0)
    {
        sb.append_char('<');
        sb.append_chars(',', generic_param_count - 1);
        sb.append_char('>');
    }

    sb.sure_null_terminator_but_not_append();
    RET_VOID_OK();
}

static RtResultVoid append_open_declaring_type_method_name_with_params(utils::Utf8StringBuilder& sb, const metadata::RtMethodInfo* method)
{
    RET_ERR_ON_FAIL(append_open_declaring_type_method_name(sb, method));

    sb.append_char('(');
    for (uint16_t i = 0; i < method->parameter_count; ++i)
    {
        if (i > 0)
        {
            sb.append_char(',');
        }

        RET_ERR_ON_FAIL(metadata::MetadataName::append_type_full_name(sb, method->parameters[i], metadata::TypeNameFormat::InternalName, false));
    }
    sb.append_char(')');
    sb.sure_null_terminator_but_not_append();
    RET_VOID_OK();
}

// Get intrinsic by method info (builds full method name with params)
RtResult<const IntrinsicRegistry*> Intrinsics::get_intrinsic_by_method(const metadata::RtMethodInfo* method)
{
    static const IntrinsicRegistry runtime_intrinsics_is_supported_false = {
        reinterpret_cast<IntrinsicFunction>(get_runtime_intrinsics_is_supported_false_invoker),
        get_runtime_intrinsics_is_supported_false_invoker};

    utils::Utf8StringBuilder sb;

    {
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_with_params(sb, method, metadata::TypeNameFormat::InternalName));
        auto it = g_intrinsicMap.find(sb.get_const_chars());
        if (it != g_intrinsicMap.end())
            RET_OK(&it->second);
    }

    if (!is_coreclr_corlib_method(method))
    {
        sb.clear();
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method, metadata::TypeNameFormat::InternalName));
        auto it = g_intrinsicMap.find(sb.get_const_chars());
        if (it != g_intrinsicMap.end())
            RET_OK(&it->second);
    }

    if (is_generic_intrinsic_shape_fallback_allowed(method))
    {
        sb.clear();
        RET_ERR_ON_FAIL(append_open_declaring_type_method_name(sb, method));
        auto it = g_intrinsicMap.find(sb.get_const_chars());
        if (it != g_intrinsicMap.end())
            RET_OK(&it->second);
    }

    if (is_runtime_intrinsics_is_supported(method))
    {
        RET_OK(&runtime_intrinsics_is_supported_false);
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
    utils::Utf8StringBuilder sb;

    {
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_with_params(sb, method, metadata::TypeNameFormat::InternalName));
        auto it = g_newobjIntrinsicMap.find(sb.get_const_chars());
        if (it != g_newobjIntrinsicMap.end())
            RET_OK(it->second);
    }

    {
        sb.clear();
        RET_ERR_ON_FAIL(append_open_declaring_type_method_name_with_params(sb, method));
        auto it = g_newobjIntrinsicMap.find(sb.get_const_chars());
        if (it != g_newobjIntrinsicMap.end())
            RET_OK(it->second);
    }

    if (!is_coreclr_corlib_method(method))
    {
        sb.clear();
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method, metadata::TypeNameFormat::InternalName));
        auto it = g_newobjIntrinsicMap.find(sb.get_const_chars());
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

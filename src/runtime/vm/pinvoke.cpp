#include "pinvoke.h"

#include "method.h"
#include "class.h"
#include "metadata/module_def.h"
#include "pinvokes/coreclr_qcall.h"
#include "utils/string_builder.h"
#include "utils/string_util.h"
#include "metadata/metadata_name.h"
#include "const_strs.h"

#include <cstring>

namespace leanclr
{
namespace vm
{

// Static maps for internal call functions
static utils::HashMap<const char*, PInvokeRegistry, utils::CStrHasher, utils::CStrCompare> g_internalcall_map;

static bool is_coreclr_corlib_method(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr || method->parent == nullptr || method->parent->image == nullptr)
    {
        return false;
    }

    return method->parent->image->is_corlib() && std::strcmp(method->parent->image->get_name_no_ext(), STR_SYSTEM_PRIVATE_CORELIB_NAME) == 0;
}

static bool is_coreclr_generated_pinvoke_wrapper(const metadata::RtMethodInfo* method) noexcept
{
    return is_coreclr_corlib_method(method) && method->name != nullptr && std::strstr(method->name, "g____PInvoke|") != nullptr;
}

// Register an internal call function by name
void PInvokes::register_pinvoke(const char* name, PInvokeFunction func, PInvokeInvoker invoker)
{
    assert(g_internalcall_map.find(name) == g_internalcall_map.end() && "PInvoke already registered");
    g_internalcall_map[name] = PInvokeRegistry{func, invoker};
}

// Get internal call by name
const PInvokeRegistry* PInvokes::get_pinvoke(const char* name)
{
    auto it = g_internalcall_map.find(name);
    if (it != g_internalcall_map.end())
        return &it->second;
    return nullptr;
}

PInvokeFunction PInvokes::get_pinvoke_function(const char* dll_name_no_ext, const char* function_name)
{
    if (function_name == nullptr)
    {
        return nullptr;
    }

    if (dll_name_no_ext != nullptr && dll_name_no_ext[0] != '\0')
    {
        utils::Utf8StringBuilder sb;
        sb.append_char('[');
        sb.append_cstr(dll_name_no_ext);
        sb.append_char(']');
        sb.append_cstr(function_name);
        auto it = g_internalcall_map.find(sb.get_const_chars());
        if (it != g_internalcall_map.end() && it->second.func != nullptr)
        {
            return it->second.func;
        }
    }

    auto it = g_internalcall_map.find(function_name);
    if (it != g_internalcall_map.end() && it->second.func != nullptr)
    {
        return it->second.func;
    }

    return nullptr;
}

// Get internal call by method info (builds full method name with params)
RtResult<const PInvokeRegistry*> PInvokes::get_pinvoke_by_method(const metadata::RtMethodInfo* method)
{
    // Try with full method name (including parameters)
    utils::Utf8StringBuilder sb;
    {
        // signature: [ModuleName]Namespace.Class.Method(params)
        sb.append_char('[');
        sb.append_cstr(method->parent->image->get_name_no_ext());
        sb.append_char(']');
        size_t length = sb.length();
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_with_params(sb, method, metadata::TypeNameFormat::InternalName));
        const char* signature_with_module = sb.get_const_chars();
        auto it = g_internalcall_map.find(signature_with_module);
        if (it != g_internalcall_map.end())
            RET_OK(&it->second);

        // signature: Namespace.Class.Method(params)
        const char* signature_without_module = sb.get_const_chars() + length;
        it = g_internalcall_map.find(signature_without_module);
        if (it != g_internalcall_map.end())
            RET_OK(&it->second);
    }

    // Try with method name without parameters
    if (!is_coreclr_corlib_method(method) || is_coreclr_generated_pinvoke_wrapper(method))
    {
        // signature: Namespace.Class.Method
        sb.clear();
        sb.append_char('[');
        sb.append_cstr(method->parent->image->get_name_no_ext());
        sb.append_char(']');
        size_t length = sb.length();
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method, metadata::TypeNameFormat::InternalName));

        const char* signature_with_module = sb.get_const_chars();
        auto it = g_internalcall_map.find(signature_with_module);
        if (it != g_internalcall_map.end())
            RET_OK(&it->second);
        const char* signature_without_module = sb.get_const_chars() + length;
        it = g_internalcall_map.find(signature_without_module);
        if (it != g_internalcall_map.end())
            RET_OK(&it->second);
    }

    if (is_coreclr_corlib_method(method) && method->parent->namespaze != nullptr && method->parent->namespaze[0] == '\0' &&
        std::strcmp(method->parent->name, "BoxCache") == 0 && std::strcmp(method->name, "GetBoxInfo") == 0)
    {
        auto it = g_internalcall_map.find("BoxCache::GetBoxInfo");
        if (it != g_internalcall_map.end())
            RET_OK(&it->second);
    }

    if (is_coreclr_corlib_method(method) && method->parent->namespaze != nullptr && method->parent->namespaze[0] == '\0' &&
        std::strcmp(method->parent->name, "CreateUninitializedCache") == 0 &&
        std::strcmp(method->name, "GetCreateUninitializedInfo") == 0)
    {
        auto it = g_internalcall_map.find("CreateUninitializedCache::GetCreateUninitializedInfo");
        if (it != g_internalcall_map.end())
            RET_OK(&it->second);
    }

    RET_OK(nullptr);
}

// Initialize internal calls system
void PInvokes::initialize()
{
    pinvokes::register_coreclr_qcall_pinvokes();
}

} // namespace vm
} // namespace leanclr

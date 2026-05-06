#include "pinvoke.h"

#include "method.h"
#include "class.h"
#include "metadata/module_def.h"
#include "utils/string_builder.h"
#include "utils/string_util.h"
#include "metadata/metadata_name.h"

namespace leanclr
{
namespace vm
{

// Static maps for internal call functions
static utils::HashMap<const char*, PInvokeRegistry, utils::CStrHasher, utils::CStrCompare> g_internalcall_map;

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
    return nullptr;
}

// Get internal call by method info (builds full method name with params)
RtResult<const PInvokeRegistry*> PInvokes::get_pinvoke_by_method(const metadata::RtMethodInfo* method)
{
    // Try with full method name (including parameters)
    utils::StringBuilder sb;
    {
        // signature: [ModuleName]Namespace.Class.Method(params)
        sb.append_char('[');
        sb.append_cstr(method->parent->image->get_name_no_ext());
        sb.append_char(']');
        size_t length = sb.length();
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_with_params(sb, method));
        const char* signature_with_module = sb.as_cstr();
        auto it = g_internalcall_map.find(signature_with_module);
        if (it != g_internalcall_map.end())
            RET_OK(&it->second);

        // signature: Namespace.Class.Method(params)
        const char* signature_without_module = sb.as_cstr() + length;
        it = g_internalcall_map.find(signature_without_module);
        if (it != g_internalcall_map.end())
            RET_OK(&it->second);
    }

    // Try with method name without parameters
    {
        // signature: Namespace.Class.Method
        sb.clear();
        sb.append_char('[');
        sb.append_cstr(method->parent->image->get_name_no_ext());
        sb.append_char(']');
        size_t length = sb.length();
        RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method));

        const char* signature_with_module = sb.as_cstr();
        auto it = g_internalcall_map.find(signature_with_module);
        if (it != g_internalcall_map.end())
            RET_OK(&it->second);
        const char* signature_without_module = sb.as_cstr() + length;
        it = g_internalcall_map.find(signature_without_module);
        if (it != g_internalcall_map.end())
            RET_OK(&it->second);
    }

    RET_OK(nullptr);
}

// Initialize internal calls system
void PInvokes::initialize()
{
}

} // namespace vm
} // namespace leanclr

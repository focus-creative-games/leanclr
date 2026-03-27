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
static utils::HashMap<const char*, PInvokeRegistry, utils::CStrHasher, utils::CStrCompare> g_pinvoke_map;

// Register an internal call function by name
void PInvokes::register_pinvoke(const char* name, PInvokeFunction func, PInvokeInvoker invoker)
{
    assert(g_pinvoke_map.find(name) == g_pinvoke_map.end() && "PInvoke already registered");
    g_pinvoke_map[name] = PInvokeRegistry{func, invoker};
}

// Get internal call by name
const PInvokeRegistry* PInvokes::get_pinvoke(const char* name)
{
    auto it = g_pinvoke_map.find(name);
    if (it != g_pinvoke_map.end())
        return &it->second;
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
        auto it = g_pinvoke_map.find(signature_with_module);
        if (it != g_pinvoke_map.end())
            RET_OK(&it->second);

        // signature: Namespace.Class.Method(params)
        const char* signature_without_module = sb.as_cstr() + length;
        it = g_pinvoke_map.find(signature_without_module);
        if (it != g_pinvoke_map.end())
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
        auto it = g_pinvoke_map.find(signature_with_module);
        if (it != g_pinvoke_map.end())
            RET_OK(&it->second);
        const char* signature_without_module = sb.as_cstr() + length;
        it = g_pinvoke_map.find(signature_without_module);
        if (it != g_pinvoke_map.end())
            RET_OK(&it->second);
    }

    RET_OK(nullptr);
}

static void nop_function()
{
}

static RtResultVoid nop_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                interp::RtStackObject* ret)
{
    RET_VOID_OK();
}

static PInvokeEntry s_pinvoke_entries[] = {
    // #if LEANCLR_ENABLE_TEST_PINVOKES
    {"Interop/Sys::LChflagsCanSetHiddenFlag", (PInvokeFunction)&nop_function, nop_invoker},
    // #endif
};

// Initialize internal calls system
void PInvokes::initialize()
{
    for (PInvokeEntry& entry : s_pinvoke_entries)
    {
        register_pinvoke(entry.name, entry.func, entry.invoker);
    }
}

} // namespace vm
} // namespace leanclr

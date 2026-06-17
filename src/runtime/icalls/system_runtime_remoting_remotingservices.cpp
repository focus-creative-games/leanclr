#include "system_runtime_remoting_remotingservices.h"

#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/class.h"

namespace leanclr
{
namespace icalls
{

namespace
{
metadata::RtClass* get_transparent_proxy_class() noexcept
{
    static metadata::RtClass* s_transparent_proxy_class = nullptr;
    static bool s_initialized = false;
    if (!s_initialized)
    {
        s_initialized = true;
        metadata::RtAssembly* corlib = vm::Assembly::get_corlib();
        if (corlib != nullptr && corlib->mod != nullptr)
        {
            auto result = corlib->mod->get_class_by_name2("System.Runtime.Remoting.Proxies", "TransparentProxy", false, false);
            if (result.is_ok())
            {
                s_transparent_proxy_class = result.unwrap();
            }
        }
    }
    return s_transparent_proxy_class;
}
} // namespace

RtResult<bool> SystemRuntimeRemotingRemotingServices::is_transparent_proxy(vm::RtObject* proxy) noexcept
{
    if (proxy == nullptr)
    {
        RET_OK(false);
    }

    metadata::RtClass* transparent_proxy_class = get_transparent_proxy_class();
    if (transparent_proxy_class == nullptr)
    {
        RET_OK(false);
    }

    RET_OK(vm::Class::is_subclass_of_initialized(proxy->klass, transparent_proxy_class, false));
}

/// @icall: System.Runtime.Remoting.RemotingServices::IsTransparentProxy(System.Object)
static RtResultVoid is_transparent_proxy_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    auto proxy = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeRemotingRemotingServices::is_transparent_proxy(proxy));
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

static vm::InternalCallEntry s_internal_call_entries_system_runtime_remoting_remotingservices[] = {
    {"System.Runtime.Remoting.RemotingServices::IsTransparentProxy(System.Object)",
     (vm::InternalCallFunction)&SystemRuntimeRemotingRemotingServices::is_transparent_proxy, is_transparent_proxy_invoker},
};

utils::Span<vm::InternalCallEntry> SystemRuntimeRemotingRemotingServices::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_runtime_remoting_remotingservices,
                                              sizeof(s_internal_call_entries_system_runtime_remoting_remotingservices) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

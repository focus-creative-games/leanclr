#pragma once
#include "metadata/rt_metadata.h"

namespace leanclr
{
namespace vm
{
// Optional host callback adaptation during cold metadata resolution. No public
// ABI types or per-method fields are introduced into the execution core.
class HostInvokers
{
  public:
    using BindingHook = RtResultVoid (*)(const metadata::RtMethodInfo*, const char*, metadata::RtInvokeMethodPointer);
    static BindingHook binding_hook;

    static RtResultVoid bind(const metadata::RtMethodInfo* method, const char* name, metadata::RtInvokeMethodPointer invoker)
    {
        if (binding_hook)
            return binding_hook(method, name, invoker);
        RET_VOID_OK();
    }
};
} // namespace vm
} // namespace leanclr

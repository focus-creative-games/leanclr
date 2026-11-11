#pragma once

#include "vm/method.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace codegen
{
// One entry per selected loop callsite, owned by the current AOT invocation.
// Keep metadata only: neither receivers nor managed values become GC roots.
// Stack ownership also isolates recursion/reentry and requires no synchronization.
struct InterfaceCallCache
{
    const metadata::RtClass* receiver_class;
    const metadata::RtMethodInfo* declaration;
    const metadata::RtMethodInfo* implementation;
};

// The caller performs the same null check as an uncached callvirt. Only cache
// successful resolutions; the existing resolver owns variance and generic
// inflation semantics, including interpreted implementations.
inline RtResult<const metadata::RtMethodInfo*> get_interface_method_impl_cached(vm::RtObject* obj, const metadata::RtMethodInfo* method,
                                                                                InterfaceCallCache& cache) noexcept
{
    const auto* klass = obj->klass;
    if (cache.receiver_class == klass && cache.declaration == method)
    {
        RET_OK(cache.implementation);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, implementation, vm::Method::get_virtual_method_impl(obj, method));
    cache.receiver_class = klass;
    cache.declaration = method;
    cache.implementation = implementation;
    RET_OK(implementation);
}
} // namespace codegen
} // namespace leanclr

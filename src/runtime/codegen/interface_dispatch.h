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

// Two receiver classes, one declaration. Hits never modify the replacement
// cursor. Misses replace slots in round-robin order so a new two-class phase
// can displace both old classes, without pinning the first receiver forever.
struct InterfaceCallCache2
{
    const metadata::RtMethodInfo* declaration;
    const metadata::RtClass* receiver_classes[2];
    const metadata::RtMethodInfo* implementations[2];
    uint8_t next_victim;
};

inline RtResult<const metadata::RtMethodInfo*> get_interface_method_impl_cached(vm::RtObject* obj, const metadata::RtMethodInfo* method,
                                                                                InterfaceCallCache2& cache) noexcept
{
    const auto* klass = obj->klass;
    if (cache.declaration == method)
    {
        if (cache.receiver_classes[0] == klass)
            RET_OK(cache.implementations[0]);
        if (cache.receiver_classes[1] == klass)
            RET_OK(cache.implementations[1]);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, implementation, vm::Method::get_virtual_method_impl(obj, method));
    // Leave every field untouched on resolution failure, including the victim
    // cursor. A different declaration invalidates both slots only on success.
    if (cache.declaration != method)
    {
        cache.declaration = method;
        cache.receiver_classes[1] = nullptr;
        cache.implementations[1] = nullptr;
        cache.next_victim = 0;
    }
    const uint8_t slot = cache.next_victim;
    cache.receiver_classes[slot] = klass;
    cache.implementations[slot] = implementation;
    cache.next_victim = slot ^ 1;
    RET_OK(implementation);
}
} // namespace codegen
} // namespace leanclr

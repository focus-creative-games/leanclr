#include "invoker_bridge.h"
#include "alloc/general_allocation.h"
#include "vm/host_invokers.h"
#include "vm/rt_exception.h"
#include "host/services.h"
#include <cstring>

namespace leanclr
{
namespace public_impl
{
namespace
{
struct Registration
{
    Registration* next;
    InvokerKind kind;
    LeanclrMethodInvoker callback;
    const char* name;
};

Registration* registrations = nullptr;

struct Binding
{
    const metadata::RtMethodInfo* method;
    InvokerKind kind;
    LeanclrMethodInvoker callback;
};

Binding* bindings = nullptr;
size_t capacity = 0, count = 0;

size_t slot(const metadata::RtMethodInfo* method, InvokerKind kind, size_t size)
{
    uintptr_t key = reinterpret_cast<uintptr_t>(method) / alignof(metadata::RtMethodInfo);
    key ^= key >> 16;
    key = key * uintptr_t(0x9e3779b1u) + static_cast<uint8_t>(kind);
    return static_cast<size_t>(key) & (size - 1);
}

LeanclrMethodInvoker find_registration(InvokerKind kind, const char* name)
{
    for (auto* entry = registrations; entry; entry = entry->next)
        if (entry->kind == kind && std::strcmp(entry->name, name) == 0)
            return entry->callback;
    return nullptr;
}

RtResult<LeanclrMethodInvoker> bind(InvokerKind kind, const metadata::RtMethodInfo* method)
{
    if (!method)
        RET_ERR(RtErr::ArgumentNull);
    if (capacity)
    {
        size_t index = slot(method, kind, capacity);
        while (bindings[index].method)
        {
            if (bindings[index].method == method && bindings[index].kind == kind)
                RET_OK(bindings[index].callback);
            index = (index + 1) & (capacity - 1);
        }
    }
    RET_ERR(RtErr::NotSupported);
}

RtResultVoid cache_binding(InvokerKind kind, const metadata::RtMethodInfo* method, LeanclrMethodInvoker callback)
{
    if (capacity)
    {
        size_t index = slot(method, kind, capacity);
        while (bindings[index].method)
        {
            if (bindings[index].method == method && bindings[index].kind == kind)
                RET_VOID_OK();
            index = (index + 1) & (capacity - 1);
        }
    }
    if (!capacity || count >= capacity - capacity / 4)
    {
        if (capacity > SIZE_MAX / 2)
            RET_ERR(RtErr::OutOfMemory);
        const size_t next = capacity ? capacity * 2 : 16;
        auto* replacement = alloc::GeneralAllocation::calloc_any<Binding>(next);
        if (!replacement)
            RET_ERR(RtErr::OutOfMemory);
        for (size_t i = 0; i < capacity; ++i)
        {
            if (!bindings[i].method)
                continue;
            size_t index = slot(bindings[i].method, bindings[i].kind, next);
            while (replacement[index].method)
                index = (index + 1) & (next - 1);
            replacement[index] = bindings[i];
        }
        alloc::GeneralAllocation::free(bindings);
        bindings = replacement;
        capacity = next;
    }
    size_t index = slot(method, kind, capacity);
    while (bindings[index].method)
        index = (index + 1) & (capacity - 1);
    bindings[index] = {method, kind, callback};
    ++count;
    RET_VOID_OK();
}

template <InvokerKind Kind>
RtResultVoid invoke(metadata::RtManagedMethodPointer pointer, const metadata::RtMethodInfo* method, const interp::RtStackObject* args,
                    interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(LeanclrMethodInvoker, callback, bind(Kind, method));
    LeanclrException* exception = nullptr;
    callback(pointer, reinterpret_cast<const LeanclrMethodInfo*>(method), reinterpret_cast<const LeanclrStackObject*>(args),
             reinterpret_cast<LeanclrStackObject*>(ret), &exception);
    if (exception)
    {
        vm::Exception::set_current_exception(reinterpret_cast<vm::RtException*>(exception));
        RET_ERR(RtErr::ManagedException);
    }
    RET_VOID_OK();
}

RtResultVoid bind_registration(const metadata::RtMethodInfo* method, const char* name, metadata::RtInvokeMethodPointer bridge)
{
    InvokerKind kind;
    if (bridge == invoke<InvokerKind::PInvoke>)
        kind = InvokerKind::PInvoke;
    else if (bridge == invoke<InvokerKind::InternalCall>)
        kind = InvokerKind::InternalCall;
    else if (bridge == invoke<InvokerKind::NewObjectInternalCall>)
        kind = InvokerKind::NewObjectInternalCall;
    else if (bridge == invoke<InvokerKind::Intrinsic>)
        kind = InvokerKind::Intrinsic;
    else if (bridge == invoke<InvokerKind::NewObjectIntrinsic>)
        kind = InvokerKind::NewObjectIntrinsic;
    else
        RET_VOID_OK();
    auto callback = find_registration(kind, name);
    if (!callback)
        RET_ERR(RtErr::NotSupported);
    return cache_binding(kind, method, callback);
}

} // namespace

bool prepare_invoker(InvokerKind kind, const char* name, LeanclrMethodInvoker callback, const char*& owned_name, metadata::RtInvokeMethodPointer& bridge)
{
    vm::HostInvokers::binding_hook = bind_registration;
    owned_name = name;
    bridge = nullptr;
    if (!name || !*name)
        return false;
    size_t bytes;
    if (!utils::CheckedSize::add(sizeof(Registration), std::strlen(name), bytes) || !utils::CheckedSize::add(bytes, size_t(1), bytes))
        return false;
    auto* entry = static_cast<Registration*>(alloc::GeneralAllocation::malloc(bytes));
    if (!entry)
    {
        host::Services::logf(host::LogLevel::Error, "Cannot register public invoker: out of memory");
        return false;
    }
    char* copy = reinterpret_cast<char*>(entry + 1);
    std::memcpy(copy, name, std::strlen(name) + 1);
    *entry = {registrations, kind, callback, copy};
    registrations = entry;
    owned_name = copy;
    if (!callback)
        return true;
    switch (kind)
    {
    case InvokerKind::PInvoke:
        bridge = invoke<InvokerKind::PInvoke>;
        break;
    case InvokerKind::InternalCall:
        bridge = invoke<InvokerKind::InternalCall>;
        break;
    case InvokerKind::NewObjectInternalCall:
        bridge = invoke<InvokerKind::NewObjectInternalCall>;
        break;
    case InvokerKind::Intrinsic:
        bridge = invoke<InvokerKind::Intrinsic>;
        break;
    case InvokerKind::NewObjectIntrinsic:
        bridge = invoke<InvokerKind::NewObjectIntrinsic>;
        break;
    }
    return true;
}
} // namespace public_impl
} // namespace leanclr

#pragma once

#include "rt_managed_types.h"
#include "metadata/rt_metadata.h"
#include "interp/interp_defs.h"

namespace leanclr
{
namespace vm
{

// Structure containing invoker type and pointer
using RtInvokerType = metadata::RtInvokerType;

struct InvokeTypeAndMethod
{
    RtInvokerType invoker_type;
    metadata::RtInvokeMethodPointer invoker;
    metadata::RtInvokeMethodPointer virtual_invoker;

    InvokeTypeAndMethod(RtInvokerType type, metadata::RtInvokeMethodPointer invoker, metadata::RtInvokeMethodPointer virtual_invoker)
        : invoker_type(type), invoker(invoker), virtual_invoker(virtual_invoker)
    {
    }

    InvokeTypeAndMethod(RtInvokerType type, metadata::RtInvokeMethodPointer invoker) : invoker_type(type), invoker(invoker), virtual_invoker(invoker)
    {
    }
};

class Shim
{
  public:
    // Public functions
    static RtResult<InvokeTypeAndMethod> get_invoker(const metadata::RtMethodInfo* method);
    static metadata::RtManagedMethodPointer get_method_pointer(const metadata::RtMethodInfo* method);
};

} // namespace vm
} // namespace leanclr

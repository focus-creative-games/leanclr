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

    InvokeTypeAndMethod(RtInvokerType type, metadata::RtInvokeMethodPointer invoker) : invoker_type(type), invoker(invoker)
    {
    }
};


struct MethodAndVirtualMethod
{
    metadata::RtManagedMethodPointer method_ptr;
    metadata::RtManagedMethodPointer virtual_method_ptr;
    MethodAndVirtualMethod(metadata::RtManagedMethodPointer method_ptr, metadata::RtManagedMethodPointer virtual_method_ptr)
        : method_ptr(method_ptr), virtual_method_ptr(virtual_method_ptr)
    {
    }
};

class Shim
{
  public:
    // Public functions
    static RtResult<InvokeTypeAndMethod> get_invoker(const metadata::RtMethodInfo* method);
    static MethodAndVirtualMethod get_method_pointer(const metadata::RtMethodInfo* method);
};

} // namespace vm
} // namespace leanclr

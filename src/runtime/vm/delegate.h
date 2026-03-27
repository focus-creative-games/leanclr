#pragma once

#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{
class Delegate
{
  public:
    // Static initializer
    static RtResultVoid initialize();
    static RtResultVoid constructor_delegate(RtMulticastDelegate* del, RtObject* target, const metadata::RtMethodInfo* method);
    static RtResult<RtMulticastDelegate*> new_delegate(metadata::RtClass* delelgate_type, RtObject* target, const metadata::RtMethodInfo* method);
    // Placeholder delegate invokers (to be implemented)
    static RtResultVoid call_delegate_ctor_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret);

    static RtResultVoid invoke_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret);

    static RtResultVoid begin_invoke_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                      const interp::RtStackObject* params, interp::RtStackObject* ret);
    static RtResultVoid end_invoke_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                    const interp::RtStackObject* params, interp::RtStackObject* ret);
    static RtResultVoid newobj_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret);
};
} // namespace vm
} // namespace leanclr

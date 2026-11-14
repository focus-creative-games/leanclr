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
    // AOT typed-call fast path. Returns null for bindings needing the general
    // invoker. No new delegate fields, caches or per-call allocations.
    static const metadata::RtMethodInfo* get_singlecast_aot_target(RtObject* delegate_object, const metadata::RtMethodInfo* invoke, RtObject*& target) noexcept;
    static RtResult<RtMulticastDelegate*> create_delegate_from_reflection(RtReflectionType* delegate_type, RtObject* target,
                                                                          const metadata::RtMethodInfo* method, bool throw_on_bind) noexcept;
    static RtResultVoid constructor_delegate(RtMulticastDelegate* del, RtObject* target, const metadata::RtMethodInfo* method) noexcept;
    static RtResult<RtMulticastDelegate*> new_delegate(const metadata::RtClass* delelgate_type, RtObject* target,
                                                       const metadata::RtMethodInfo* method) noexcept;
    // Placeholder delegate invokers (to be implemented)
    static RtResultVoid call_delegate_ctor_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;

    static RtResultVoid invoke_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;

    static RtResultVoid begin_invoke_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                      const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;
    static RtResultVoid end_invoke_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                    const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;
    static RtResultVoid newobj_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;
};
} // namespace vm
} // namespace leanclr

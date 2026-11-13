#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace vm
{
// Native layout is deliberately separate from managed field/GC layout.
class StructMarshal
{
  public:
    static RtResult<int32_t> size_of(const metadata::RtClass* klass);
    static RtResult<intptr_t> offset_of(const metadata::RtClass* klass, const char* name);
    static RtResultVoid to_native(RtObject* obj, void* destination, bool delete_old);
    // The caller must root managed references in value for the duration of this call.
    static RtResultVoid to_native_data(const metadata::RtClass* klass, const void* value, void* destination, bool delete_old);
    static RtResultVoid from_native(const void* source, RtObject* obj);
    static RtResult<RtObject*> from_native(const void* source, const metadata::RtClass* klass);
    static RtResultVoid destroy(void* native, const metadata::RtClass* klass);
};
} // namespace vm
} // namespace leanclr

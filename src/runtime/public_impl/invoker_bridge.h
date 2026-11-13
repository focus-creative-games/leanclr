#pragma once
#include "public/leanclr.h"
#include "metadata/rt_metadata.h"

namespace leanclr
{
namespace public_impl
{
enum class InvokerKind : uint8_t
{
    PInvoke,
    InternalCall,
    NewObjectInternalCall,
    Intrinsic,
    NewObjectIntrinsic
};
// Owns the registration name for as long as runtime metadata can reference it.
bool prepare_invoker(InvokerKind kind, const char* name, LeanclrMethodInvoker callback, const char*& owned_name, metadata::RtInvokeMethodPointer& bridge);
} // namespace public_impl
} // namespace leanclr

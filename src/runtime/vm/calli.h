#pragma once
#include "metadata/rt_metadata.h"

namespace leanclr
{
namespace vm
{
class Calli
{
  public:
    // Producers accept only trusted runtime metadata. Consumers validate membership
    // before dereferencing a possibly user-supplied function-pointer value.
    static RtResultVoid register_managed_target(const metadata::RtMethodInfo* method);
    static RtResultVoid validate_managed_target(const metadata::RtMethodSig* signature, const metadata::RtMethodInfo* target);
    static RtResultVoid validate_managed_signature(const metadata::RtMethodSig* signature);
};
} // namespace vm
} // namespace leanclr

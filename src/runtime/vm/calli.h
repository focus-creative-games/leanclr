#pragma once
#include "metadata/rt_metadata.h"

namespace leanclr
{
namespace vm
{
struct CalliSite;

class Calli
{
  public:
    // A tiny per-emitted-callsite fast cache. A shared generic body can switch
    // caller contexts; the process index retains one site per actual caller/IP.
    struct SiteCache
    {
        const metadata::RtMethodInfo* caller = nullptr;
        CalliSite* site = nullptr;
    };

    static RtResultVoid invoke_managed(SiteCache& cache, const metadata::RtMethodInfo* caller, uint32_t il_offset, const metadata::RtMethodInfo* target,
                                       interp::RtStackObject* args, interp::RtStackObject* ret);

    // Producers accept only trusted runtime metadata. Consumers validate membership
    // before dereferencing a possibly user-supplied function-pointer value.
    static void* value_receiver(const metadata::RtMethodInfo* target, void* receiver);
    static RtResultVoid register_managed_target(const metadata::RtMethodInfo* method);
    static RtResultVoid validate_managed_target(const metadata::RtMethodSig* signature, const metadata::RtMethodInfo* target);
    static RtResultVoid validate_managed_signature(const metadata::RtMethodSig* signature);
};
} // namespace vm
} // namespace leanclr

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
    struct ManagedBinding
    {
        const metadata::RtMethodSig* signature;
        const metadata::RtMethodInfo* target;
    };

    static RtResultVoid validate_managed_binding(ManagedBinding& binding, const metadata::RtMethodInfo* target);

    // A tiny per-emitted-callsite fast cache. A shared generic body can switch
    // caller contexts; the process index retains one site per actual caller/IP.
    struct SiteCache
    {
        const metadata::RtMethodInfo* caller = nullptr;
        CalliSite* site = nullptr;
    };

    static RtResultVoid invoke_managed(SiteCache& cache, const metadata::RtMethodInfo* caller, uint32_t il_offset, const metadata::RtMethodInfo* target,
                                       interp::RtStackObject* args, interp::RtStackObject* ret);

    static uint8_t char_to_native(uint16_t value);
    static uint16_t char_from_native(uint8_t value);

    // Tables must remain alive for the runtime lifetime; registration is cold.
    static RtResultVoid register_native_bridges(const metadata::RtCalliBridge* entries, uint32_t count);
    static RtResult<const metadata::RtCalliBridge*> bind_native(const metadata::RtMethodSig* signature);
    static RtResultVoid invoke_native(const metadata::RtCalliBridge* bridge, void* target, interp::RtStackObject* args, interp::RtStackObject* ret);
    static RtResultVoid invoke_unmanaged(SiteCache& cache, const metadata::RtMethodInfo* caller, uint32_t offset, void* target, interp::RtStackObject* args,
                                         interp::RtStackObject* ret);

    // Producers accept only trusted runtime metadata. Consumers validate membership
    // before dereferencing a possibly user-supplied function-pointer value.
    static void* value_receiver(const metadata::RtMethodInfo* target, void* receiver);
    static RtResultVoid register_managed_target(const metadata::RtMethodInfo* method);
    static RtResultVoid validate_managed_target(const metadata::RtMethodSig* signature, const metadata::RtMethodInfo* target);
    static RtResultVoid validate_managed_signature(const metadata::RtMethodSig* signature);

  private:
    static bool is_managed_target(const metadata::RtMethodInfo* target);
};
} // namespace vm
} // namespace leanclr

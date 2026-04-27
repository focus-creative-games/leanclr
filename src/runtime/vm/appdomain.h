#pragma once

#include "metadata/rt_metadata.h"
#include "rt_managed_types.h"
#include "utils/rt_span.h"

namespace leanclr
{
namespace vm
{
class AppDomain
{
  public:
    static RtResult<RtAppDomain*> init_default_app_domain();
    static RtResultVoid initialize_context();

    static RtAppDomain* get_default_appdomain();
    static RtMonoAppDomain* get_default_mono_appdomain();
    static RtAppContext* get_default_appcontext();
    static RtObject* get_ephemeron_tombstone();
    static RtObject* get_setup();
    static const char* get_friendly_name();

    static RtObject* get_domain_data(RtString* name);
    static void set_domain_data(RtString* name, RtObject* data);

    static int32_t get_appdomain_id();

    static utils::Span<metadata::RtModuleDef*> get_modules(RtAppDomain* this_domain);
};
} // namespace vm
} // namespace leanclr

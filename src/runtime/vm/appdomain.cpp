#include "appdomain.h"

#include "class.h"
#include "object.h"
#include "rt_string.h"
#include "rt_thread.h"
#include "utils/hashmap.h"
#include "utils/string_util.h"
#include "metadata/module_def.h"
#include "settings.h"
#include "gc/roots/gc_roots.h"

namespace leanclr
{
namespace vm
{

namespace
{

RtAppDomain* g_default_appdomain = nullptr;
RtMonoAppDomain g_default_mono_domain = {};
utils::HashMap<utils::Utf16StrWithLen, RtObject*, utils::Utf16StrHasher, utils::Utf16StrCompare> g_appdomain_private_data;

RtResult<RtObject*> create_appdomain_setup()
{
    auto appdomain_setup_class = Class::get_corlib_types().cls_appdomain_setup;
    return LEANCLR_NEWOBJ_INTERNAL(appdomain_setup_class, "AppDomain::create_appdomain_setup");
}
} // namespace

RtResult<RtAppDomain*> AppDomain::init_default_app_domain()
{
    assert(g_default_appdomain == nullptr);
    auto mono_app_domain = &g_default_mono_domain;

    auto& corlib_types = Class::get_corlib_types();
    auto appdomain_class = corlib_types.cls_appdomain;

    auto default_appdomain_res = LEANCLR_NEWOBJ_INTERNAL(appdomain_class, "AppDomain::init_default_app_domain");
    if (default_appdomain_res.is_err())
    {
        return RtResult<RtAppDomain*>::Err(default_appdomain_res.unwrap_err());
    }
    auto default_appdomain = static_cast<RtAppDomain*>(default_appdomain_res.unwrap());

    auto appdomain_setup_res = create_appdomain_setup();
    if (appdomain_setup_res.is_err())
    {
        return RtResult<RtAppDomain*>::Err(appdomain_setup_res.unwrap_err());
    }
    auto appdomain_setup = appdomain_setup_res.unwrap();

    mono_app_domain->domain_id = 1;
    const char* domain_name = Settings::get_domain_name();
    // we has dup string in Settings, so we don't need to dup it here
    mono_app_domain->friendly_name = domain_name != nullptr ? domain_name : utils::StringUtil::strdup("LeanCLR-Domain");
    mono_app_domain->appdomain = default_appdomain;

    auto ephemeron_res = LEANCLR_NEWOBJ_INTERNAL(corlib_types.cls_object, "AppDomain::ephemeron_tombstone");
    if (ephemeron_res.is_err())
    {
        return RtResult<RtAppDomain*>::Err(ephemeron_res.unwrap_err());
    }

    mono_app_domain->ephemeron_tombstone = ephemeron_res.unwrap();
    mono_app_domain->setup = appdomain_setup;

    default_appdomain->mono_app_domain = mono_app_domain;

    g_default_appdomain = default_appdomain;

    return RtResult<RtAppDomain*>::Ok(default_appdomain);
}

RtResultVoid AppDomain::initialize_context()
{
    auto& corlib_types = Class::get_corlib_types();
    auto appcontext_class = corlib_types.cls_appcontext;

    auto context_res = LEANCLR_NEWOBJ_INTERNAL(appcontext_class, "AppDomain::initialize_context");
    RET_ERR_ON_FAIL(context_res);
    auto context = reinterpret_cast<RtAppContext*>(context_res.unwrap());

    context->domain_id = g_default_mono_domain.domain_id;
    context->context_id = 0;
    g_default_mono_domain.context = context;

    auto current_thread = Thread::get_current_thread();
    current_thread->internal_thread->current_appcontext = reinterpret_cast<RtObject*>(context);

    RET_VOID_OK();
}

RtAppDomain* AppDomain::get_default_appdomain()
{
    assert(g_default_appdomain != nullptr);
    return g_default_appdomain;
}

RtMonoAppDomain* AppDomain::get_default_mono_appdomain()
{
    return &g_default_mono_domain;
}

RtAppContext* AppDomain::get_default_appcontext()
{
    return g_default_mono_domain.context;
}

RtObject* AppDomain::get_ephemeron_tombstone()
{
    return g_default_mono_domain.ephemeron_tombstone;
}

RtObject* AppDomain::get_setup()
{
    return g_default_mono_domain.setup;
}

const char* AppDomain::get_friendly_name()
{
    return g_default_mono_domain.friendly_name;
}

RtObject* AppDomain::get_domain_data(RtString* name)
{
    utils::Utf16StrWithLen key(String::get_chars_ptr(name), static_cast<size_t>(String::get_length(name)));
    auto it = g_appdomain_private_data.find(key);
    if (it != g_appdomain_private_data.end())
    {
        return it->second;
    }
    return nullptr;
}

void AppDomain::set_domain_data(RtString* name, RtObject* data)
{
    utils::Utf16StrWithLen key(String::get_chars_ptr(name), static_cast<size_t>(String::get_length(name)));
    auto it = g_appdomain_private_data.find(key);
    ;
    if (it != g_appdomain_private_data.end())
    {
        it->second = data;
        return;
    }

    const Utf16Char* new_chars = utils::StringUtil::strdup_utf16_without_null_terminator(key.str, key.length);
    key.str = new_chars;
    g_appdomain_private_data.insert({key, data});
}

int32_t AppDomain::get_appdomain_id()
{
    return g_default_mono_domain.domain_id;
}

void AppDomain::get_modules(RtAppDomain* this_domain, utils::Vector<metadata::RtModuleDef*>& modules)
{
    (void)this_domain;
    metadata::RtModuleDef::get_registered_modules(modules);
}

static void visit_appdomain_private_data(gc::GcVisitObjectRoot visit, void* userdata)
{
    for (utils::HashMap<utils::Utf16StrWithLen, RtObject*, utils::Utf16StrHasher, utils::Utf16StrCompare>::const_iterator it = g_appdomain_private_data.begin();
         it != g_appdomain_private_data.end(); ++it)
    {
        if (it->second != nullptr)
        {
            visit(it->second, userdata);
        }
    }
}

void register_appdomain_gc_roots()
{
    gc::GcRoots::register_slot(reinterpret_cast<RtObject**>(&g_default_appdomain));

    gc::GcRoots::register_slot(reinterpret_cast<RtObject**>(&g_default_mono_domain.appdomain));
    gc::GcRoots::register_slot(reinterpret_cast<RtObject**>(&g_default_mono_domain.setup));
    gc::GcRoots::register_slot(reinterpret_cast<RtObject**>(&g_default_mono_domain.ephemeron_tombstone));
    gc::GcRoots::register_slot(reinterpret_cast<RtObject**>(&g_default_mono_domain.context));

    gc::GcRoots::register_visit_object_roots(visit_appdomain_private_data);
}
} // namespace vm
} // namespace leanclr

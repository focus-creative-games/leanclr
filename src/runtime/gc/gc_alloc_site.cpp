#include "gc_alloc_site.h"

#include <cstdio>
#include <cstring>

#include "alloc/general_allocation.h"
#include "metadata/rt_metadata.h"
#include "utils/rt_vector.h"

namespace leanclr
{
namespace gc
{

GcAllocSite GcAllocSite::none()
{
    GcAllocSite site = {};
    site.kind = GcAllocSiteKind::None;
    return site;
}

GcAllocSite GcAllocSite::make_codegen(const char* file, uint32_t line, const char* managed_method)
{
    GcAllocSite site = {};
    site.kind = GcAllocSiteKind::Codegen;
    site.u.codegen.file = file;
    site.u.codegen.line = line;
    site.u.codegen.managed_method = managed_method;
    return site;
}

GcAllocSite GcAllocSite::make_interp(const metadata::RtMethodInfo* method, uint32_t il_offset)
{
    GcAllocSite site = {};
    site.kind = GcAllocSiteKind::Interp;
    site.u.interp.method = method;
    site.u.interp.il_offset = il_offset;
    return site;
}

GcAllocSite GcAllocSite::make_internal(const char* file, uint32_t line, const char* native_method)
{
    GcAllocSite site = {};
    site.kind = GcAllocSiteKind::Internal;
    site.u.internal.file = file;
    site.u.internal.line = line;
    site.u.internal.native_method = native_method;
    return site;
}

struct SiteEntry
{
    GcAllocSite site;
    char* description;
};

static utils::Vector<SiteEntry> s_sites;
static uint32_t s_next_alloc_id = 1;

uint32_t GcAllocSite::intern_site_id() const
{
#if LEANCLR_GC_DEBUG
    if (kind == GcAllocSiteKind::None)
    {
        return 0;
    }
    for (size_t i = 0; i < s_sites.size(); ++i)
    {
        const SiteEntry& e = s_sites[i];
        if (e.site.kind != kind)
        {
            continue;
        }
        switch (kind)
        {
        case GcAllocSiteKind::Codegen:
            if (e.site.u.codegen.file == u.codegen.file && e.site.u.codegen.line == u.codegen.line &&
                e.site.u.codegen.managed_method == u.codegen.managed_method)
            {
                return static_cast<uint32_t>(i + 1);
            }
            break;
        case GcAllocSiteKind::Interp:
            if (e.site.u.interp.method == u.interp.method && e.site.u.interp.il_offset == u.interp.il_offset)
            {
                return static_cast<uint32_t>(i + 1);
            }
            break;
        case GcAllocSiteKind::Internal:
            if (e.site.u.internal.file == u.internal.file && e.site.u.internal.line == u.internal.line &&
                e.site.u.internal.native_method == u.internal.native_method)
            {
                return static_cast<uint32_t>(i + 1);
            }
            break;
        default:
            break;
        }
    }

    SiteEntry entry;
    entry.site = *this;
    entry.description = nullptr;
    s_sites.push_back(entry);
    const uint32_t site_id = static_cast<uint32_t>(s_sites.size());
    // Description built lazily in get_site_description.
    return site_id;
#else
    (void)this;
    return 0;
#endif
}

void GcAllocSite::get_site_description(uint32_t site_id, utils::Utf8StringBuilder& sb)
{
#if LEANCLR_GC_DEBUG
    if (site_id == 0 || site_id > s_sites.size())
    {
        sb.append_cstr("<unknown>");
        return;
    }
    SiteEntry& e = s_sites[site_id - 1];
    if (e.description != nullptr)
    {
        sb.append_cstr(e.description);
        return;
    }
    // Build a short description on first use (stored in fixed heap via malloc).
    char buf[512];
    switch (e.site.kind)
    {
    case GcAllocSiteKind::Codegen:
        std::snprintf(buf, sizeof(buf), "%s:%u @ %s", e.site.u.codegen.file ? e.site.u.codegen.file : "?", e.site.u.codegen.line,
                      e.site.u.codegen.managed_method ? e.site.u.codegen.managed_method : "?");
        break;
    case GcAllocSiteKind::Interp:
        if (e.site.u.interp.method && e.site.u.interp.method->parent)
        {
            std::snprintf(buf, sizeof(buf), "%s::%s IL+%u", e.site.u.interp.method->parent->namespaze ? e.site.u.interp.method->parent->namespaze : "",
                          e.site.u.interp.method->name ? e.site.u.interp.method->name : "?", e.site.u.interp.il_offset);
        }
        else
        {
            std::snprintf(buf, sizeof(buf), "interp IL+%u", e.site.u.interp.il_offset);
        }
        break;
    case GcAllocSiteKind::Internal:
        std::snprintf(buf, sizeof(buf), "%s:%u %s", e.site.u.internal.file ? e.site.u.internal.file : "?", e.site.u.internal.line,
                      e.site.u.internal.native_method ? e.site.u.internal.native_method : "?");
        break;
    default:
        std::snprintf(buf, sizeof(buf), "<none>");
        break;
    }
    sb.append_cstr(buf);
#else
    (void)site_id;
    (void)sb;
#endif
}

} // namespace gc
} // namespace leanclr

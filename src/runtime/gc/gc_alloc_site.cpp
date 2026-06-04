#include "gc_alloc_site.h"

#include <cstdio>
#include <cstring>

#include "alloc/general_allocation.h"
#include "metadata/rt_metadata.h"
#include "utils/rt_vector.h"
#include "utils/hashset.h"

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

#if LEANCLR_GC_DEBUG
struct GcAllocSiteHash
{
    size_t operator()(const GcAllocSite* site) const noexcept
    {
        size_t h = std::hash<size_t>()((size_t)site->kind);
        switch (site->kind)
        {
        case GcAllocSiteKind::Codegen:
            return h ^ std::hash<const char*>()(site->u.codegen.file) ^ std::hash<uint32_t>()(site->u.codegen.line) ^
                   std::hash<const char*>()(site->u.codegen.managed_method);
        case GcAllocSiteKind::Interp:
            return h ^ std::hash<const metadata::RtMethodInfo*>()(site->u.interp.method) ^ std::hash<uint32_t>()(site->u.interp.il_offset);
        case GcAllocSiteKind::Internal:
            return h ^ std::hash<const char*>()(site->u.internal.file) ^ std::hash<uint32_t>()(site->u.internal.line) ^
                   std::hash<const char*>()(site->u.internal.native_method);
        default:
            assert(false && "Invalid GcAllocSiteKind");
            return 0;
        }
    }
};

struct GcAllocSiteEqual
{
    bool operator()(const GcAllocSite* a, const GcAllocSite* b) const noexcept
    {
        if (a->kind != b->kind)
        {
            return false;
        }
        switch (a->kind)
        {
        case GcAllocSiteKind::Codegen:
            return a->u.codegen.file == b->u.codegen.file && a->u.codegen.line == b->u.codegen.line &&
                   a->u.codegen.managed_method == b->u.codegen.managed_method;
        case GcAllocSiteKind::Interp:
            return a->u.interp.method == b->u.interp.method && a->u.interp.il_offset == b->u.interp.il_offset;
        case GcAllocSiteKind::Internal:
            return a->u.internal.file == b->u.internal.file && a->u.internal.line == b->u.internal.line &&
                   a->u.internal.native_method == b->u.internal.native_method;
        default:
            assert(false && "Invalid GcAllocSiteKind");
            return false;
        }
    }
};

static utils::HashSet<const GcAllocSite*, GcAllocSiteHash, GcAllocSiteEqual> s_sites;
#endif

const GcAllocSite* GcAllocSite::intern_site() const
{
#if LEANCLR_GC_DEBUG
    assert(kind != GcAllocSiteKind::None);
    auto it = s_sites.find(this);
    if (it != s_sites.end())
    {
        return *it;
    }
    const GcAllocSite* new_site = new GcAllocSite(*this);
    s_sites.insert(new_site);
    return new_site;
#else
    (void)this;
    return nullptr;
#endif
}

} // namespace gc
} // namespace leanclr

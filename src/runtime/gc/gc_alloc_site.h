#pragma once

#include "core/rt_base.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace metadata
{
struct RtClass;
struct RtMethodInfo;
}

namespace gc
{

enum class GcAllocSiteKind : uint8_t
{
    None = 0,
    Codegen,
    Interp,
    Internal,
};

struct GcAllocSite
{
    GcAllocSiteKind kind;

    union
    {
        struct
        {
            const char* file;
            uint32_t line;
            const char* managed_method;
        } codegen;
        struct
        {
            const metadata::RtMethodInfo* method;
            uint32_t il_offset;
        } interp;
        struct
        {
            const char* file;
            uint32_t line;
            const char* native_method;
        } internal;
    } u;
    uint32_t intern_site_id() const;

    static GcAllocSite none();
    static GcAllocSite make_codegen(const char* file, uint32_t line, const char* managed_method);
    static GcAllocSite make_interp(const metadata::RtMethodInfo* method, uint32_t il_offset);
    static GcAllocSite make_internal(const char* file, uint32_t line, const char* native_method);

    static void get_site_description(uint32_t site_id, utils::Utf8StringBuilder& sb);
};


} // namespace gc
} // namespace leanclr

#pragma once

#include "core/rt_base.h"
#include "rt_managed_types.h"
#include "rt_thread.h"
#include "utils/rt_span.h"

namespace leanclr
{
namespace vm
{
class Assembly
{
  public:
    static RtResult<metadata::RtAssembly*> load_corlib();
    static metadata::RtAssembly* get_corlib();
    static metadata::RtAssembly* find_by_name(const char* name_no_ext);
    static RtResult<metadata::RtAssembly*> load_by_name(const char* name_no_ext);
    static RtResult<metadata::RtAssembly*> load_by_name(RtAppDomain* app_domain, const char* name_no_ext, RtObject* evidence, bool ref_only,
                                                        RtStackCrawlMark& stack_crawl_mark);
    static RtResult<metadata::RtAssembly*> load_from_data(const utils::Span<byte> dllData, const utils::Span<byte>* symbolData);
    static RtResult<metadata::RtAssembly*> load_from_data(RtAppDomain* app_domain, RtArray* dll_data, RtArray* symbol_data, RtObject* evidence, bool ref_only);

    static RtResult<RtArray*> get_types(metadata::RtAssembly* assembly, bool exported_only);
};
} // namespace vm
} // namespace leanclr

#pragma once
#include "core/stl_compat.h"

#include "module_def.h"

namespace leanclr
{
namespace metadata
{
typedef RtInvokeMethodPointer AotInvoker;

class AotModule
{
  public:
    static void register_aot_modules(const RtAotModulesData* initData);
    static const RtAotModuleData* find_aot_module_by_name(const char* module_name);
    static std::optional<RtAotMethodImplData> find_aot_method_impl(const RtMethodInfo* method);
    static const RtAotMethodDefData* find_aot_method_def_impl(const RtModuleDef* module, EncodedTokenId token);
};
} // namespace metadata
} // namespace leanclr
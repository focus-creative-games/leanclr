#pragma once
#include "core/stl_compat.h"

#include "module_def.h"

namespace leanclr
{
namespace metadata
{
typedef RtInvokeMethodPointer AotInvoker;

struct RtAotMethodBinding
{
    const RtAotMethodDefData* implementation;
    uint8_t return_protocol;
};

class AotModule
{
  public:
    static RtResultVoid register_aot_modules(const RtAotModulesData* initData);
    static RtAotMethodBinding find_aot_method_binding(const RtMethodInfo* method);
    static const RtAotGenericMethodData* find_aot_generic_method_impl(const RtMethodInfo* method);
    static const RtAotModuleData* find_aot_module_by_name(const char* module_name);
    static const RtAotMethodDefData* find_aot_method_impl(const RtMethodInfo* method);
    static const RtAotMethodDefData* find_aot_method_def_impl(const RtModuleDef* module, EncodedTokenId token);
    static const RtAotMethodMonoPInvokeCallbackData* find_mono_pinvoke_callback_method(const RtModuleDef* module, EncodedTokenId token);

  public:
#if LEANCLR_BCL_CORECLR_NET10
    static RtResult<const RtMethodInfo*> find_mono_pinvoke_callback_method_by_native_ptr(RtNativeMethodPointer ptr);
#endif
};
} // namespace metadata
} // namespace leanclr

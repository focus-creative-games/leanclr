#pragma once

#include "rt_metadata.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace metadata
{
class MetadataName
{
  public:
    static RtResultVoid append_klass_full_name(utils::StringBuilder& sb, const RtClass* klass);
    static RtResultVoid append_type_sig_name(utils::StringBuilder& sb, const RtTypeSig* type_sig);
    static RtResultVoid append_method_sig_name(utils::StringBuilder& sb, const RtMethodSig* method_sig);
    static RtResultVoid append_method_full_name_with_params(utils::StringBuilder& sb, const RtMethodInfo* method);
    static RtResultVoid append_method_full_name_without_params(utils::StringBuilder& sb, const RtMethodInfo* method);
    static const char* get_call_convention_name(RtSigType call_conv);

    // static RtResult<const char*> build_class_full_name(const RtClass* klass);
    // static RtResult<const char*> build_method_full_name_with_params(const RtMethodInfo* method);
    // static RtResult<const char*> build_method_full_name_without_params(const RtMethodInfo* method);
};
} // namespace metadata
} // namespace leanclr

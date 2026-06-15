#pragma once

#include "rt_metadata.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace metadata
{

enum class TypeNameFormat
{
    IL,
    Reflection,
    FullName,
    AssemblyQualified,
    InternalName,
    DnlibToString,
};

class MetadataName
{
  public:
    static RtResultVoid append_klass_full_name_without_generic_params(utils::Utf8StringBuilder& sb, const RtClass* klass, TypeNameFormat format);

    static RtResultVoid append_type_full_name(utils::Utf8StringBuilder& sb, const metadata::RtTypeSig* typeSig, TypeNameFormat format, bool nested);
    static void append_assembly_name(utils::Utf8StringBuilder& sb, const RtAssemblyName& assemblyName);
    // static RtResultVoid append_type_sig_name(utils::Utf8StringBuilder& sb, const RtTypeSig* type_sig, TypeNameFormat format);
    static RtResultVoid append_method_sig_name(utils::Utf8StringBuilder& sb, const RtMethodSig* method_sig, TypeNameFormat format);
    static RtResultVoid append_method_full_name_with_params(utils::Utf8StringBuilder& sb, const RtMethodInfo* method, TypeNameFormat format);
    static RtResultVoid append_method_full_name_without_params(utils::Utf8StringBuilder& sb, const RtMethodInfo* method, TypeNameFormat format);
    static const char* get_call_convention_name(RtSigType call_conv);

    // static RtResult<const char*> build_class_full_name(const RtClass* klass);
    // static RtResult<const char*> build_method_full_name_with_params(const RtMethodInfo* method);
    // static RtResult<const char*> build_method_full_name_without_params(const RtMethodInfo* method);
};
} // namespace metadata
} // namespace leanclr

#pragma once
#include "rt_managed_types.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace vm
{

enum class TypeNameFormat
{
    IL,
    Reflection,
    FullName,
    AssemblyQualified,
};

struct AssemblyQualifiedNames
{
  public:
    std::string assembly_name;
    std::string type_full_name;
    std::string version;
    std::string culture;
    std::string public_key_token;

    AssemblyQualifiedNames(const char* assembly_qualified_name, size_t name_len) : _str(assembly_qualified_name), _len(name_len), _pos(0)
    {
    }

    void parse();

  private:
    void trim_whitespaces();
    std::string_view parse_after(char delimiter);
    const char* _str;
    size_t _len;
    size_t _pos;
};

class Type
{
  public:
    static RtResult<bool> is_value_type(const metadata::RtTypeSig* typeSig);
    static RtResult<size_t> get_size_of_type(const metadata::RtTypeSig* typeSig);
    static bool is_generic_param(const metadata::RtTypeSig* typeSig);
    static bool contains_generic_param(const metadata::RtTypeSig* typeSig);
    static bool contains_not_instantiated_generic_param_in_generic_inst(const metadata::RtGenericInst* genericInst);
    static RtResult<const metadata::RtTypeSig*> resolve_assembly_qualified_name(metadata::RtModuleDef* default_mod, const char* type_full_name, size_t name_len,
                                                                                bool ignore_case);
    static RtResult<RtString*> get_full_name(const metadata::RtTypeSig* typeSig, bool full_name, bool assembly_qualified);
    static RtResultVoid append_type_full_name(utils::StringBuilder& sb, const metadata::RtTypeSig* typeSig, TypeNameFormat format, bool nested);
    static void append_assembly_name(utils::StringBuilder& sb, const metadata::RtAssemblyName& assemblyName);
    static RtResult<metadata::RtClass*> get_declaring_type(const metadata::RtTypeSig* typeSig);
    static RtResult<const metadata::RtMethodInfo*> get_declaring_method_of_mvar(const metadata::RtTypeSig* typeSig);
    static RtResultVoid parse_assembly_name(const char* input, size_t input_len, metadata::RtMonoAssemblyName* assembly_name_info, bool* is_version_defined,
                                            bool* is_token_defined);
    static RtResult<const metadata::RtTypeSig*> parse_assembly_qualified_type(metadata::RtModuleDef* default_mod, const char* assembly_qualified_type_name,
                                                                              size_t name_len, bool ignore_case);
};
} // namespace vm
} // namespace leanclr

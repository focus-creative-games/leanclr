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

/// Parses a .NET assembly display name (fully qualified assembly name), e.g.
/// "MyAssembly, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null".
/// This is assembly-only; use AssemblyQualifiedNames for assembly-qualified type names.
class FullyQualifiedAssemblyName
{
  public:
    FullyQualifiedAssemblyName(const char* input, size_t input_len) : _input(input), _input_len(input_len)
    {
    }

    RtResultVoid parse();

    static RtResultVoid parse_into(const char* input, size_t input_len, metadata::RtMonoAssemblyName* assembly_name_info, bool* is_version_defined,
                                   bool* is_token_defined);

    const char* name() const
    {
        return _name;
    }
    size_t name_length() const
    {
        return _name_length;
    }
    const char* culture() const
    {
        return _culture;
    }
    size_t culture_length() const
    {
        return _culture_length;
    }
    bool is_culture_neutral() const
    {
        return _is_culture_neutral;
    }
    const char* public_key_token() const
    {
        return _public_key_token;
    }
    size_t public_key_token_length() const
    {
        return _public_key_token_length;
    }
    uint16_t version_major() const
    {
        return _version_major;
    }
    uint16_t version_minor() const
    {
        return _version_minor;
    }
    uint16_t version_build() const
    {
        return _version_build;
    }
    uint16_t version_revision() const
    {
        return _version_revision;
    }
    bool is_version_defined() const
    {
        return _is_version_defined;
    }
    bool is_public_key_token_defined() const
    {
        return _is_public_key_token_defined;
    }

    RtResultVoid write_to(metadata::RtMonoAssemblyName* assembly_name_info) const;

  private:
    static const char* trim_spaces(const char* start, const char* end);
    static const char* trim_spaces_end(const char* start, const char* end);
    static bool case_insensitive_starts_with(const char* str, size_t str_len, const char* prefix);
    static RtResultVoid parse_version(const char* start, const char* end, uint16_t* major, uint16_t* minor, uint16_t* build, uint16_t* revision);

    const char* _input;
    size_t _input_len;

    const char* _name = nullptr;
    size_t _name_length = 0;
    const char* _culture = nullptr;
    size_t _culture_length = 0;
    bool _is_culture_neutral = false;
    const char* _public_key_token = nullptr;
    size_t _public_key_token_length = 0;
    uint16_t _version_major = 0;
    uint16_t _version_minor = 0;
    uint16_t _version_build = 0;
    uint16_t _version_revision = 0;
    bool _is_version_defined = false;
    bool _is_public_key_token_defined = false;
};

class Type
{
  public:
    static RtResult<bool> is_value_type(const metadata::RtTypeSig* typeSig);
    static RtResult<bool> is_reference_type(const metadata::RtTypeSig* typeSig);
    static RtResult<size_t> get_size_of_type(const metadata::RtTypeSig* typeSig);
    static bool is_generic_param(const metadata::RtTypeSig* typeSig);
    static bool contains_generic_param(const metadata::RtTypeSig* typeSig);
    static bool contains_not_instantiated_generic_param_in_generic_inst(const metadata::RtGenericInst* genericInst);
    static RtResult<const metadata::RtTypeSig*> resolve_assembly_qualified_name(metadata::RtModuleDef* default_mod, const char* type_full_name, size_t name_len,
                                                                                bool ignore_case);
    static RtResult<RtString*> get_full_name(const metadata::RtTypeSig* typeSig, bool full_name, bool assembly_qualified);
    static RtResultVoid append_type_full_name(utils::Utf8StringBuilder& sb, const metadata::RtTypeSig* typeSig, TypeNameFormat format, bool nested);
    static void append_assembly_name(utils::Utf8StringBuilder& sb, const metadata::RtAssemblyName& assemblyName);
    static RtResult<metadata::RtClass*> get_declaring_type(const metadata::RtTypeSig* typeSig);
    static RtResult<const metadata::RtMethodInfo*> get_declaring_method_of_mvar(const metadata::RtTypeSig* typeSig);
    static RtResultVoid parse_assembly_name(const char* input, size_t input_len, metadata::RtMonoAssemblyName* assembly_name_info, bool* is_version_defined,
                                            bool* is_token_defined);
    static RtResult<const metadata::RtTypeSig*> parse_assembly_qualified_type(metadata::RtModuleDef* default_mod, const char* assembly_qualified_type_name,
                                                                              size_t name_len, bool ignore_case);
};
} // namespace vm
} // namespace leanclr

#include "type.h"

#include <cctype>
#include <cstring>

#include "class.h"
#include "generic_class.h"
#include "rt_string.h"
#include "method.h"
#include "assembly.h"
#include "metadata/rt_metadata.h"
#include "metadata/module_def.h"
#include "metadata/metadata_cache.h"
#include "metadata/metadata_name.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace vm
{
RtResult<bool> Type::is_value_type(const metadata::RtTypeSig* typeSig)
{
    if (typeSig->by_ref)
    {
        RET_OK(false);
    }
    switch (typeSig->ele_type)
    {
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::Char:
    case metadata::RtElementType::I1:
    case metadata::RtElementType::U1:
    case metadata::RtElementType::I2:
    case metadata::RtElementType::U2:
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    case metadata::RtElementType::R4:
    case metadata::RtElementType::R8:
    case metadata::RtElementType::I:
    case metadata::RtElementType::U:
    case metadata::RtElementType::ValueType:
    case metadata::RtElementType::TypedByRef:
        RET_OK(true);
    case metadata::RtElementType::GenericInst:
    {
        uint32_t typeGid = typeSig->data.generic_class->base_type_def_gid;
        uint32_t modId = metadata::RtMetadata::decode_module_id_from_gid(typeGid);
        metadata::RtModuleDef* mod = metadata::RtModuleDef::get_module_by_id(modId);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, baseTypeSig,
                                                mod->get_type_def_by_val_typesig(metadata::RtMetadata::decode_rid_from_gid(typeGid)));
        return is_value_type(baseTypeSig);
    }
    default:
        RET_OK(false);
    }
}

RtResult<size_t> Type::get_size_of_type(const metadata::RtTypeSig* typeSig)
{
    if (typeSig->by_ref)
    {
        RET_OK(PTR_SIZE);
    }
    switch (typeSig->ele_type)
    {
    case metadata::RtElementType::Void:
        RET_OK(0);
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::I1:
    case metadata::RtElementType::U1:
        RET_OK(1);
    case metadata::RtElementType::Char:
    case metadata::RtElementType::I2:
    case metadata::RtElementType::U2:
        RET_OK(2);
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    case metadata::RtElementType::R4:
        RET_OK(4);
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    case metadata::RtElementType::R8:
        RET_OK(8);
    case metadata::RtElementType::I:
    case metadata::RtElementType::U:
    case metadata::RtElementType::Ptr:
    case metadata::RtElementType::FnPtr:
    case metadata::RtElementType::Object:
    case metadata::RtElementType::String:
    case metadata::RtElementType::Class:
    case metadata::RtElementType::SZArray:
    case metadata::RtElementType::Array:
        RET_OK(PTR_SIZE);
    case metadata::RtElementType::TypedByRef:
        RET_OK(RT_TYPED_REFERENCE_SIZE);
    case metadata::RtElementType::ValueType:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, cls, Class::get_class_from_typesig(typeSig));
        RET_ERR_ON_FAIL(Class::initialize_fields(cls));
        RET_OK(Class::get_instance_size_without_object_header(cls));
    }
    case metadata::RtElementType::GenericInst:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, baseClass, GenericClass::get_base_class(typeSig->data.generic_class));
        if (Class::is_reference_type(baseClass))
        {
            RET_OK(PTR_SIZE);
        }
        else
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, inflatedClass, Class::get_class_from_typesig(typeSig));
            RET_ERR_ON_FAIL(Class::initialize_fields(inflatedClass));
            RET_OK(Class::get_instance_size_without_object_header(inflatedClass));
        }
    }
    default:
        assert(false && "Unreachable");
        RET_OK(0);
    }
}

bool Type::is_generic_param(const metadata::RtTypeSig* typeSig)
{
    return typeSig->ele_type == metadata::RtElementType::Var || typeSig->ele_type == metadata::RtElementType::MVar;
}

bool Type::contains_generic_param(const metadata::RtTypeSig* typeSig)
{
    switch (typeSig->ele_type)
    {
    case metadata::RtElementType::Var:
    case metadata::RtElementType::MVar:
        return true;
    case metadata::RtElementType::GenericInst:
    {
        const metadata::RtGenericClass* genericClass = typeSig->data.generic_class;
        const metadata::RtGenericInst* genericInst = genericClass->class_inst;
        for (uint8_t i = 0; i < genericInst->generic_arg_count; ++i)
        {
            if (contains_generic_param(genericInst->generic_args[i]))
            {
                return true;
            }
        }
        return false;
    }
    case metadata::RtElementType::Array:
    {
        const metadata::RtArrayType* arrayType = typeSig->data.array_type;
        return contains_generic_param(arrayType->ele_type);
    }
    case metadata::RtElementType::Ptr:
    case metadata::RtElementType::SZArray:
        return contains_generic_param(typeSig->data.element_type);
    default:
        return false;
    }
}

bool Type::contains_not_instantiated_generic_param_in_generic_inst(const metadata::RtGenericInst* genericInst)
{
    for (uint8_t i = 0; i < genericInst->generic_arg_count; ++i)
    {
        const metadata::RtTypeSig* argTypeSig = genericInst->generic_args[i];
        if (contains_generic_param(argTypeSig))
        {
            return true;
        }
    }
    return false;
}

void AssemblyQualifiedNames::parse()
{
    trim_whitespaces();

    std::string_view type_name_view = parse_after(',');
    type_full_name = utils::StringUtil::trim(type_name_view);
    while (_pos < _len)
    {
        trim_whitespaces();
        std::string_view part = parse_after(',');
        size_t equal_pos = part.find('=');
        if (equal_pos != std::string_view::npos)
        {
            std::string_view key = utils::StringUtil::trim(part.substr(0, equal_pos));
            std::string_view value = utils::StringUtil::trim(part.substr(equal_pos + 1));
            if (key == "Version")
            {
                version = value;
            }
            else if (key == "Culture")
            {
                culture = value;
            }
            else if (key == "PublicKeyToken")
            {
                public_key_token = value;
            }
        }
        else
        {
            assembly_name = utils::StringUtil::trim(part);
        }
    }
}

void AssemblyQualifiedNames::trim_whitespaces()
{
    while (_pos < _len && std::isspace(_str[_pos]))
    {
        _pos++;
    }
}

std::string_view AssemblyQualifiedNames::parse_after(char delimiter)
{
    size_t cur = _pos;
    while (_pos < _len && _str[_pos] != delimiter)
    {
        _pos++;
    }
    if (_pos < _len)
    {
        ++_pos;
        return std::string_view(_str + cur, _pos - 1 - cur);
    }
    else
    {
        return std::string_view(_str + cur, _len - cur);
    }
}

RtResult<const metadata::RtTypeSig*> Type::resolve_assembly_qualified_name(metadata::RtModuleDef* mod, const char* type_full_name, size_t name_len,
                                                                           bool ignore_case)
{
    if (name_len >= 2 && type_full_name[name_len - 2] == '[' && type_full_name[name_len - 1] == ']')
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, elementTypeSig,
                                                resolve_assembly_qualified_name(mod, type_full_name, name_len - 2, ignore_case));
        if (!elementTypeSig)
        {
            RET_OK((const metadata::RtTypeSig*)nullptr);
        }
        return metadata::MetadataCache::get_pooled_szarray_typesig_by_element_typesig(elementTypeSig, false);
    }
    DUP_STR_TO_LOCAL_TEMP_ZERO_END_STR(temp_zero_end_full_name, type_full_name, name_len);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, mod->get_class_by_nested_full_name(temp_zero_end_full_name, ignore_case, false));
    if (klass)
    {
        RET_OK(klass->by_val);
    }
    else
    {
        RET_OK((const metadata::RtTypeSig*)nullptr);
    }
}

static void append_public_key_token(utils::StringBuilder& sb, const uint8_t* token, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        sb.append_hex(token[i]);
    }
}

// Append assembly display name (Name, Version, Culture, PublicKeyToken)
static void append_assembly_name(utils::StringBuilder& sb, const metadata::RtAssemblyName& aname)
{
    sb.append_cstr(aname.name ? aname.name : "");
    sb.append_cstr(", Version=");
    sb.append_u16(aname.version_major);
    sb.append_char('.');
    sb.append_u16(aname.version_minor);
    sb.append_char('.');
    sb.append_u16(aname.version_build);
    sb.append_char('.');
    sb.append_u16(aname.version_revision);

    sb.append_cstr(", Culture=");
    if (aname.culture == nullptr || aname.culture[0] == '\0')
    {
        sb.append_cstr("neutral");
    }
    else
    {
        sb.append_cstr(aname.culture);
    }

    sb.append_cstr(", PublicKeyToken=");
    if (aname.public_key_token[0] != 0)
    {
        append_public_key_token(sb, aname.public_key_token, sizeof(aname.public_key_token));
    }
    else
    {
        sb.append_cstr("null");
    }
}

RtResultVoid Type::append_type_full_name(utils::StringBuilder& sb, const metadata::RtTypeSig* typeSig, TypeNameFormat format, bool nested)
{
    switch (typeSig->ele_type)
    {
    case metadata::RtElementType::Array:
    {
        const metadata::RtArrayType* arrayType = typeSig->data.array_type;
        const metadata::RtTypeSig* elementType = arrayType->ele_type;
        TypeNameFormat nextFormat = format == TypeNameFormat::AssemblyQualified ? TypeNameFormat::FullName : format;

        RET_ERR_ON_FAIL(append_type_full_name(sb, elementType, nextFormat, false));

        sb.append_char('[');
        if (arrayType->rank > 1)
        {
            sb.append_chars(',', arrayType->rank - 1);
        }
        else
        {
            sb.append_char('*');
        }
        sb.append_char(']');

        if (typeSig->by_ref)
        {
            sb.append_char('&');
        }

        if (format == TypeNameFormat::AssemblyQualified)
        {
            sb.append_cstr(", ");
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, elementKlass, Class::get_class_from_typesig(elementType));
            append_assembly_name(sb, elementKlass->image->get_assembly_name());
        }
        break;
    }
    case metadata::RtElementType::SZArray:
    {
        const metadata::RtTypeSig* elementType = typeSig->data.element_type;
        TypeNameFormat nextFormat = format == TypeNameFormat::AssemblyQualified ? TypeNameFormat::FullName : format;

        RET_ERR_ON_FAIL(append_type_full_name(sb, elementType, nextFormat, false));
        sb.append_cstr("[]");

        if (typeSig->by_ref)
        {
            sb.append_char('&');
        }

        if (format == TypeNameFormat::AssemblyQualified)
        {
            sb.append_cstr(", ");
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, elementKlass, Class::get_class_from_typesig(elementType));
            append_assembly_name(sb, elementKlass->image->get_assembly_name());
        }
        break;
    }
    case metadata::RtElementType::Ptr:
    {
        const metadata::RtTypeSig* elementType = typeSig->data.element_type;
        TypeNameFormat nextFormat = format == TypeNameFormat::AssemblyQualified ? TypeNameFormat::FullName : format;

        RET_ERR_ON_FAIL(append_type_full_name(sb, elementType, nextFormat, false));
        sb.append_char('*');

        if (typeSig->by_ref)
        {
            sb.append_char('&');
        }

        if (format == TypeNameFormat::AssemblyQualified)
        {
            sb.append_cstr(", ");
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, elementKlass, Class::get_class_from_typesig(elementType));
            append_assembly_name(sb, elementKlass->image->get_assembly_name());
        }
        break;
    }
    case metadata::RtElementType::FnPtr:
    {
        sb.append_cstr("delegate* unmanaged");
        const metadata::RtMethodSig* method_sig = typeSig->data.method_sig;
        assert(method_sig->generic_param_count == 0 && "Generic parameters are not supported for function pointers");
        sb.append_char('[');
        sb.append_cstr(metadata::MetadataName::get_call_convention_name((metadata::RtSigType)method_sig->flags));
        sb.append_char(']');
        sb.append_char('<');
        for (size_t i = 0; i < method_sig->params.size(); ++i)
        {
            RET_ERR_ON_FAIL(append_type_full_name(sb, method_sig->params[i], format, false));
            sb.append_char(',');
        }
        RET_ERR_ON_FAIL(append_type_full_name(sb, method_sig->return_type, format, false));
        sb.append_char('>');
        break;
    }
    case metadata::RtElementType::Var:
    case metadata::RtElementType::MVar:
    {
        const metadata::RtGenericParam* genericParam = typeSig->data.generic_param;
        sb.append_cstr(genericParam->name);

        if (typeSig->by_ref)
        {
            sb.append_char('&');
        }
        break;
    }
    default:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(typeSig));

        metadata::RtClass* declaringClass = Class::get_enclosing_class(klass);
        if (declaringClass)
        {
            RET_ERR_ON_FAIL(append_type_full_name(sb, Class::get_by_val_type_sig(declaringClass), format, true));
            sb.append_char(format == TypeNameFormat::IL ? '.' : '+');
        }
        else if (klass->namespaze[0] != '\0')
        {
            sb.append_cstr(klass->namespaze);
            sb.append_char('.');
        }

        const char* name = klass->name;
        if (format == TypeNameFormat::IL)
        {
            const char* tickPos = std::strchr(name, '`');
            if (tickPos)
            {
                sb.append_cstr(reinterpret_cast<const uint8_t*>(name), static_cast<size_t>(tickPos - name));
            }
            else
            {
                sb.append_cstr(name);
            }
        }
        else
        {
            sb.append_cstr(name);
        }

        if (!nested)
        {
            if (Class::is_generic_inst(klass))
            {
                const metadata::RtGenericClass* genericClass = typeSig->data.generic_class;
                const metadata::RtGenericInst* inst = genericClass->class_inst;
                TypeNameFormat nestedFormat = format == TypeNameFormat::AssemblyQualified ? TypeNameFormat::FullName : format;

                sb.append_char(format == TypeNameFormat::IL ? '<' : '[');
                for (uint8_t i = 0; i < inst->generic_arg_count; ++i)
                {
                    if (i > 0)
                    {
                        sb.append_char(',');
                    }

                    const metadata::RtTypeSig* argType = inst->generic_args[i];
                    metadata::RtElementType argEleType = argType->ele_type;

                    if (nestedFormat == TypeNameFormat::AssemblyQualified && argEleType != metadata::RtElementType::Var &&
                        argEleType != metadata::RtElementType::MVar)
                    {
                        sb.append_char('[');
                        RET_ERR_ON_FAIL(append_type_full_name(sb, argType, nestedFormat, false));
                        sb.append_char(']');
                    }
                    else
                    {
                        RET_ERR_ON_FAIL(append_type_full_name(sb, argType, nestedFormat, false));
                    }
                }
                sb.append_char(format == TypeNameFormat::IL ? '>' : ']');
            }
            else if (klass->generic_container)
            {
                const metadata::RtGenericContainer* gc = klass->generic_container;
                sb.append_char(format == TypeNameFormat::IL ? '<' : '[');
                for (uint8_t i = 0; i < gc->generic_param_count; ++i)
                {
                    if (i > 0)
                    {
                        sb.append_char(',');
                    }

                    const metadata::RtGenericParam& gp = gc->generic_params[i];
                    sb.append_cstr(gp.name);
                }
                sb.append_char(format == TypeNameFormat::IL ? '>' : ']');
            }

            if (typeSig->by_ref)
            {
                sb.append_char('&');
            }

            if (format == TypeNameFormat::AssemblyQualified && typeSig->ele_type != metadata::RtElementType::Var &&
                typeSig->ele_type != metadata::RtElementType::MVar)
            {
                sb.append_cstr(", ");
                append_assembly_name(sb, klass->image->get_assembly_name());
            }
        }
        break;
    }
    }
    RET_VOID_OK();
}

RtResult<RtString*> Type::get_full_name(const metadata::RtTypeSig* typeSig, bool full_name, bool assembly_qualified)
{
    utils::StringBuilder sb;
    TypeNameFormat format = full_name ? (assembly_qualified ? TypeNameFormat::AssemblyQualified : TypeNameFormat::FullName) : TypeNameFormat::Reflection;
    RET_ERR_ON_FAIL(append_type_full_name(sb, typeSig, format, false));
    RtString* name = String::create_string_from_utf8chars(sb.as_cstr(), static_cast<int32_t>(sb.length()));
    RET_OK(name);
}

void Type::append_assembly_name(utils::StringBuilder& sb, const metadata::RtAssemblyName& assemblyName)
{
    sb.append_cstr(assemblyName.name);
    sb.append_cstr(", Version=");
    sb.append_u16(assemblyName.version_major);
    sb.append_char('.');
    sb.append_u16(assemblyName.version_minor);
    sb.append_char('.');
    sb.append_u16(assemblyName.version_build);
    sb.append_char('.');
    sb.append_u16(assemblyName.version_revision);
    sb.append_cstr(", Culture=");
    if (assemblyName.culture == nullptr || assemblyName.culture[0] == '\0')
    {
        sb.append_cstr("neutral");
    }
    else
    {
        sb.append_cstr(assemblyName.culture);
    }
    sb.append_cstr(", PublicKeyToken=");
    if (assemblyName.public_key_token[0] != 0)
    {
        append_public_key_token(sb, assemblyName.public_key_token, sizeof(assemblyName.public_key_token));
    }
    else
    {
        sb.append_cstr("null");
    }
    sb.sure_null_terminator_but_not_append();
}

RtResult<metadata::RtClass*> Type::get_declaring_type(const metadata::RtTypeSig* typeSig)
{
    if (typeSig->is_by_ref())
    {
        RET_OK(nullptr);
    }
    if (is_generic_param(typeSig))
    {
        const metadata::RtGenericParam* genericParam = typeSig->data.generic_param;
        const metadata::RtGenericContainer* owner = genericParam->owner;
        if (!owner->is_method)
        {
            return Class::get_class_by_type_def_gid(owner->owner_gid);
        }
        else
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method, Method::get_method_by_method_def_gid(owner->owner_gid));
            RET_OK(const_cast<metadata::RtClass*>(method->parent));
        }
    }
    else
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(typeSig));
        const metadata::RtClass* declaringClass = Class::get_enclosing_class(klass);
        RET_OK(const_cast<metadata::RtClass*>(declaringClass));
    }
}

RtResult<const metadata::RtMethodInfo*> Type::get_declaring_method_of_mvar(const metadata::RtTypeSig* typeSig)
{
    if (!typeSig->is_by_ref() && typeSig->ele_type == metadata::RtElementType::MVar)
    {
        const metadata::RtGenericParam* genericParam = typeSig->data.generic_param;
        const metadata::RtGenericContainer* owner = genericParam->owner;
        if (owner->is_method)
        {
            return vm::Method::get_method_by_method_def_gid(owner->owner_gid);
        }
    }
    RET_OK(nullptr);
}

// Helper functions for parse_assembly_name
static const char* trim_spaces(const char* start, const char* end)
{
    while (start < end && std::isspace(*start))
        start++;
    return start;
}

static const char* trim_spaces_end(const char* start, const char* end)
{
    while (end > start && std::isspace(*(end - 1)))
        end--;
    return end;
}

static bool case_insensitive_compare(const char* str, size_t str_len, const char* prefix)
{
    size_t prefix_len = std::strlen(prefix);
    if (str_len < prefix_len)
        return false;
    for (size_t i = 0; i < prefix_len; i++)
    {
        if (std::tolower(str[i]) != std::tolower(static_cast<uint8_t>(prefix[i])))
            return false;
    }
    return true;
}

static RtResultVoid parse_assembly_version(const char* start, const char* end, metadata::RtMonoAssemblyName* assembly_name_info)
{
    // Parse version string like "1.0.0.0"
    const char* p = start;
    uint16_t parts[4] = {0, 0, 0, 0};
    int part_idx = 0;

    while (p < end && part_idx < 4)
    {
        if (*p >= '0' && *p <= '9')
        {
            parts[part_idx] = static_cast<uint16_t>(static_cast<unsigned int>(parts[part_idx]) * 10u + static_cast<unsigned int>(*p - '0'));
            p++;
        }
        else if (*p == '.')
        {
            part_idx++;
            p++;
        }
        else
        {
            RET_ERR(RtErr::Argument);
        }
    }

    assembly_name_info->version_major = parts[0];
    assembly_name_info->version_minor = parts[1];
    assembly_name_info->version_build = parts[2];
    assembly_name_info->version_revision = parts[3];

    RET_VOID_OK();
}

RtResultVoid Type::parse_assembly_name(const char* input, size_t input_len, metadata::RtMonoAssemblyName* assembly_name_info, bool* is_version_defined,
                                       bool* is_token_defined)
{
    *is_version_defined = false;
    *is_token_defined = false;

    const char* p = input;
    const char* end = input + input_len;

    // Find first comma to get assembly name
    const char* comma = p;
    while (comma < end && *comma != ',')
        comma++;

    // Extract assembly name
    const char* name_start = trim_spaces(p, comma);
    const char* name_end = trim_spaces_end(name_start, comma);
    size_t name_len = static_cast<size_t>(name_end - name_start);

    if (name_len > 0)
    {
        char* name_copy = static_cast<char*>(alloc::GeneralAllocation::malloc(name_len + 1));
        std::memcpy(name_copy, name_start, name_len);
        name_copy[name_len] = '\0';
        assembly_name_info->name = name_copy;
    }

    p = comma;
    if (p >= end)
    {
        RET_VOID_OK();
    }

    // Parse additional parts
    while (p < end)
    {
        if (*p == ',')
            p++;

        const char* seg_start = trim_spaces(p, end);

        // Find next comma
        const char* next_comma = seg_start;
        while (next_comma < end && *next_comma != ',')
            next_comma++;

        const char* seg_end = trim_spaces_end(seg_start, next_comma);
        size_t seg_len = static_cast<size_t>(seg_end - seg_start);

        if (seg_len > 0)
        {
            if (case_insensitive_compare(seg_start, seg_len, "Version="))
            {
                *is_version_defined = true;
                const char* value_start = seg_start + 8; // strlen("Version=")
                value_start = trim_spaces(value_start, seg_end);
                RET_ERR_ON_FAIL(parse_assembly_version(value_start, seg_end, assembly_name_info));
            }
            else if (case_insensitive_compare(seg_start, seg_len, "Culture="))
            {
                const char* value_start = seg_start + 8; // strlen("Culture=")
                value_start = trim_spaces(value_start, seg_end);
                size_t value_len = static_cast<size_t>(seg_end - value_start);

                if (value_len == 7 && std::memcmp(value_start, "neutral", 7) == 0)
                {
                    assembly_name_info->culture = nullptr;
                }
                else
                {
                    char* culture_copy = static_cast<char*>(alloc::GeneralAllocation::malloc(value_len + 1));
                    std::memcpy(culture_copy, value_start, value_len);
                    culture_copy[value_len] = '\0';
                    assembly_name_info->culture = culture_copy;
                }
            }
            else if (case_insensitive_compare(seg_start, seg_len, "PublicKeyToken="))
            {
                *is_token_defined = true;
                const char* value_start = seg_start + 15; // strlen("PublicKeyToken=")
                value_start = trim_spaces(value_start, seg_end);
                size_t value_len = static_cast<size_t>(seg_end - value_start);

                if (value_len != RT_PUBLIC_KEY_TOKEN_HEX_STRING_WITH_NULL_TERMINATOR_LENGTH - 1)
                {
                    RET_ERR(RtErr::Argument);
                }

                std::memcpy(assembly_name_info->public_key_token, value_start, RT_PUBLIC_KEY_TOKEN_HEX_STRING_WITH_NULL_TERMINATOR_LENGTH - 1);
                assembly_name_info->public_key_token[RT_PUBLIC_KEY_TOKEN_HEX_STRING_WITH_NULL_TERMINATOR_LENGTH - 1] = '\0';
            }
        }

        p = next_comma;
    }

    RET_VOID_OK();
}

RtResult<const metadata::RtTypeSig*> Type::parse_assembly_qualified_type(metadata::RtModuleDef* default_mod, const char* assembly_qualified_type_name,
                                                                         size_t name_len, bool ignore_case)
{
    AssemblyQualifiedNames qn(assembly_qualified_type_name, name_len);
    qn.parse();

    assert(default_mod);
    metadata::RtModuleDef* search_ass_list[2];
    if (!qn.assembly_name.empty())
    {
        metadata::RtAssembly* ass = Assembly::find_by_name(qn.assembly_name.c_str());
        if (!ass)
        {
            RET_ERR(RtErr::TypeLoad);
        }
        search_ass_list[0] = ass->mod;
        search_ass_list[1] = nullptr;
    }
    else
    {
        search_ass_list[0] = default_mod;
        metadata::RtModuleDef* corlib = Assembly::get_corlib()->mod;
        search_ass_list[1] = default_mod != corlib ? corlib : nullptr;
    }
    for (metadata::RtModuleDef* mod : search_ass_list)
    {
        if (!mod)
        {
            break;
        }
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, typeSig,
                                                Type::resolve_assembly_qualified_name(mod, qn.type_full_name.c_str(), qn.type_full_name.size(), false));
        if (!typeSig)
        {
            continue;
        }
        return typeSig;
    }
    // search all assemblies
    auto modules_span = metadata::RtModuleDef::get_registered_modules();
    utils::Vector<metadata::RtModuleDef*> registered_modules = utils::Vector<metadata::RtModuleDef*>(modules_span.begin(), modules_span.end());
    for (metadata::RtModuleDef* mod : registered_modules)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, typeSig,
                                                Type::resolve_assembly_qualified_name(mod, qn.type_full_name.c_str(), qn.type_full_name.size(), false));
        if (!typeSig)
        {
            continue;
        }
        return typeSig;
    }
    RET_ERR(RtErr::TypeLoad);
}
} // namespace vm
} // namespace leanclr

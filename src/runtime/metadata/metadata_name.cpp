#include "metadata_name.h"
#include "utils/string_builder.h"
#include "vm/class.h"
#include "vm/method.h"
#include "module_def.h"

namespace leanclr
{
namespace metadata
{

static char get_nested_type_separator(TypeNameFormat format)
{
    switch (format)
    {
    case TypeNameFormat::IL:
        return '.';
    case TypeNameFormat::InternalName:
    case TypeNameFormat::DnlibToString:
        return '/';
    default:
        return '+';
    }
}

static char get_generic_param_start_separator(TypeNameFormat format)
{
    switch (format)
    {
    case TypeNameFormat::IL:
    case TypeNameFormat::InternalName:
    case TypeNameFormat::DnlibToString:
        return '<';
    default:
        return '[';
    }
}

static char get_generic_param_end_separator(TypeNameFormat format)
{
    switch (format)
    {
    case TypeNameFormat::IL:
    case TypeNameFormat::InternalName:
    case TypeNameFormat::DnlibToString:
        return '>';
    default:
        return ']';
    }
}

// Helper to append class full name recursively (namespace + name, handling nested types)
RtResultVoid MetadataName::append_klass_full_name_without_generic_params(utils::Utf8StringBuilder& sb, const RtClass* klass, TypeNameFormat format)
{
    auto optEnclosingTypeDefRid = klass->image->get_enclosing_type_def_rid(klass->token);
    if (optEnclosingTypeDefRid)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, enclosing_klass, klass->image->get_class_by_type_def_rid(optEnclosingTypeDefRid.value()));
        RET_ERR_ON_FAIL(append_klass_full_name_without_generic_params(sb, enclosing_klass, format));
        sb.append_char(get_nested_type_separator(format));
    }
    else
    {
        // Not a nested type - append namespace and name
        if (klass->namespaze && klass->namespaze[0] != 0)
        {
            sb.append_cstr(klass->namespaze);
            sb.append_char('.');
        }
    }
    if (format == TypeNameFormat::IL)
    {
        const char* il_name_end = std::strchr(klass->name, '`');
        if (il_name_end)
        {
            sb.append_cstr(klass->name, static_cast<size_t>(il_name_end - klass->name));
        }
        else
        {
            sb.append_cstr(klass->name);
        }
    }
    else
    {
        sb.append_cstr(klass->name);
    }
    RET_VOID_OK();
}

static void append_public_key_token(utils::Utf8StringBuilder& sb, const uint8_t* token, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        sb.append_hex(token[i]);
    }
}

void MetadataName::append_assembly_name(utils::Utf8StringBuilder& sb, const metadata::RtAssemblyName& assemblyName)
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

RtResultVoid MetadataName::append_type_full_name(utils::Utf8StringBuilder& sb, const metadata::RtTypeSig* typeSig, TypeNameFormat cur_format, bool nested)
{
    TypeNameFormat nextFormat = cur_format == TypeNameFormat::AssemblyQualified ? TypeNameFormat::FullName : cur_format;
    switch (typeSig->ele_type)
    {
    case metadata::RtElementType::Array:
    {
        const metadata::RtArrayType* arrayType = typeSig->data.array_type;
        const metadata::RtTypeSig* elementType = arrayType->ele_type;

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

        if (cur_format == TypeNameFormat::AssemblyQualified)
        {
            sb.append_cstr(", ");
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, elementKlass, vm::Class::get_class_from_typesig(elementType));
            append_assembly_name(sb, elementKlass->image->get_assembly_name());
        }
        break;
    }
    case metadata::RtElementType::SZArray:
    {
        const metadata::RtTypeSig* elementType = typeSig->data.element_type;

        RET_ERR_ON_FAIL(append_type_full_name(sb, elementType, cur_format, false));
        sb.append_cstr("[]");

        if (typeSig->by_ref)
        {
            sb.append_char('&');
        }

        if (cur_format == TypeNameFormat::AssemblyQualified)
        {
            sb.append_cstr(", ");
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, elementKlass, vm::Class::get_class_from_typesig(elementType));
            append_assembly_name(sb, elementKlass->image->get_assembly_name());
        }
        break;
    }
    case metadata::RtElementType::Ptr:
    {
        const metadata::RtTypeSig* elementType = typeSig->data.element_type;

        RET_ERR_ON_FAIL(append_type_full_name(sb, elementType, cur_format, false));
        sb.append_char('*');

        if (typeSig->by_ref)
        {
            sb.append_char('&');
        }

        if (cur_format == TypeNameFormat::AssemblyQualified)
        {
            sb.append_cstr(", ");
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, elementKlass, vm::Class::get_class_from_typesig(elementType));
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
        sb.append_char(get_generic_param_start_separator(cur_format));
        for (size_t i = 0; i < method_sig->params.size(); ++i)
        {
            RET_ERR_ON_FAIL(append_type_full_name(sb, method_sig->params[i], nextFormat, false));
            sb.append_char(',');
        }
        RET_ERR_ON_FAIL(append_type_full_name(sb, method_sig->return_type, nextFormat, false));
        sb.append_char(get_generic_param_end_separator(cur_format));
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
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(typeSig));
        RET_ERR_ON_FAIL(append_klass_full_name_without_generic_params(sb, klass, cur_format));
        if (!nested)
        {
            if (vm::Class::is_generic_inst(klass))
            {
                const metadata::RtGenericClass* genericClass = typeSig->data.generic_class;
                const metadata::RtGenericInst* inst = genericClass->class_inst;

                sb.append_char(get_generic_param_start_separator(cur_format));
                for (uint8_t i = 0; i < inst->generic_arg_count; ++i)
                {
                    if (i > 0)
                    {
                        sb.append_char(',');
                    }

                    const metadata::RtTypeSig* argType = inst->generic_args[i];
                    metadata::RtElementType argEleType = argType->ele_type;

                    if (nextFormat == TypeNameFormat::AssemblyQualified && argEleType != metadata::RtElementType::Var &&
                        argEleType != metadata::RtElementType::MVar)
                    {
                        sb.append_char('[');
                        RET_ERR_ON_FAIL(append_type_full_name(sb, argType, nextFormat, false));
                        sb.append_char(']');
                    }
                    else
                    {
                        RET_ERR_ON_FAIL(append_type_full_name(sb, argType, nextFormat, false));
                    }
                }
                sb.append_char(get_generic_param_end_separator(cur_format));
            }
            else if (klass->generic_container)
            {
                const metadata::RtGenericContainer* gc = klass->generic_container;
                sb.append_char(get_generic_param_start_separator(cur_format));
                for (uint8_t i = 0; i < gc->generic_param_count; ++i)
                {
                    if (i > 0)
                    {
                        sb.append_char(',');
                    }

                    const metadata::RtGenericParam& gp = gc->generic_params[i];
                    sb.append_cstr(gp.name);
                }
                sb.append_char(get_generic_param_end_separator(cur_format));
            }

            if (typeSig->by_ref)
            {
                sb.append_char('&');
            }

            if (cur_format == TypeNameFormat::AssemblyQualified && typeSig->ele_type != metadata::RtElementType::Var &&
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

const char* MetadataName::get_call_convention_name(RtSigType call_conv)
{
    switch ((RtSigType)((uint8_t)call_conv & (uint8_t)RtSigType::TypeMask))
    {
    case RtSigType::C:
        return "Cdecl";
    case RtSigType::StdCall:
        return "StdCall";
    case RtSigType::ThisCall:
        return "ThisCall";
    case RtSigType::FastCall:
        return "FastCall";
    case RtSigType::VarArg:
        return "VarArg";
    default:
        return "Default";
    }
}

RtResultVoid MetadataName::append_method_sig_name(utils::Utf8StringBuilder& sb, const RtMethodSig* method_sig, TypeNameFormat cur_format)
{
    TypeNameFormat nextFormat = cur_format == TypeNameFormat::AssemblyQualified ? TypeNameFormat::FullName : cur_format;
    sb.append_cstr("delegate* unmanaged");
    assert(method_sig->generic_param_count == 0 && "Generic parameters are not supported for function pointers");
    sb.append_char('[');
    sb.append_cstr(get_call_convention_name((RtSigType)method_sig->flags));
    sb.append_char(']');
    sb.append_char(get_generic_param_start_separator(cur_format));
    for (size_t i = 0; i < method_sig->params.size(); ++i)
    {
        RET_ERR_ON_FAIL(append_type_full_name(sb, method_sig->params[i], nextFormat, false));
        sb.append_char(',');
    }
    RET_ERR_ON_FAIL(append_type_full_name(sb, method_sig->return_type, nextFormat, false));
    sb.append_char(get_generic_param_end_separator(cur_format));
    RET_VOID_OK();
}

RtResultVoid MetadataName::append_method_full_name_without_params(utils::Utf8StringBuilder& sb, const RtMethodInfo* method, TypeNameFormat cur_format)
{
    const metadata::RtClass* klass = method->parent;

    // Append class full name
    RET_ERR_ON_FAIL(append_type_full_name(sb, klass->by_val, cur_format, false));

    // Append :: and method name
    sb.append_cstr("::");
    sb.append_cstr(method->name);
    if (cur_format == TypeNameFormat::InternalName)
    {
        uint16_t generic_param_count = vm::Method::get_generic_param_count(method);
        if (generic_param_count > 0)
        {
            sb.append_char(get_generic_param_start_separator(cur_format));
            sb.append_chars(',', generic_param_count - 1);
            sb.append_char(get_generic_param_end_separator(cur_format));
        }
    }
    else
    {
        // Append generic parameters if present
        if (method->generic_container)
        {
            const metadata::RtGenericContainer* gc = method->generic_container;
            sb.append_char(get_generic_param_start_separator(cur_format));
            sb.append_chars(',', gc->generic_param_count - 1);
            sb.append_char(get_generic_param_end_separator(cur_format));
        }
        else if (method->generic_method)
        {
            const metadata::RtGenericInst* method_inst = method->generic_method->generic_context.method_inst;
            if (method_inst)
            {
                TypeNameFormat nextFormat = cur_format == TypeNameFormat::AssemblyQualified ? TypeNameFormat::FullName : cur_format;
                sb.append_char(get_generic_param_start_separator(cur_format));
                for (uint8_t i = 0; i < method_inst->generic_arg_count; ++i)
                {
                    if (i > 0)
                    {
                        sb.append_char(',');
                    }
                    RET_ERR_ON_FAIL(append_type_full_name(sb, method_inst->generic_args[i], nextFormat, false));
                }
                sb.append_char(get_generic_param_end_separator(cur_format));
            }
        }
    }

    sb.sure_null_terminator_but_not_append();
    RET_VOID_OK();
}

RtResultVoid MetadataName::append_method_full_name_with_params(utils::Utf8StringBuilder& sb, const RtMethodInfo* method, TypeNameFormat cur_format)
{
    RET_ERR_ON_FAIL(append_method_full_name_without_params(sb, method, cur_format));
    TypeNameFormat nextFormat = cur_format == TypeNameFormat::AssemblyQualified ? TypeNameFormat::FullName : cur_format;

    // Append parameters
    sb.append_char('(');
    uint16_t param_count = method->parameter_count;
    for (uint16_t i = 0; i < param_count; ++i)
    {
        if (i > 0)
        {
            sb.append_char(',');
        }
        const RtTypeSig* param = method->parameters[i];
        RET_ERR_ON_FAIL(append_type_full_name(sb, param, nextFormat, false));
    }
    sb.append_char(')');
    sb.sure_null_terminator_but_not_append();
    RET_VOID_OK();
}

} // namespace metadata
} // namespace leanclr

#include "metadata_name.h"
#include "utils/string_builder.h"
#include "vm/class.h"
#include "vm/method.h"
#include "module_def.h"

namespace leanclr
{
namespace metadata
{

// Helper to append class full name recursively (namespace + name, handling nested types)
RtResultVoid MetadataName::append_klass_full_name(utils::StringBuilder& sb, const RtClass* klass)
{
    // Check for enclosing type (nested class)
    if (klass->image)
    {
        auto optEnclosingTypeDefRid = klass->image->get_enclosing_type_def_rid(klass->token);
        if (optEnclosingTypeDefRid)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, enclosing_klass, klass->image->get_class_by_type_def_rid(optEnclosingTypeDefRid.value()));
            RET_ERR_ON_FAIL(append_klass_full_name(sb, enclosing_klass));
            sb.append_char('/'); // nested types use '/' separator
            sb.append_cstr(klass->name);
            RET_VOID_OK();
        }
    }

    // Not a nested type - append namespace and name
    if (klass->namespaze && klass->namespaze[0] != 0)
    {
        sb.append_cstr(klass->namespaze);
        sb.append_char('.');
    }
    sb.append_cstr(klass->name);
    RET_VOID_OK();
}

// Helper to append type signature name based on element type
RtResultVoid MetadataName::append_type_sig_name(utils::StringBuilder& sb, const RtTypeSig* type_sig)
{
    if (!type_sig)
    {
        RET_VOID_OK();
    }

    switch (type_sig->ele_type)
    {
    case RtElementType::Void:
        sb.append_cstr("System.Void");
        break;
    case RtElementType::Boolean:
        sb.append_cstr("System.Boolean");
        break;
    case RtElementType::Char:
        sb.append_cstr("System.Char");
        break;
    case RtElementType::I1:
        sb.append_cstr("System.SByte");
        break;
    case RtElementType::U1:
        sb.append_cstr("System.Byte");
        break;
    case RtElementType::I2:
        sb.append_cstr("System.Int16");
        break;
    case RtElementType::U2:
        sb.append_cstr("System.UInt16");
        break;
    case RtElementType::I4:
        sb.append_cstr("System.Int32");
        break;
    case RtElementType::U4:
        sb.append_cstr("System.UInt32");
        break;
    case RtElementType::I8:
        sb.append_cstr("System.Int64");
        break;
    case RtElementType::U8:
        sb.append_cstr("System.UInt64");
        break;
    case RtElementType::R4:
        sb.append_cstr("System.Single");
        break;
    case RtElementType::R8:
        sb.append_cstr("System.Double");
        break;
    case RtElementType::String:
        sb.append_cstr("System.String");
        break;
    case RtElementType::Ptr:
    {
        const RtTypeSig* base_type = type_sig->data.element_type;
        RET_ERR_ON_FAIL(append_type_sig_name(sb, base_type));
        sb.append_char('*');
        break;
    }
    case RtElementType::ByRef:
    {
        const RtTypeSig* base_type = type_sig->data.element_type;
        RET_ERR_ON_FAIL(append_type_sig_name(sb, base_type));
        sb.append_char('&');
        break;
    }
    case RtElementType::ValueType:
    case RtElementType::Class:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, klass, vm::Class::get_class_by_type_def_gid(type_sig->data.type_def_gid));
        RET_ERR_ON_FAIL(append_klass_full_name(sb, klass));
        break;
    }
    case RtElementType::Var:
    case RtElementType::MVar:
    {
        const RtGenericParam* generic_param = type_sig->data.generic_param;
        sb.append_cstr(generic_param->name);
        break;
    }
    case RtElementType::Array:
    {
        const RtArrayType* arr_type = type_sig->data.array_type;
        const RtTypeSig* base_type = arr_type->ele_type;
        RET_ERR_ON_FAIL(append_type_sig_name(sb, base_type));
        sb.append_char('[');
        uint8_t rank = arr_type->rank;
        sb.append_chars(',', rank - 1);
        sb.append_char(']');
        break;
    }
    case RtElementType::GenericInst:
    {
        const RtGenericClass* generic_class = type_sig->data.generic_class;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, generic_base_klass, vm::Class::get_class_by_type_def_gid(generic_class->base_type_def_gid));
        RET_ERR_ON_FAIL(append_klass_full_name(sb, generic_base_klass));
        sb.append_char('<');

        const RtGenericInst* generic_inst = generic_class->class_inst;
        uint16_t generic_arg_count = generic_inst->generic_arg_count;
        const RtTypeSig* const* generic_args = generic_inst->generic_args;
        for (uint16_t i = 0; i < generic_arg_count; ++i)
        {
            if (i > 0)
            {
                sb.append_char(',');
            }
            const RtTypeSig* generic_arg_type = generic_args[i];
            RET_ERR_ON_FAIL(append_type_sig_name(sb, generic_arg_type));
        }
        sb.append_char('>');
        break;
    }
    case RtElementType::TypedByRef:
        sb.append_cstr("System.TypedReference");
        break;
    case RtElementType::I:
        sb.append_cstr("System.IntPtr");
        break;
    case RtElementType::U:
        sb.append_cstr("System.UIntPtr");
        break;
    case RtElementType::FnPtr:
    {
        const RtMethodSig* method_sig = type_sig->data.method_sig;
        RET_ERR_ON_FAIL(append_method_sig_name(sb, method_sig));
        break;
    }
    case RtElementType::Object:
        sb.append_cstr("System.Object");
        break;
    case RtElementType::SZArray:
    {
        const RtTypeSig* base_type = type_sig->data.element_type;
        RET_ERR_ON_FAIL(append_type_sig_name(sb, base_type));
        sb.append_cstr("[]");
        break;
    }
    case RtElementType::Sentinel:
        RETURN_NOT_IMPLEMENTED_ERROR();
    default:
        // Unreachable
        break;
    }

    // Handle by_ref flag after type (similar to Rust's trailing &)
    if (type_sig->by_ref)
    {
        sb.append_cstr("&");
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

RtResultVoid MetadataName::append_method_sig_name(utils::StringBuilder& sb, const RtMethodSig* method_sig)
{
    sb.append_cstr("delegate* unmanaged");
    assert(method_sig->generic_param_count == 0 && "Generic parameters are not supported for function pointers");
    sb.append_char('[');
    sb.append_cstr(get_call_convention_name((RtSigType)method_sig->flags));
    sb.append_char(']');
    sb.append_char('<');
    for (size_t i = 0; i < method_sig->params.size(); ++i)
    {
        RET_ERR_ON_FAIL(append_type_sig_name(sb, method_sig->params[i]));
        sb.append_char(',');
    }
    RET_ERR_ON_FAIL(append_type_sig_name(sb, method_sig->return_type));
    sb.append_char('>');
    RET_VOID_OK();
}

RtResultVoid MetadataName::append_method_full_name_without_params(utils::StringBuilder& sb, const RtMethodInfo* method)
{
    const metadata::RtClass* klass = method->parent;

    // Append class full name
    RET_ERR_ON_FAIL(append_klass_full_name(sb, klass));

    // Append :: and method name
    sb.append_cstr("::");
    sb.append_cstr(method->name);

    // Append generic parameters if present
    uint16_t generic_param_count = vm::Method::get_generic_param_count(method);
    if (generic_param_count > 0)
    {
        sb.append_char('<');
        sb.append_chars(',', generic_param_count - 1);
        sb.append_char('>');
    }
    sb.sure_null_terminator_but_not_append();
    RET_VOID_OK();
}

RtResultVoid MetadataName::append_method_full_name_with_params(utils::StringBuilder& sb, const RtMethodInfo* method)
{
    RET_ERR_ON_FAIL(append_method_full_name_without_params(sb, method));

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
        RET_ERR_ON_FAIL(append_type_sig_name(sb, param));
    }
    sb.append_char(')');
    sb.sure_null_terminator_but_not_append();
    RET_VOID_OK();
}

// RtResult<const char*> MetadataName::build_class_full_name(const RtClass* klass)
// {
//     utils::StringBuilder sb;
//     RET_ERR_ON_FAIL(append_klass_full_name(sb, const_cast<RtClass*>(klass)));
//     RET_OK(sb.dup_to_zero_end_cstr());
// }

// RtResult<const char*> MetadataName::build_method_full_name_with_params(const RtMethodInfo* method)
// {
//     utils::StringBuilder sb;
//     RET_ERR_ON_FAIL(append_method_full_name_with_params(sb, const_cast<RtMethodInfo*>(method)));
//     RET_OK(sb.dup_to_zero_end_cstr());
// }

// RtResult<const char*> MetadataName::build_method_full_name_without_params(const RtMethodInfo* method)
// {
//     utils::StringBuilder sb;
//     RET_ERR_ON_FAIL(append_method_full_name_without_params(sb, const_cast<RtMethodInfo*>(method)));
//     RET_OK(sb.dup_to_zero_end_cstr());
// }

} // namespace metadata
} // namespace leanclr

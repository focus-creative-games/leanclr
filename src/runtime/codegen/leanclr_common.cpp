#include "leanclr_common.h"
#include "vm/object.h"
#include "metadata/module_def.h"
#include "utils/string_builder.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace codegen
{

vm::RtString* resolve_string_literal(metadata::RtModuleDef* mod, uint32_t token)
{
    assert((int32_t)metadata::RtToken::decode_table_type(token) == metadata::USER_STRING_HEAP_FAKE_TABLE_TYPE && "Token is not a string literal");
    uint32_t string_index = metadata::RtToken::decode_rid(token);
    auto ret = mod->get_user_string(string_index);
    return ret.is_ok() ? ret.unwrap() : nullptr;
}

void* resolve_metadata_token(metadata::RtModuleDef* mod, uint32_t token, const metadata::RtMethodInfo* generic_method_info)
{
    metadata::RtToken t = metadata::RtToken::decode(token);
    metadata::RtGenericContainerContext gcc = {generic_method_info ? generic_method_info->parent->generic_container : nullptr,
                                               generic_method_info ? generic_method_info->generic_container : nullptr};
    const metadata::RtGenericContext* generic_context =
        generic_method_info && generic_method_info->generic_method ? &generic_method_info->generic_method->generic_context : nullptr;
    switch (t.table_type)
    {
    case metadata::TableType::TypeDef:
    case metadata::TableType::TypeRef:
    case metadata::TableType::TypeSpec:
    {
        auto ret = mod->get_typesig_by_type_def_ref_spec_token(t, gcc, generic_context);
        if (ret.is_err())
        {
            assert(false && "Failed to resolve type token in resolve_metadata_token");
            return nullptr;
        }
        const metadata::RtTypeSig* type_sig = ret.unwrap();
        auto ret2 = vm::Class::get_class_from_typesig(type_sig);
        if (ret2.is_err())
        {
            assert(false && "Failed to get class from type signature in resolve_metadata_token");
            return nullptr;
        }
        auto klass = ret2.unwrap();
        auto ret3 = vm::Class::initialize_all(klass);
        if (ret3.is_err())
        {
            assert(false && "Failed to initialize class in resolve_metadata_token");
            return nullptr;
        }
        return klass;
    }
    case metadata::TableType::Method:
    case metadata::TableType::MethodSpec:
    {
        auto ret = mod->get_method_by_token(t, gcc, generic_context);
        if (ret.is_err())
        {
            assert(false && "Failed to resolve method token in resolve_metadata_token");
            return nullptr;
        }
        const metadata::RtMethodInfo* methodInfo = ret.unwrap();
        if (vm::Class::initialize_all(const_cast<metadata::RtClass*>(methodInfo->parent)).is_err())
        {
            assert(false && "Failed to initialize method's declaring class in resolve_metadata_token");
            return nullptr;
        }
        return (void*)methodInfo;
    }
    case metadata::TableType::Field:
    {
        auto ret = mod->get_field_by_token(t, gcc, generic_context);
        if (ret.is_err())
        {
            assert(false && "Failed to resolve field token in resolve_metadata_token");
            return nullptr;
        }
        const metadata::RtFieldInfo* fieldInfo = ret.unwrap();
        if (vm::Class::initialize_all(const_cast<metadata::RtClass*>(fieldInfo->parent)).is_err())
        {
            assert(false && "Failed to initialize field's declaring class in resolve_metadata_token");
            return nullptr;
        }
        return (void*)fieldInfo;
    }
    case metadata::TableType::MemberRef:
    {
        auto ret = mod->get_member_ref_by_rid(t.rid, gcc, generic_context);
        if (ret.is_err())
        {
            assert(false && "Failed to resolve member ref token in resolve_metadata_token");
            return nullptr;
        }
        metadata::RtRuntimeHandle handle = ret.unwrap();
        switch (handle.type)
        {
        case metadata::RtRuntimeHandleType::Type:
        {
            auto ret2 = vm::Class::get_class_from_typesig(handle.typeSig);
            if (ret2.is_err())
            {
                assert(false && "Failed to get class from type signature in resolve_metadata_token (MemberRef)");
                return nullptr;
            }
            auto klass = ret2.unwrap();
            if (vm::Class::initialize_all(const_cast<metadata::RtClass*>(klass)).is_err())
            {
                assert(false && "Failed to initialize class in resolve_metadata_token (MemberRef)");
                return nullptr;
            }
            return klass;
        }
        case metadata::RtRuntimeHandleType::Method:
        {
            const metadata::RtMethodInfo* methodInfo = handle.method;
            if (vm::Class::initialize_all(const_cast<metadata::RtClass*>(methodInfo->parent)).is_err())
            {
                assert(false && "Failed to initialize method's declaring class in resolve_metadata_token (MemberRef)");
                return nullptr;
            }
            return (void*)methodInfo;
        }
        case metadata::RtRuntimeHandleType::Field:
        {
            const metadata::RtFieldInfo* fieldInfo = handle.field;
            if (vm::Class::initialize_all(const_cast<metadata::RtClass*>(fieldInfo->parent)).is_err())
            {
                assert(false && "Failed to initialize field's declaring class in resolve_metadata_token (MemberRef)");
                return nullptr;
            }
            return (void*)fieldInfo;
        }
        default:
        {
            assert(false && "Unsupported runtime handle type in resolve_metadata_token (MemberRef)");
            return nullptr;
        }
        }
    }
    default:
    {
        assert(false && "Unsupported token type in resolve_metadata_token");
        return nullptr;
    }
    }
}

const char* marshal_utf16_string_to_utf8(vm::RtString* str)
{
    if (str == nullptr)
    {
        return nullptr;
    }
    utils::StringBuilder sb;
    utils::StringUtil::utf16_to_utf8(vm::String::get_chars_ptr(str), static_cast<size_t>(vm::String::get_length(str)), sb);
    // FIXME: should we use std::malloc instead of alloc::GeneralAllocation::malloc?
    return sb.dup_to_zero_end_cstr();
}

} // namespace codegen
} // namespace leanclr
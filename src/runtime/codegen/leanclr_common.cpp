#include "leanclr_common.h"
#include <cstdio>
#include <cstring>
#include "vm/object.h"
#include "metadata/module_def.h"
#include "utils/string_builder.h"
#include "vm/rt_string.h"
#include "vm/pinvoke.h"
#include "vm/rt_exception.h"

namespace leanclr
{
namespace codegen
{

vm::RtString* resolve_string_literal(metadata::RtModuleDef* mod, uint32_t token)
{
    assert(metadata::RtToken::decode_table_type(token) == metadata::TableType::String && "Token is not a string literal");
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
    case metadata::TableType::String:
    {
        uint32_t string_index = t.rid;
        auto ret = mod->get_user_string(string_index);
        if (ret.is_err())
        {
            assert(false && "Failed to resolve string token in resolve_metadata_token");
            return nullptr;
        }
        return ret.unwrap();
    }
    default:
    {
        assert(false && "Unsupported token type in resolve_metadata_token");
        return nullptr;
    }
    }
}

void resolve_metadata_tokens(metadata::RtModuleDef* mod, const uint32_t* tokens, size_t count, void** resolved_metadatas)
{
    resolve_generic_metadata_tokens(mod, tokens, count, nullptr, resolved_metadatas);
}

void resolve_generic_metadata_tokens(metadata::RtModuleDef* mod, const uint32_t* tokens, size_t count, const metadata::RtMethodInfo* generic_method_info, void** resolved_metadatas)
{
    for (size_t i = 0; i < count; i++)
    {
        resolved_metadatas[i] = resolve_metadata_token(mod, tokens[i], generic_method_info);
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

RtErr raise_internal_call_entry_not_found_error(const char* name) noexcept
{
    char err_msg[1024];
    snprintf(err_msg, sizeof(err_msg), "Internal call entry not found: %s", name);
    RET_ERR_WITH_MSG(RtErr::EntryPointNotFound, err_msg);
}

RtErr raise_pinvoke_entry_not_found_error(const char* dll_name_no_ext, const char* function_name) noexcept
{
    char err_msg[1024];
    snprintf(err_msg, sizeof(err_msg), "P/Invoke entry not found: dll=%s, function=%s", dll_name_no_ext, function_name);
    RET_ERR_WITH_MSG(RtErr::EntryPointNotFound, err_msg);
}

void* pinvoke_marshal_delegate_to_void_ptr(vm::RtDelegate* del) noexcept
{
    const auto r = vm::Marshal::get_function_pointer_for_delegate(del);
    if (LEANCLR_UNLIKELY(r.is_err()))
    {
        return nullptr;
    }
    return r.unwrap();
}

void* pinvoke_marshal_safe_handle_to_void_ptr(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        return nullptr;
    }
    const metadata::RtClass* klass = obj->klass;
    const metadata::RtFieldInfo* fi = vm::Class::get_field_for_name(klass, "handle", true);
    if (fi == nullptr)
    {
        fi = vm::Class::get_field_for_name(klass, "_handle", true);
    }
    if (fi == nullptr)
    {
        return nullptr;
    }
    const uint8_t* field_addr =
        reinterpret_cast<const uint8_t*>(obj) + vm::Field::get_field_offset_includes_object_header_for_all_type(fi);
    return reinterpret_cast<void*>(*reinterpret_cast<const intptr_t*>(field_addr));
}

RtResult<vm::RtArray*> mono_pinvoke_reverse_marshal_szarray_blittable_copy(const metadata::RtClass* ele_klass, const void* native_element_data,
                                                                          int32_t length) noexcept
{
    if (length < 0)
    {
        RET_ERR(RtErr::Overflow);
    }
    if (native_element_data == nullptr)
    {
        RET_OK(nullptr);
    }
    if (!vm::Class::is_blittable(ele_klass))
    {
        RET_ERR(RtErr::NotSupported);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, new_szarray_from_ele_class(ele_klass, length));
    const size_t ele_size = vm::Class::get_stack_location_size(ele_klass);
    std::memcpy(vm::Array::get_array_data_start_as_ptr_void(arr), native_element_data, static_cast<size_t>(length) * ele_size);
    RET_OK(arr);
}

} // namespace codegen
} // namespace leanclr
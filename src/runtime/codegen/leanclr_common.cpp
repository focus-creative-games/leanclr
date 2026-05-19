#include "leanclr_common.h"
#include <cstdio>
#include <cstring>
#include "vm/object.h"
#include "metadata/module_def.h"
#include "utils/string_builder.h"
#include "vm/rt_string.h"
#include "vm/pinvoke.h"
#include "vm/rt_exception.h"

#if LEANCLR_PLATFORM_WIN
#include <windows.h>
#endif

namespace leanclr
{
namespace codegen
{

namespace
{
int32_t clamp_string_builder_length(int32_t length, int32_t capacity) noexcept
{
    if (length < 0)
    {
        return 0;
    }
    return length > capacity ? capacity : length;
}

void copy_utf16_to_managed_string_builder(vm::RtObject* sb, const Utf16Char* chars, int32_t length) noexcept
{
    if (sb == nullptr || chars == nullptr)
    {
        return;
    }

    Utf16Char* chunk_chars = nullptr;
    int32_t capacity = 0;
    vmutils::StringBuilder::get_chunk_chars(sb, &chunk_chars, &capacity);
    length = clamp_string_builder_length(length, capacity);
    if (length > 0)
    {
        std::memcpy(chunk_chars, chars, static_cast<size_t>(length) * sizeof(Utf16Char));
    }
    if (length < capacity)
    {
        chunk_chars[length] = 0;
    }
    vmutils::StringBuilder::set_chunk_length(sb, length);
}
} // namespace

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

void resolve_generic_metadata_tokens(metadata::RtModuleDef* mod, const uint32_t* tokens, size_t count, const metadata::RtMethodInfo* generic_method_info,
                                     void** resolved_metadatas)
{
    for (size_t i = 0; i < count; i++)
    {
        resolved_metadatas[i] = resolve_metadata_token(mod, tokens[i], generic_method_info);
    }
}

RtMarshalAnsiStr marshal_managed_string_to_ansi_string(vm::RtString* str, AnsiStringBuilder& temp) noexcept
{
    if (str == nullptr)
    {
        return nullptr;
    }
    temp.append_utf16_str(vm::String::get_chars_ptr(str), static_cast<size_t>(vm::String::get_length(str)));
    return (RtMarshalAnsiStr)temp.get_mut_chars();
}

RtMarshalAnsiStr marshal_managed_string_to_ansi_string(vm::RtString* str) noexcept
{
    if (str == nullptr)
    {
        return nullptr;
    }
    AnsiStringBuilder temp(vm::String::get_chars_ptr(str), static_cast<size_t>(vm::String::get_length(str)));
    return (RtMarshalAnsiStr)temp.dup_zero_terminated_chars();
}

vm::RtString* marshal_ansi_string_to_managed_string(const RtMarshalAnsiStr str) noexcept
{
    if (str == nullptr)
    {
        return nullptr;
    }
    Utf16StringBuilder temp;
    temp.append_ansi_str(str);
    return vm::String::create_string_from_utf16chars(temp.get_const_chars(), static_cast<int32_t>(temp.length()));
}

RtMarshalUTF8Str marshal_managed_string_builder_to_utf8_string(vm::RtObject* sb, Utf8StringBuilder& temp) noexcept
{
    if (sb == nullptr)
    {
        return nullptr;
    }

    Utf16Char* chars = nullptr;
    int32_t capacity = 0;
    vmutils::StringBuilder::get_chunk_chars(sb, &chars, &capacity);
    int32_t length = clamp_string_builder_length(vmutils::StringBuilder::get_chunk_length(sb), capacity);
    temp.append_utf16_str(chars, static_cast<size_t>(length));
    // TODO: optimize it , we should avoid the copy here.
    return (RtMarshalUTF8Str)temp.get_mut_chars();
}

RtMarshalUTF16Str marshal_managed_string_builder_to_utf16_string(vm::RtObject* sb) noexcept
{
    if (sb == nullptr)
    {
        return nullptr;
    }

    Utf16Char* chars = nullptr;
    int32_t capacity = 0;
    vmutils::StringBuilder::get_chunk_chars(sb, &chars, &capacity);
    int32_t length = clamp_string_builder_length(vmutils::StringBuilder::get_chunk_length(sb), capacity);
    if (length < capacity)
    {
        chars[length] = 0;
    }
    return chars;
}

void sync_managed_string_builder_from_utf16_buffer(vm::RtObject* sb, const Utf16Char* str) noexcept
{
    if (sb == nullptr || str == nullptr)
    {
        return;
    }

    copy_utf16_to_managed_string_builder(sb, str, utils::StringUtil::get_utf16chars_length(str));
}

RtMarshalAnsiStr marshal_managed_string_builder_to_ansi_string(vm::RtObject* sb, AnsiStringBuilder& temp) noexcept
{
    if (sb == nullptr)
    {
        return nullptr;
    }

    Utf16Char* chars = nullptr;
    int32_t capacity = 0;
    vmutils::StringBuilder::get_chunk_chars(sb, &chars, &capacity);
    int32_t length = clamp_string_builder_length(vmutils::StringBuilder::get_chunk_length(sb), capacity);
    temp.append_utf16_str(chars, static_cast<size_t>(length));
    return (RtMarshalAnsiStr)temp.get_mut_chars();
}

void sync_managed_string_builder_from_ansi_buffer(vm::RtObject* sb, const RtMarshalAnsiStr str) noexcept
{
    if (str == nullptr || sb == nullptr)
    {
        return;
    }
    Utf16StringBuilder temp;
    temp.append_ansi_str(str);
    copy_utf16_to_managed_string_builder(sb, temp.get_const_chars(), static_cast<int32_t>(temp.length()));
}

void sync_managed_string_builder_from_utf8_buffer(vm::RtObject* sb, const Utf8Char* str) noexcept
{
    if (str == nullptr || sb == nullptr)
    {
        return;
    }
    Utf16StringBuilder temp;
    temp.append_utf8_str(str);
    copy_utf16_to_managed_string_builder(sb, temp.get_const_chars(), static_cast<int32_t>(temp.length()));
}

RtErr raise_internal_call_entry_not_found_error(const char* name) noexcept
{
    char err_msg[1024];
    snprintf(err_msg, sizeof(err_msg), "Internal call not implemented: %s", name);
    RET_ERR_WITH_MSG(RtErr::NotImplemented, err_msg);
}

RtErr raise_pinvoke_entry_not_found_error(const char* dll_name_no_ext, const char* function_name) noexcept
{
    char err_msg[1024];
    snprintf(err_msg, sizeof(err_msg), "P/Invoke entry not found: dll=%s, function=%s", dll_name_no_ext, function_name);
    RET_ERR_WITH_MSG(RtErr::EntryPointNotFound, err_msg);
}

RtMarshalHandle marshal_safe_handle_to_handle(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        return 0;
    }
#if LEANCLR_DEBUG
    const metadata::RtClass* klass = obj->klass;
    const metadata::RtFieldInfo* fi = vm::Class::get_field_for_name(klass, "handle", true);
    if (fi == nullptr)
    {
        fi = vm::Class::get_field_for_name(klass, "_handle", true);
    }
    assert(fi);
    assert(fi->offset == 0);
#endif
    return *reinterpret_cast<RtMarshalHandle*>(obj + 1);
}

RtResult<vm::RtObject*> marshal_handle_to_safe_handle(RtMarshalHandle handle, const metadata::RtTypeSig* handle_typesig) noexcept
{
    if (handle == 0)
    {
        RET_OK(nullptr);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(handle_typesig));
#if LEANCLR_DEBUG
    RET_ERR_ON_FAIL(vm::Class::initialize_all(klass));
    const metadata::RtFieldInfo* fi = vm::Class::get_field_for_name(klass, "handle", true);
    if (fi == nullptr)
    {
        fi = vm::Class::get_field_for_name(klass, "_handle", true);
    }
    assert(fi);
    assert(fi->offset == 0);
#endif
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, LEANCLR_CODEGEN_NEWOBJ(klass, "codegen::marshal_handle_to_safe_handle"));
    RtMarshalHandle* field_addr = reinterpret_cast<RtMarshalHandle*>(obj + 1);
    *field_addr = handle;
    RET_OK(obj);
}

RtResult<vm::RtArray*> marshal_native_array_to_managed_array(const metadata::RtTypeSig* array_param_typesig, const void* native_element_data,
                                                             int32_t length) noexcept
{
    assert(array_param_typesig != nullptr);
    if (length < 0)
    {
        RET_ERR(RtErr::Overflow);
    }
    if (native_element_data == nullptr)
    {
        RET_OK(nullptr);
    }
    assert(array_param_typesig->ele_type == metadata::RtElementType::SZArray);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, arr_klass, vm::Class::get_class_from_typesig(array_param_typesig));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(arr_klass));
    if (!vm::Class::is_blittable(arr_klass->element_class))
    {
        RET_ERR(RtErr::NotSupported);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, LEANCLR_CODEGEN_NEW_SZARRAY_FROM_ARRAY_KLASS(arr_klass, length, "codegen::marshal_native_array_to_managed_array"));
    const size_t ele_size = vm::Array::get_array_element_size(arr);
    std::memcpy(vm::Array::get_array_data_start_as_ptr_void(arr), native_element_data, static_cast<size_t>(length) * ele_size);
    RET_OK(arr);
}

void* marshal_managed_array_to_native_array(vm::RtArray* managed_array, size_t& native_element_count) noexcept
{
    if (managed_array == nullptr)
    {
        return nullptr;
    }
    const size_t ele_size = vm::Array::get_array_element_size(managed_array);
    native_element_count = static_cast<size_t>(vm::Array::get_array_length(managed_array));

    void* native_array = alloc::GeneralAllocation::malloc(native_element_count * ele_size);
    std::memcpy(native_array, vm::Array::get_array_data_start_as_ptr_void(managed_array), native_element_count * ele_size);
    return native_array;
}

RtResult<vm::RtArray*> marshal_native_array_to_managed_array(void* native_array, size_t native_element_count,
                                                             const metadata::RtTypeSig* array_param_typesig) noexcept
{
    assert(array_param_typesig != nullptr);
    if (native_array == nullptr)
    {
        RET_OK(nullptr);
    }
    assert(array_param_typesig->ele_type == metadata::RtElementType::SZArray);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, arr_klass, vm::Class::get_class_from_typesig(array_param_typesig));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, LEANCLR_CODEGEN_NEW_SZARRAY_FROM_ARRAY_KLASS(arr_klass, static_cast<int32_t>(native_element_count), "codegen::marshal_native_array_to_managed_array"));
    const size_t ele_size = vm::Array::get_array_element_size(arr);
    std::memcpy(vm::Array::get_array_data_start_as_ptr_void(arr), native_array, native_element_count * ele_size);
    RET_OK(arr);
}

void free_native_array(void* native_array) noexcept
{
    alloc::GeneralAllocation::free(native_array);
}

void marshal_managed_array_to_native_val_array(vm::RtArray* managed_array, void* native_array, size_t native_element_count) noexcept
{
    if (managed_array == nullptr)
    {
        return;
    }
    const size_t ele_size = vm::Array::get_array_element_size(managed_array);
    std::memcpy(native_array, vm::Array::get_array_data_start_as_ptr_void(managed_array), native_element_count * ele_size);
}

RtResult<vm::RtArray*> marshal_native_val_array_to_managed_array(void* native_array, size_t native_element_count,
                                                                 const metadata::RtTypeSig* array_typesig) noexcept
{
    if (native_array == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    assert(array_typesig != nullptr);
    assert(array_typesig->ele_type == metadata::RtElementType::SZArray);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, arr_klass, vm::Class::get_class_from_typesig(array_typesig));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, arr, LEANCLR_CODEGEN_NEW_SZARRAY_FROM_ARRAY_KLASS(arr_klass, static_cast<int32_t>(native_element_count), "codegen::marshal_native_val_array_to_managed_array"));
    const size_t ele_size = vm::Array::get_array_element_size(arr);
    std::memcpy(vm::Array::get_array_data_start_as_ptr_void(arr), native_array, native_element_count * ele_size);
    RET_OK(arr);
}

} // namespace codegen
} // namespace leanclr
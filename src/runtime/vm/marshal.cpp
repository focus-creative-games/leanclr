#include "marshal.h"

#include "alloc/general_allocation.h"
#include "rt_string.h"
#include "class.h"
#include "field.h"
#include "utils/string_util.h"
#include "utils/string_builder.h"
#include "platform/kernel32.h"

namespace leanclr
{
namespace vm
{

void* leanclr::vm::Marshal::alloc_hglobal(size_t size)
{
    return alloc::GeneralAllocation::malloc(size);
}

void* Marshal::re_alloc_hglobal(void* ptr, size_t size)
{
    return alloc::GeneralAllocation::realloc(ptr, size);
}

void Marshal::free_hglobal(void* ptr)
{
    alloc::GeneralAllocation::free(ptr);
}

void* Marshal::alloc_co_task_mem(int32_t size)
{
    return alloc_hglobal(static_cast<size_t>(size));
}

void* Marshal::re_alloc_co_task_mem(void* ptr, int32_t size)
{
    return re_alloc_hglobal(ptr, static_cast<size_t>(size));
}

void Marshal::free_co_task_mem(void* ptr)
{
    free_hglobal(ptr);
}

void* Marshal::alloc_co_task_mem_size(size_t size)
{
    return alloc_hglobal(size);
}

vm::RtString* Marshal::ptr_to_string_ansi(void* ptr)
{
    return String::create_string_from_utf8cstr(reinterpret_cast<const char*>(ptr));
    ;
}

vm::RtString* Marshal::ptr_to_string_ansi_len(void* ptr, int32_t len)
{
    return String::create_string_from_utf8chars(reinterpret_cast<const char*>(ptr), len);
}

vm::RtString* Marshal::ptr_to_string_uni(void* ptr)
{
    int32_t utf16_length = utils::StringUtil::get_utf16chars_length(reinterpret_cast<const Utf16Char*>(ptr));
    return String::create_string_from_utf16chars(reinterpret_cast<const uint16_t*>(ptr), utf16_length);
}

vm::RtString* Marshal::ptr_to_string_uni_len(void* ptr, int32_t len)
{
    return String::create_string_from_utf16chars(reinterpret_cast<const uint16_t*>(ptr), len);
}

vm::RtString* Marshal::ptr_to_string_bstr(void* ptr)
{
    return ptr_to_string_uni(ptr);
}

void* Marshal::string_to_hglobal_ansi(const Utf16Char* chars, int32_t len)
{
    utils::StringBuilder utf8_str;
    utils::StringUtil::utf16_to_utf8(chars, static_cast<size_t>(len), utf8_str);
    return const_cast<char*>(utils::StringUtil::strdup(utf8_str.as_cstr()));
}

void* Marshal::string_to_hglobal_uni(const Utf16Char* chars, int32_t len)
{
    return (void*)utils::StringUtil::strdup_utf16_with_null_terminator(chars, static_cast<size_t>(len));
}

void* Marshal::buffer_to_bstr(const Utf16Char* chars, int32_t len)
{
    return (void*)utils::StringUtil::strdup_utf16_with_null_terminator(chars, static_cast<size_t>(len));
}

void Marshal::free_bstr(void* ptr)
{
    alloc::GeneralAllocation::free(ptr);
}

RtResultVoid Marshal::ptr_to_structure(void* ptr, vm::RtObject* obj)
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<vm::RtObject*> Marshal::ptr_to_structure_type(void* ptr, vm::RtReflectionType* ref_type)
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResultVoid Marshal::structure_to_ptr(vm::RtObject* obj, void* ptr, int32_t delete_old)
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResultVoid Marshal::destroy_structure(void* ptr, vm::RtReflectionType* ref_type)
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<int32_t> Marshal::sizeof_type(vm::RtReflectionType* ref_type)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(ref_type->type_handle));
    RET_ERR_ON_FAIL(Class::initialize_fields(klass));
    int32_t size = static_cast<int32_t>(vm::Class::get_instance_size_without_object_header(klass));
    RET_OK(size);
}

RtResult<intptr_t> Marshal::offset_of(vm::RtReflectionType* ref_type, const char* field_name)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, Class::get_class_from_typesig(ref_type->type_handle));
    RET_ERR_ON_FAIL(Class::initialize_fields(klass));
    const metadata::RtFieldInfo* field_info = Class::get_field_for_name(klass, field_name, false);
    if (!field_info)
    {
        RET_ERR(RtErr::FileNotFound);
    }
    int32_t offset = static_cast<int32_t>(Field::get_field_offset_excludes_object_header_for_all_type(field_info));
    RET_OK(static_cast<intptr_t>(offset));
}

RtResult<void*> Marshal::unsafe_addr_of_pinned_array_element(vm::RtArray* arr, int32_t index)
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<RtDelegate*> Marshal::marshal_function_pointer_to_delegate(void* ptr, metadata::RtClass* delegate_class)
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<void*> Marshal::get_function_pointer_for_delegate(RtDelegate* delegate)
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

int32_t Marshal::get_last_win32_error()
{
    return platform::Kernel32::get_last_win32_error();
}

void Marshal::set_last_win32_error(int32_t error)
{
    platform::Kernel32::set_last_win32_error(error);
}

} // namespace vm
} // namespace leanclr

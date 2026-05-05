#pragma once

#include "rt_managed_types.h"
#include "rt_string.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace vm
{

class TempUtf16StringToUtf8Converter
{
  public:
    TempUtf16StringToUtf8Converter(RtString* str)
    {
        if (str)
        {
            _utf8_str_builder.append_utf16_str(String::get_chars_ptr(str), static_cast<size_t>(String::get_length(str)));
            _utf8_str = _utf8_str_builder.as_cstr();
        }
        else
        {
            _utf8_str = nullptr;
        }
    }

    const char* get_utf8_str() const
    {
        return _utf8_str;
    }

  private:
    utils::StringBuilder _utf8_str_builder;
    const char* _utf8_str;
};

class Marshal
{
  public:
    static void* alloc_hglobal(size_t size);
    static void* re_alloc_hglobal(void* ptr, size_t size);
    static void free_hglobal(void* ptr);

    static void* alloc_co_task_mem(int32_t size);
    static void* re_alloc_co_task_mem(void* ptr, int32_t size);
    static void free_co_task_mem(void* ptr);
    static void* alloc_co_task_mem_size(size_t size);

    static vm::RtString* ptr_to_string_ansi(void* ptr);
    static vm::RtString* ptr_to_string_ansi_len(void* ptr, int32_t len);
    static vm::RtString* ptr_to_string_uni(void* ptr);
    static vm::RtString* ptr_to_string_uni_len(void* ptr, int32_t len);
    static vm::RtString* ptr_to_string_bstr(void* ptr);

    static void* string_to_hglobal_ansi(const Utf16Char* chars, int32_t len);
    static void* string_to_hglobal_uni(const Utf16Char* chars, int32_t len);
    static void* buffer_to_bstr(const Utf16Char* chars, int32_t len);
    static void free_bstr(void* ptr);

    static RtResultVoid ptr_to_structure(void* ptr, vm::RtObject* obj);
    static RtResult<vm::RtObject*> ptr_to_structure_type(void* ptr, vm::RtReflectionType* ref_type);
    static RtResultVoid structure_to_ptr(vm::RtObject* obj, void* ptr, int32_t delete_old);
    static RtResultVoid destroy_structure(void* ptr, vm::RtReflectionType* ref_type);

    static RtResult<int32_t> sizeof_type(vm::RtReflectionType* ref_type);
    static RtResult<intptr_t> offset_of(vm::RtReflectionType* ref_type, const char* field_name);
    static RtResult<void*> unsafe_addr_of_pinned_array_element(vm::RtArray* arr, int32_t index);

    static RtResult<RtDelegate*> marshal_function_pointer_to_delegate(void* ptr, metadata::RtClass* delegate_class);
    static RtResult<void*> get_function_pointer_for_delegate(RtDelegate* delegate);

    static int32_t get_last_win32_error();
    static void set_last_win32_error(int32_t error);
};
} // namespace vm
} // namespace leanclr

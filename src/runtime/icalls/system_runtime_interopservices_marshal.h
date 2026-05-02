#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemRuntimeInteropServicesMarshal
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;

    // Memory allocation
    static RtResult<void*> alloc_hglobal(size_t size) noexcept;
    static RtResult<void*> re_alloc_hglobal(void* ptr, size_t size) noexcept;
    static RtResultVoid free_hglobal(void* ptr) noexcept;
    static RtResult<void*> alloc_co_task_mem(int32_t size) noexcept;
    static RtResult<void*> re_alloc_co_task_mem(void* ptr, int32_t size) noexcept;
    static RtResultVoid free_co_task_mem(void* ptr) noexcept;
    static RtResult<void*> alloc_co_task_mem_size(size_t size) noexcept;

    // String conversion
    static RtResult<vm::RtString*> ptr_to_string_ansi(void* ptr) noexcept;
    static RtResult<vm::RtString*> ptr_to_string_ansi_len(void* ptr, int32_t len) noexcept;
    static RtResult<vm::RtString*> ptr_to_string_uni(void* ptr) noexcept;
    static RtResult<vm::RtString*> ptr_to_string_uni_len(void* ptr, int32_t len) noexcept;
    static RtResult<vm::RtString*> ptr_to_string_bstr(void* ptr) noexcept;
    static RtResult<void*> string_to_hglobal_ansi(const Utf16Char* chars, int32_t len) noexcept;
    static RtResult<void*> string_to_hglobal_uni(const Utf16Char* chars, int32_t len) noexcept;
    static RtResult<void*> buffer_to_bstr(const Utf16Char* chars, int32_t len) noexcept;
    static RtResultVoid free_bstr(void* ptr) noexcept;

    // Structure marshaling
    static RtResultVoid ptr_to_structure(void* ptr, vm::RtObject* obj) noexcept;
    static RtResult<vm::RtObject*> ptr_to_structure_type(void* ptr, vm::RtReflectionType* ref_type) noexcept;
    static RtResultVoid structure_to_ptr(vm::RtObject* obj, void* ptr, int32_t delete_old) noexcept;
    static RtResultVoid destroy_structure(void* ptr, vm::RtReflectionType* ref_type) noexcept;

    // Type operations
    static RtResult<int32_t> sizeof_type(vm::RtReflectionType* ref_type) noexcept;
    static RtResult<intptr_t> offset_of(vm::RtReflectionType* ref_type, vm::RtString* field_name) noexcept;

    // Array operations
    static RtResult<void*> unsafe_addr_of_pinned_array_element(vm::RtArray* arr, int32_t index) noexcept;
    static RtResultVoid copy_to_unmanaged_fixed(vm::RtArray* arr, int32_t start_index, void* dest, int32_t length, void* etype) noexcept;
    static RtResultVoid copy_from_unmanaged_fixed(void* src, int32_t start_index, vm::RtArray* arr, int32_t length, void* etype) noexcept;

    // Delegate marshaling
    static RtResult<vm::RtDelegate*> get_delegate_for_function_pointer_internal(void* ptr, vm::RtReflectionType* ref_type) noexcept;
    static RtResult<void*> get_function_pointer_for_delegate_internal(vm::RtDelegate* delegate) noexcept;

    // Win32 error
    static RtResult<int32_t> get_last_win32_error() noexcept;
    static RtResultVoid set_last_win32_error(int32_t error) noexcept;

    // COM/WinRT stubs (not implemented)
    static RtResult<int32_t> query_interface_internal(void* ptr, void* guid_ref, void* out_ptr_ref) noexcept;
    static RtResult<int32_t> release_internal(void* ptr) noexcept;
    static RtResult<int32_t> release_com_object_internal(vm::RtObject* obj) noexcept;
    static RtResult<void*> get_raw_iunknown_for_com_object_no_add_ref(vm::RtObject* obj) noexcept;
    static RtResult<int32_t> get_hr_for_exception_winrt(vm::RtObject* exception) noexcept;
    static RtResult<vm::RtObject*> get_native_activation_factory(vm::RtObject* type_obj) noexcept;
    static RtResult<int32_t> add_ref_internal(void* ptr) noexcept;
    static RtResult<void*> get_iunknown_for_object_internal(vm::RtObject* obj) noexcept;
    static RtResult<void*> get_idispatch_for_object_internal(vm::RtObject* obj) noexcept;
    static RtResult<void*> get_ccw(vm::RtObject* obj, vm::RtObject* type_obj) noexcept;
    static RtResult<vm::RtObject*> get_object_for_ccw(void* ptr) noexcept;

    // Prelink stubs
    static RtResultVoid prelink_all(vm::RtReflectionType* ref_type) noexcept;
    static RtResultVoid prelink(vm::RtObject* method_info) noexcept;
};

} // namespace icalls
} // namespace leanclr

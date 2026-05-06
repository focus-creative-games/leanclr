#include "system_runtime_interopservices_marshal.h"
#include "icall_base.h"
#include "vm/runtime.h"
#include "vm/rt_string.h"
#include "vm/rt_array.h"
#include "vm/object.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/marshal.h"
#include "utils/string_builder.h"
#include "utils/string_util.h"

namespace leanclr
{
namespace icalls
{

// ========== Memory allocation ==========

RtResult<void*> SystemRuntimeInteropServicesMarshal::alloc_hglobal(size_t size) noexcept
{
    void* ptr = vm::Marshal::alloc_hglobal(size);
    RET_OK(ptr);
}

RtResult<void*> SystemRuntimeInteropServicesMarshal::re_alloc_hglobal(void* ptr, size_t size) noexcept
{
    void* new_ptr = vm::Marshal::re_alloc_hglobal(ptr, size);
    RET_OK(new_ptr);
}

RtResultVoid SystemRuntimeInteropServicesMarshal::free_hglobal(void* ptr) noexcept
{
    if (ptr != 0)
    {
        vm::Marshal::free_hglobal(ptr);
    }
    RET_VOID_OK();
}

RtResult<void*> SystemRuntimeInteropServicesMarshal::alloc_co_task_mem(int32_t size) noexcept
{
    if (size < 0)
    {
        RET_ERR(RtErr::Argument);
    }

    void* ptr = vm::Marshal::alloc_co_task_mem(size);
    RET_OK(ptr);
}

RtResult<void*> SystemRuntimeInteropServicesMarshal::re_alloc_co_task_mem(void* ptr, int32_t size) noexcept
{
    if (size < 0)
    {
        RET_ERR(RtErr::Argument);
    }

    void* new_ptr = vm::Marshal::re_alloc_co_task_mem(ptr, size);
    RET_OK(new_ptr);
}

RtResultVoid SystemRuntimeInteropServicesMarshal::free_co_task_mem(void* ptr) noexcept
{
    if (ptr != 0)
    {
        vm::Marshal::free_co_task_mem(ptr);
    }
    RET_VOID_OK();
}

RtResult<void*> SystemRuntimeInteropServicesMarshal::alloc_co_task_mem_size(size_t size) noexcept
{
    void* ptr = vm::Marshal::alloc_co_task_mem_size(size);
    RET_OK(ptr);
}

// ========== String conversion ==========

RtResult<vm::RtString*> SystemRuntimeInteropServicesMarshal::ptr_to_string_ansi(void* ptr) noexcept
{
    if (ptr == 0)
    {
        RET_OK(nullptr);
    }

    vm::RtString* result = vm::Marshal::ptr_to_string_ansi(ptr);
    RET_OK(result);
}

RtResult<vm::RtString*> SystemRuntimeInteropServicesMarshal::ptr_to_string_ansi_len(void* ptr, int32_t len) noexcept
{
    if (ptr == 0)
    {
        RET_OK(nullptr);
    }

    if (len < 0)
    {
        RET_ERR(RtErr::Argument);
    }

    vm::RtString* result = vm::Marshal::ptr_to_string_ansi_len(ptr, len);
    RET_OK(result);
}

RtResult<vm::RtString*> SystemRuntimeInteropServicesMarshal::ptr_to_string_uni(void* ptr) noexcept
{
    if (ptr == 0)
    {
        RET_OK(nullptr);
    }

    vm::RtString* result = vm::Marshal::ptr_to_string_uni(ptr);
    RET_OK(result);
}

RtResult<vm::RtString*> SystemRuntimeInteropServicesMarshal::ptr_to_string_uni_len(void* ptr, int32_t len) noexcept
{
    if (ptr == 0)
    {
        RET_OK(nullptr);
    }

    if (len < 0)
    {
        RET_ERR(RtErr::Argument);
    }
    vm::RtString* result = vm::Marshal::ptr_to_string_uni_len(ptr, len);
    RET_OK(result);
}

RtResult<vm::RtString*> SystemRuntimeInteropServicesMarshal::ptr_to_string_bstr(void* ptr) noexcept
{
    if (ptr == 0)
    {
        RET_OK(nullptr);
    }
    vm::RtString* result = vm::Marshal::ptr_to_string_bstr(ptr);
    RET_OK(result);
}

RtResult<void*> SystemRuntimeInteropServicesMarshal::string_to_hglobal_ansi(const Utf16Char* chars, int32_t len) noexcept
{
    if (!chars)
    {
        RET_OK(nullptr);
    }
    void* buffer = vm::Marshal::string_to_hglobal_ansi(chars, len);
    RET_OK(buffer);
}

RtResult<void*> SystemRuntimeInteropServicesMarshal::string_to_hglobal_uni(const Utf16Char* chars, int32_t len) noexcept
{
    if (!chars)
    {
        RET_OK(nullptr);
    }

    void* buffer = vm::Marshal::string_to_hglobal_uni(chars, len);
    RET_OK(buffer);
}

RtResult<void*> SystemRuntimeInteropServicesMarshal::buffer_to_bstr(const Utf16Char* chars, int32_t len) noexcept
{
    if (!chars)
    {
        RET_OK(nullptr);
    }
    void* bstr = vm::Marshal::buffer_to_bstr(chars, len);
    RET_OK(bstr);
}

RtResultVoid SystemRuntimeInteropServicesMarshal::free_bstr(void* ptr) noexcept
{
    if (ptr)
    {
        vm::Marshal::free_bstr(ptr);
    }
    RET_VOID_OK();
}

// ========== Structure marshaling ==========

RtResultVoid SystemRuntimeInteropServicesMarshal::ptr_to_structure(void* ptr, vm::RtObject* obj) noexcept
{
    if (!ptr || !obj)
    {
        RET_ERR(RtErr::Argument);
    }
    return vm::Marshal::ptr_to_structure(ptr, obj);
}

RtResult<vm::RtObject*> SystemRuntimeInteropServicesMarshal::ptr_to_structure_type(void* ptr, vm::RtReflectionType* ref_type) noexcept
{
    if (!ptr || !ref_type)
    {
        RET_ERR(RtErr::Argument);
    }
    return vm::Marshal::ptr_to_structure_type(ptr, ref_type);
}

RtResultVoid SystemRuntimeInteropServicesMarshal::structure_to_ptr(vm::RtObject* obj, void* ptr, int32_t delete_old) noexcept
{
    if (!obj || !ptr)
    {
        RET_ERR(RtErr::Argument);
    }
    return vm::Marshal::structure_to_ptr(obj, ptr, delete_old);
}

RtResultVoid SystemRuntimeInteropServicesMarshal::destroy_structure(void* ptr, vm::RtReflectionType* ref_type) noexcept
{
    if (!ptr || !ref_type)
    {
        RET_ERR(RtErr::Argument);
    }
    return vm::Marshal::destroy_structure(ptr, ref_type);
}

// ========== Type operations ==========

RtResult<int32_t> SystemRuntimeInteropServicesMarshal::sizeof_type(vm::RtReflectionType* ref_type) noexcept
{
    return vm::Marshal::sizeof_type(ref_type);
}

RtResult<intptr_t> SystemRuntimeInteropServicesMarshal::offset_of(vm::RtReflectionType* ref_type, vm::RtString* field_name) noexcept
{
    utils::StringBuilder utf8_field_name;
    utils::StringUtil::utf16_to_utf8(vm::String::get_chars_ptr(field_name), static_cast<size_t>(field_name->length), utf8_field_name);
    return vm::Marshal::offset_of(ref_type, utf8_field_name.as_cstr());
}

// ========== Array operations ==========

RtResult<void*> SystemRuntimeInteropServicesMarshal::unsafe_addr_of_pinned_array_element(vm::RtArray* arr, int32_t index) noexcept
{
    if (!arr)
    {
        RET_ERR(RtErr::NullReference);
    }
    void* addr = vm::Array::get_array_element_address_as_ptr_void(arr, index);
    RET_OK(addr);
}

RtResultVoid SystemRuntimeInteropServicesMarshal::copy_to_unmanaged_fixed(vm::RtArray* arr, int32_t start_index, void* dest, int32_t length,
                                                                          void* etype) noexcept
{
    assert(arr);
    assert(length >= 0);
    assert(start_index >= 0);
    if (length > 0)
    {
        size_t ele_size = vm::Array::get_array_element_size(arr);
        std::memcpy(dest, vm::Array::get_array_element_address_with_size_as_ptr_void(arr, start_index, ele_size), static_cast<size_t>(length) * ele_size);
    }

    RET_VOID_OK();
}

RtResultVoid SystemRuntimeInteropServicesMarshal::copy_from_unmanaged_fixed(void* src, int32_t start_index, vm::RtArray* arr, int32_t length,
                                                                            void* etype) noexcept
{
    assert(arr);
    assert(length >= 0);
    assert(start_index >= 0);
    if (length > 0)
    {
        size_t ele_size = vm::Array::get_array_element_size(arr);
        std::memcpy(vm::Array::get_array_element_address_with_size_as_ptr_void(arr, start_index, ele_size), src, static_cast<size_t>(length) * ele_size);
    }

    RET_VOID_OK();
}

// ========== Delegate marshaling ==========

RtResult<vm::RtDelegate*> SystemRuntimeInteropServicesMarshal::get_delegate_for_function_pointer_internal(void* ptr, vm::RtReflectionType* ref_type) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(ref_type->type_handle));
    return vm::Marshal::marshal_function_pointer_to_delegate(ptr, klass);
}

RtResult<void*> SystemRuntimeInteropServicesMarshal::get_function_pointer_for_delegate_internal(vm::RtDelegate* delegate) noexcept
{
    return vm::Marshal::get_function_pointer_for_delegate(delegate);
}

// ========== Win32 error ==========

RtResult<int32_t> SystemRuntimeInteropServicesMarshal::get_last_win32_error() noexcept
{
    RET_OK(vm::Marshal::get_last_win32_error());
}

RtResultVoid SystemRuntimeInteropServicesMarshal::set_last_win32_error(int32_t error) noexcept
{
    vm::Marshal::set_last_win32_error(error);
    RET_VOID_OK();
}

// ========== COM/WinRT stubs ==========

RtResult<int32_t> SystemRuntimeInteropServicesMarshal::query_interface_internal(void* ptr, void* guid_ref, void* out_ptr_ref) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<int32_t> SystemRuntimeInteropServicesMarshal::release_internal(void* ptr) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<int32_t> SystemRuntimeInteropServicesMarshal::release_com_object_internal(vm::RtObject* obj) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<void*> SystemRuntimeInteropServicesMarshal::get_raw_iunknown_for_com_object_no_add_ref(vm::RtObject* obj) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<int32_t> SystemRuntimeInteropServicesMarshal::get_hr_for_exception_winrt(vm::RtObject* exception) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<vm::RtObject*> SystemRuntimeInteropServicesMarshal::get_native_activation_factory(vm::RtObject* type_obj) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<int32_t> SystemRuntimeInteropServicesMarshal::add_ref_internal(void* ptr) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<void*> SystemRuntimeInteropServicesMarshal::get_iunknown_for_object_internal(vm::RtObject* obj) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<void*> SystemRuntimeInteropServicesMarshal::get_idispatch_for_object_internal(vm::RtObject* obj) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<void*> SystemRuntimeInteropServicesMarshal::get_ccw(vm::RtObject* obj, vm::RtObject* type_obj) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResult<vm::RtObject*> SystemRuntimeInteropServicesMarshal::get_object_for_ccw(void* ptr) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

// ========== Prelink stubs ==========

RtResultVoid SystemRuntimeInteropServicesMarshal::prelink_all(vm::RtReflectionType* ref_type) noexcept
{
    // Stub: no-op
    RET_VOID_OK();
}

RtResultVoid SystemRuntimeInteropServicesMarshal::prelink(vm::RtObject* method_info) noexcept
{
    // Stub: no-op
    RET_VOID_OK();
}

// ========== Invokers ==========

// Memory allocation invokers
/// @icall: System.Runtime.InteropServices.Marshal::AllocHGlobal(System.IntPtr)
static RtResultVoid alloc_hglobal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    intptr_t size = EvalStackOp::get_param<intptr_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, ptr, SystemRuntimeInteropServicesMarshal::alloc_hglobal(static_cast<size_t>(size)));
    EvalStackOp::set_return(ret, ptr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::ReAllocHGlobal(System.IntPtr,System.IntPtr)
static RtResultVoid re_alloc_hglobal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    intptr_t size = EvalStackOp::get_param<intptr_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, new_ptr, SystemRuntimeInteropServicesMarshal::re_alloc_hglobal(ptr, static_cast<size_t>(size)));
    EvalStackOp::set_return(ret, new_ptr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::FreeHGlobal(System.IntPtr)
static RtResultVoid free_hglobal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                         interp::RtStackObject* /*ret*/) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    return SystemRuntimeInteropServicesMarshal::free_hglobal(ptr);
}

/// @icall: System.Runtime.InteropServices.Marshal::AllocCoTaskMem(System.Int32)
static RtResultVoid alloc_co_task_mem_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    int32_t size = EvalStackOp::get_param<int32_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, ptr, SystemRuntimeInteropServicesMarshal::alloc_co_task_mem(size));
    EvalStackOp::set_return(ret, ptr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::ReAllocCoTaskMem(System.IntPtr,System.Int32)
static RtResultVoid re_alloc_co_task_mem_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    int32_t size = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, new_ptr, SystemRuntimeInteropServicesMarshal::re_alloc_co_task_mem(ptr, size));
    EvalStackOp::set_return(ret, new_ptr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::FreeCoTaskMem(System.IntPtr)
static RtResultVoid free_co_task_mem_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* /*ret*/) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    return SystemRuntimeInteropServicesMarshal::free_co_task_mem(ptr);
}

/// @icall: System.Runtime.InteropServices.Marshal::AllocCoTaskMemSize(System.UIntPtr)
static RtResultVoid alloc_co_task_mem_size_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* ret) noexcept
{
    size_t size = EvalStackOp::get_param<size_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, ptr, SystemRuntimeInteropServicesMarshal::alloc_co_task_mem_size(size));
    EvalStackOp::set_return(ret, ptr);
    RET_VOID_OK();
}

// String conversion invokers
/// @icall: System.Runtime.InteropServices.Marshal::PtrToStringAnsi(System.IntPtr)
static RtResultVoid ptr_to_string_ansi_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemRuntimeInteropServicesMarshal::ptr_to_string_ansi(ptr));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::PtrToStringAnsi(System.IntPtr,System.Int32)
static RtResultVoid ptr_to_string_ansi_len_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* ret) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    int32_t len = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemRuntimeInteropServicesMarshal::ptr_to_string_ansi_len(ptr, len));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::PtrToStringUni(System.IntPtr)
static RtResultVoid ptr_to_string_uni_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* ret) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemRuntimeInteropServicesMarshal::ptr_to_string_uni(ptr));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::PtrToStringUni(System.IntPtr,System.Int32)
static RtResultVoid ptr_to_string_uni_len_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                  interp::RtStackObject* ret) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    int32_t len = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemRuntimeInteropServicesMarshal::ptr_to_string_uni_len(ptr, len));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::PtrToStringBSTR(System.IntPtr)
static RtResultVoid ptr_to_string_bstr_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtString*, str, SystemRuntimeInteropServicesMarshal::ptr_to_string_bstr(ptr));
    EvalStackOp::set_return(ret, str);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::StringToHGlobalAnsi(System.Char*,System.Int32)
static RtResultVoid string_to_hglobal_ansi_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                   interp::RtStackObject* ret) noexcept
{
    const Utf16Char* chars = EvalStackOp::get_param<const Utf16Char*>(params, 0);
    int32_t len = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, ptr, SystemRuntimeInteropServicesMarshal::string_to_hglobal_ansi(chars, len));
    EvalStackOp::set_return(ret, ptr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::StringToHGlobalUni(System.Char*,System.Int32)
static RtResultVoid string_to_hglobal_uni_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                  interp::RtStackObject* ret) noexcept
{
    const Utf16Char* chars = EvalStackOp::get_param<const Utf16Char*>(params, 0);
    int32_t len = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, ptr, SystemRuntimeInteropServicesMarshal::string_to_hglobal_uni(chars, len));
    EvalStackOp::set_return(ret, ptr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::BufferToBSTR(System.Char*,System.Int32)
static RtResultVoid buffer_to_bstr_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    const Utf16Char* chars = EvalStackOp::get_param<const Utf16Char*>(params, 0);
    int32_t len = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, ptr, SystemRuntimeInteropServicesMarshal::buffer_to_bstr(chars, len));
    EvalStackOp::set_return(ret, ptr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::FreeBSTR(System.IntPtr)
static RtResultVoid free_bstr_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* /*ret*/) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    return SystemRuntimeInteropServicesMarshal::free_bstr(ptr);
}

// Structure marshaling invokers
/// @icall: System.Runtime.InteropServices.Marshal::PtrToStructure(System.IntPtr,System.Object)
static RtResultVoid ptr_to_structure_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* /*ret*/) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    return SystemRuntimeInteropServicesMarshal::ptr_to_structure(ptr, obj);
}

/// @icall: System.Runtime.InteropServices.Marshal::PtrToStructure(System.IntPtr,System.Type)
static RtResultVoid ptr_to_structure_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                  interp::RtStackObject* ret) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    vm::RtObject* type_obj = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj,
                                            SystemRuntimeInteropServicesMarshal::ptr_to_structure_type(ptr, reinterpret_cast<vm::RtReflectionType*>(type_obj)));
    EvalStackOp::set_return(ret, obj);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::StructureToPtr(System.Object,System.IntPtr,System.Boolean)
static RtResultVoid structure_to_ptr_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* /*ret*/) noexcept
{
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    void* ptr = EvalStackOp::get_param<void*>(params, 1);
    int32_t delete_old = EvalStackOp::get_param<int32_t>(params, 2);
    return SystemRuntimeInteropServicesMarshal::structure_to_ptr(obj, ptr, delete_old);
}

/// @icall: System.Runtime.InteropServices.Marshal::DestroyStructure(System.IntPtr,System.Type)
static RtResultVoid destroy_structure_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                              interp::RtStackObject* /*ret*/) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    vm::RtReflectionType* ref_type = EvalStackOp::get_param<vm::RtReflectionType*>(params, 1);
    return SystemRuntimeInteropServicesMarshal::destroy_structure(ptr, ref_type);
}

// Type operations invokers
/// @icall: System.Runtime.InteropServices.Marshal::SizeOf(System.Type)
static RtResultVoid sizeof_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionType* ref_type = EvalStackOp::get_param<vm::RtReflectionType*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, size, SystemRuntimeInteropServicesMarshal::sizeof_type(ref_type));
    EvalStackOp::set_return(ret, size);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::OffsetOf(System.Type,System.String)
static RtResultVoid offset_of_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionType* ref_type = EvalStackOp::get_param<vm::RtReflectionType*>(params, 0);
    vm::RtString* field_name = EvalStackOp::get_param<vm::RtString*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, offset, SystemRuntimeInteropServicesMarshal::offset_of(ref_type, field_name));
    EvalStackOp::set_return(ret, offset);
    RET_VOID_OK();
}

// Array operations invokers
/// @icall: System.Runtime.InteropServices.Marshal::UnsafeAddrOfPinnedArrayElement(System.Array,System.Int32)
static RtResultVoid unsafe_addr_of_pinned_array_element_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtArray* arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    int32_t index = EvalStackOp::get_param<int32_t>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, addr, SystemRuntimeInteropServicesMarshal::unsafe_addr_of_pinned_array_element(arr, index));
    EvalStackOp::set_return(ret, addr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::copy_to_unmanaged_fixed(System.Array,System.Int32,System.IntPtr,System.Int32,System.Void*)
static RtResultVoid copy_to_unmanaged_fixed_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                    interp::RtStackObject* /*ret*/) noexcept
{
    vm::RtArray* arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    int32_t start_index = EvalStackOp::get_param<int32_t>(params, 1);
    void* dest = EvalStackOp::get_param<void*>(params, 2);
    int32_t length = EvalStackOp::get_param<int32_t>(params, 3);
    void* etype = EvalStackOp::get_param<void*>(params, 4);
    return SystemRuntimeInteropServicesMarshal::copy_to_unmanaged_fixed(arr, start_index, dest, length, etype);
}

/// @icall: System.Runtime.InteropServices.Marshal::copy_from_unmanaged_fixed(System.IntPtr,System.Int32,System.Array,System.Int32,System.Void*)
static RtResultVoid copy_from_unmanaged_fixed_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                      interp::RtStackObject* /*ret*/) noexcept
{
    void* src = EvalStackOp::get_param<void*>(params, 0);
    int32_t start_index = EvalStackOp::get_param<int32_t>(params, 1);
    vm::RtArray* arr = EvalStackOp::get_param<vm::RtArray*>(params, 2);
    int32_t length = EvalStackOp::get_param<int32_t>(params, 3);
    void* etype = EvalStackOp::get_param<void*>(params, 4);
    return SystemRuntimeInteropServicesMarshal::copy_from_unmanaged_fixed(src, start_index, arr, length, etype);
}

// Delegate marshaling invokers
/// @icall: System.Runtime.InteropServices.Marshal::GetDelegateForFunctionPointerInternal(System.IntPtr,System.Type)
static RtResultVoid get_delegate_for_function_pointer_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    vm::RtReflectionType* ref_type = EvalStackOp::get_param<vm::RtReflectionType*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtDelegate*, delegate,
                                            SystemRuntimeInteropServicesMarshal::get_delegate_for_function_pointer_internal(ptr, ref_type));
    EvalStackOp::set_return(ret, delegate);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::GetFunctionPointerForDelegateInternal(System.Delegate)
static RtResultVoid get_function_pointer_for_delegate_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtDelegate* delegate = EvalStackOp::get_param<vm::RtDelegate*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, ptr, SystemRuntimeInteropServicesMarshal::get_function_pointer_for_delegate_internal(delegate));
    EvalStackOp::set_return(ret, ptr);
    RET_VOID_OK();
}

// Win32 error invokers
/// @icall: System.Runtime.InteropServices.Marshal::GetLastWin32Error()
static RtResultVoid get_last_win32_error_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* /*params*/,
                                                 interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, error, SystemRuntimeInteropServicesMarshal::get_last_win32_error());
    EvalStackOp::set_return(ret, error);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::SetLastWin32Error(System.Int32)
static RtResultVoid set_last_win32_error_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* /*ret*/) noexcept
{
    int32_t error = EvalStackOp::get_param<int32_t>(params, 0);
    return SystemRuntimeInteropServicesMarshal::set_last_win32_error(error);
}

// COM/WinRT stub invokers
/// @icall: System.Runtime.InteropServices.Marshal::QueryInterfaceInternal(System.IntPtr,System.Guid&,System.IntPtr&)
static RtResultVoid query_interface_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                     interp::RtStackObject* ret) noexcept
{
    void* pUnk = EvalStackOp::get_param<void*>(params, 0);
    void* iid = EvalStackOp::get_param<void*>(params, 1);
    void* ppv = EvalStackOp::get_param<void*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, SystemRuntimeInteropServicesMarshal::query_interface_internal(pUnk, iid, ppv));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::ReleaseInternal(System.IntPtr)
static RtResultVoid release_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    void* pUnk = EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, SystemRuntimeInteropServicesMarshal::release_internal(pUnk));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::ReleaseComObjectInternal(System.Object)
static RtResultVoid release_com_object_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                        interp::RtStackObject* ret) noexcept
{
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, refcount, SystemRuntimeInteropServicesMarshal::release_com_object_internal(obj));
    EvalStackOp::set_return(ret, refcount);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::GetRawIUnknownForComObjectNoAddRef(System.Object)
static RtResultVoid get_raw_iunknown_for_com_object_no_add_ref_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, ptr, SystemRuntimeInteropServicesMarshal::get_raw_iunknown_for_com_object_no_add_ref(obj));
    EvalStackOp::set_return(ret, ptr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::GetHRForException_WinRT(System.Exception)
static RtResultVoid get_hr_for_exception_winrt_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                       interp::RtStackObject* ret) noexcept
{
    vm::RtObject* exception = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hr, SystemRuntimeInteropServicesMarshal::get_hr_for_exception_winrt(exception));
    EvalStackOp::set_return(ret, hr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::GetNativeActivationFactory(System.Type)
static RtResultVoid get_native_activation_factory_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                          interp::RtStackObject* ret) noexcept
{
    vm::RtObject* type_obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, factory, SystemRuntimeInteropServicesMarshal::get_native_activation_factory(type_obj));
    EvalStackOp::set_return(ret, factory);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::AddRefInternal(System.IntPtr)
static RtResultVoid add_ref_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, refcount, SystemRuntimeInteropServicesMarshal::add_ref_internal(ptr));
    EvalStackOp::set_return(ret, refcount);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::GetIUnknownForObjectInternal(System.Object)
static RtResultVoid get_iunknown_for_object_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, ptr, SystemRuntimeInteropServicesMarshal::get_iunknown_for_object_internal(obj));
    EvalStackOp::set_return(ret, ptr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::GetIDispatchForObjectInternal(System.Object)
static RtResultVoid get_idispatch_for_object_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, ptr, SystemRuntimeInteropServicesMarshal::get_idispatch_for_object_internal(obj));
    EvalStackOp::set_return(ret, ptr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::GetCCW(System.Object,System.Type)
static RtResultVoid get_ccw_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                    interp::RtStackObject* ret) noexcept
{
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    vm::RtObject* type_obj = EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, ptr, SystemRuntimeInteropServicesMarshal::get_ccw(obj, type_obj));
    EvalStackOp::set_return(ret, ptr);
    RET_VOID_OK();
}

/// @icall: System.Runtime.InteropServices.Marshal::GetObjectForCCW(System.IntPtr)
static RtResultVoid get_object_for_ccw_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, SystemRuntimeInteropServicesMarshal::get_object_for_ccw(ptr));
    EvalStackOp::set_return(ret, obj);
    RET_VOID_OK();
}

// Prelink stub invokers
/// @icall: System.Runtime.InteropServices.Marshal::PrelinkAll(System.Type)
static RtResultVoid prelink_all_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                        interp::RtStackObject* /*ret*/) noexcept
{
    vm::RtReflectionType* ref_type = EvalStackOp::get_param<vm::RtReflectionType*>(params, 0);
    return SystemRuntimeInteropServicesMarshal::prelink_all(ref_type);
}

/// @icall: System.Runtime.InteropServices.Marshal::Prelink(System.Reflection.MethodInfo)
static RtResultVoid prelink_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                    interp::RtStackObject* /*ret*/) noexcept
{
    vm::RtObject* method_info = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    return SystemRuntimeInteropServicesMarshal::prelink(method_info);
}

// ========== Registration ==========

static vm::InternalCallEntry s_internal_call_entries_system_runtime_interopservices_marshal[] = {
    // Memory allocation
    {"System.Runtime.InteropServices.Marshal::AllocHGlobal(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::alloc_hglobal,
     alloc_hglobal_invoker},
    {"System.Runtime.InteropServices.Marshal::ReAllocHGlobal(System.IntPtr,System.IntPtr)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::re_alloc_hglobal, re_alloc_hglobal_invoker},
    {"System.Runtime.InteropServices.Marshal::FreeHGlobal(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::free_hglobal,
     free_hglobal_invoker},
    {"System.Runtime.InteropServices.Marshal::AllocCoTaskMem(System.Int32)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::alloc_co_task_mem,
     alloc_co_task_mem_invoker},
    {"System.Runtime.InteropServices.Marshal::ReAllocCoTaskMem(System.IntPtr,System.Int32)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::re_alloc_co_task_mem, re_alloc_co_task_mem_invoker},
    {"System.Runtime.InteropServices.Marshal::FreeCoTaskMem(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::free_co_task_mem,
     free_co_task_mem_invoker},
    {"System.Runtime.InteropServices.Marshal::AllocCoTaskMemSize(System.UIntPtr)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::alloc_co_task_mem_size, alloc_co_task_mem_size_invoker},

    // String conversion
    {"System.Runtime.InteropServices.Marshal::PtrToStringAnsi(System.IntPtr)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::ptr_to_string_ansi, ptr_to_string_ansi_invoker},
    {"System.Runtime.InteropServices.Marshal::PtrToStringAnsi(System.IntPtr,System.Int32)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::ptr_to_string_ansi_len, ptr_to_string_ansi_len_invoker},
    {"System.Runtime.InteropServices.Marshal::PtrToStringUni(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::ptr_to_string_uni,
     ptr_to_string_uni_invoker},
    {"System.Runtime.InteropServices.Marshal::PtrToStringUni(System.IntPtr,System.Int32)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::ptr_to_string_uni_len, ptr_to_string_uni_len_invoker},
    {"System.Runtime.InteropServices.Marshal::PtrToStringBSTR(System.IntPtr)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::ptr_to_string_bstr, ptr_to_string_bstr_invoker},
    {"System.Runtime.InteropServices.Marshal::StringToHGlobalAnsi(System.Char*,System.Int32)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::string_to_hglobal_ansi, string_to_hglobal_ansi_invoker},
    {"System.Runtime.InteropServices.Marshal::StringToHGlobalUni(System.Char*,System.Int32)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::string_to_hglobal_uni, string_to_hglobal_uni_invoker},
    {"System.Runtime.InteropServices.Marshal::BufferToBSTR(System.Char*,System.Int32)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::buffer_to_bstr, buffer_to_bstr_invoker},
    {"System.Runtime.InteropServices.Marshal::FreeBSTR(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::free_bstr,
     free_bstr_invoker},

    // Structure marshaling
    {"System.Runtime.InteropServices.Marshal::PtrToStructure(System.IntPtr,System.Object)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::ptr_to_structure, ptr_to_structure_invoker},
    {"System.Runtime.InteropServices.Marshal::PtrToStructure(System.IntPtr,System.Type)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::ptr_to_structure_type, ptr_to_structure_type_invoker},
    {"System.Runtime.InteropServices.Marshal::StructureToPtr(System.Object,System.IntPtr,System.Boolean)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::structure_to_ptr, structure_to_ptr_invoker},
    {"System.Runtime.InteropServices.Marshal::DestroyStructure(System.IntPtr,System.Type)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::destroy_structure, destroy_structure_invoker},

    // Type operations
    {"System.Runtime.InteropServices.Marshal::SizeOf(System.Type)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::sizeof_type,
     sizeof_type_invoker},
    {"System.Runtime.InteropServices.Marshal::OffsetOf(System.Type,System.String)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::offset_of,
     offset_of_invoker},

    // Array operations
    {"System.Runtime.InteropServices.Marshal::UnsafeAddrOfPinnedArrayElement(System.Array,System.Int32)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::unsafe_addr_of_pinned_array_element, unsafe_addr_of_pinned_array_element_invoker},
    {"System.Runtime.InteropServices.Marshal::copy_to_unmanaged_fixed(System.Array,System.Int32,System.IntPtr,System.Int32,System.Void*)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::copy_to_unmanaged_fixed, copy_to_unmanaged_fixed_invoker},
    {"System.Runtime.InteropServices.Marshal::copy_from_unmanaged_fixed(System.IntPtr,System.Int32,System.Array,System.Int32,System.Void*)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::copy_from_unmanaged_fixed, copy_from_unmanaged_fixed_invoker},

    // Delegate marshaling
    {"System.Runtime.InteropServices.Marshal::GetDelegateForFunctionPointerInternal(System.IntPtr,System.Type)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::get_delegate_for_function_pointer_internal,
     get_delegate_for_function_pointer_internal_invoker},
    {"System.Runtime.InteropServices.Marshal::GetFunctionPointerForDelegateInternal(System.Delegate)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::get_function_pointer_for_delegate_internal,
     get_function_pointer_for_delegate_internal_invoker},

    // Win32 error
    {"System.Runtime.InteropServices.Marshal::GetLastWin32Error()", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::get_last_win32_error,
     get_last_win32_error_invoker},
    {"System.Runtime.InteropServices.Marshal::SetLastWin32Error(System.Int32)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::set_last_win32_error, set_last_win32_error_invoker},

    // COM/WinRT stubs
    {"System.Runtime.InteropServices.Marshal::QueryInterfaceInternal(System.IntPtr,System.Guid&,System.IntPtr&)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::query_interface_internal, query_interface_internal_invoker},
    {"System.Runtime.InteropServices.Marshal::ReleaseInternal(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::release_internal,
     release_internal_invoker},
    {"System.Runtime.InteropServices.Marshal::ReleaseComObjectInternal(System.Object)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::release_com_object_internal, release_com_object_internal_invoker},
    {"System.Runtime.InteropServices.Marshal::GetRawIUnknownForComObjectNoAddRef(System.Object)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::get_raw_iunknown_for_com_object_no_add_ref,
     get_raw_iunknown_for_com_object_no_add_ref_invoker},
    {"System.Runtime.InteropServices.Marshal::GetHRForException_WinRT(System.Exception)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::get_hr_for_exception_winrt, get_hr_for_exception_winrt_invoker},
    {"System.Runtime.InteropServices.Marshal::GetNativeActivationFactory(System.Type)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::get_native_activation_factory, get_native_activation_factory_invoker},
    {"System.Runtime.InteropServices.Marshal::AddRefInternal(System.IntPtr)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::add_ref_internal,
     add_ref_internal_invoker},
    {"System.Runtime.InteropServices.Marshal::GetIUnknownForObjectInternal(System.Object)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::get_iunknown_for_object_internal, get_iunknown_for_object_internal_invoker},
    {"System.Runtime.InteropServices.Marshal::GetIDispatchForObjectInternal(System.Object)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::get_idispatch_for_object_internal, get_idispatch_for_object_internal_invoker},
    {"System.Runtime.InteropServices.Marshal::GetCCW(System.Object,System.Type)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::get_ccw,
     get_ccw_invoker},
    {"System.Runtime.InteropServices.Marshal::GetObjectForCCW(System.IntPtr)",
     (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::get_object_for_ccw, get_object_for_ccw_invoker},

    // Prelink stubs
    {"System.Runtime.InteropServices.Marshal::PrelinkAll(System.Type)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::prelink_all,
     prelink_all_invoker},
    {"System.Runtime.InteropServices.Marshal::Prelink(System.Reflection.MethodInfo)", (vm::InternalCallFunction)&SystemRuntimeInteropServicesMarshal::prelink,
     prelink_invoker},
};

utils::Span<vm::InternalCallEntry> SystemRuntimeInteropServicesMarshal::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_runtime_interopservices_marshal,
                                              sizeof(s_internal_call_entries_system_runtime_interopservices_marshal) /
                                                  sizeof(s_internal_call_entries_system_runtime_interopservices_marshal[0]));
}

} // namespace icalls
} // namespace leanclr

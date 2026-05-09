#pragma once

#include <cstdlib>
#include <limits>
#include "vm/rt_managed_types.h"
#include "vm/rt_exception.h"
#include "vm/method.h"
#include "vm/class.h"
#include "vm/object.h"
#include "vm/field.h"
#include "vm/runtime.h"
#include "vm/rt_array.h"
#include "vm/delegate.h"
#include "vm/reflection.h"
#include "vm/internal_calls.h"
#include "vm/pinvoke.h"
#include "vm/rt_string.h"
#include "vm/marshal.h"
#include "metadata/module_def.h"
#include "interp/interp_defs.h"
#include "interp/execution_helper.h"

#define LEANCLR_CODEGEN_DEBUG LEANCLR_DEBUG

#define LEANCLR_CODEGEN_THROW_ON_ERROR(retExpr, methodInfo, ip)                                          \
    do                                                                                                   \
    {                                                                                                    \
        auto&& __result = (retExpr);                                                                     \
        if (__result.is_err())                                                                           \
        {                                                                                                \
            leanclr::vm::Exception::raise_aot_error_as_exception(__result.unwrap_err(), methodInfo, ip); \
            return leanclr::RtErr::ManagedException;                                                     \
        }                                                                                                \
    } while (0)

#define LEANCLR_CODEGEN_ASSIGN_OR_THROW_ON_ERROR(retVar, retExpr, methodInfo, ip)                        \
    do                                                                                                   \
    {                                                                                                    \
        auto&& __result = (retExpr);                                                                     \
        if (__result.is_err())                                                                           \
        {                                                                                                \
            leanclr::vm::Exception::raise_aot_error_as_exception(__result.unwrap_err(), methodInfo, ip); \
            return leanclr::RtErr::ManagedException;                                                     \
        }                                                                                                \
        retVar = (decltype(retVar))__result.unwrap();                                                    \
    } while (0)

#define LEANCLR_CODEGEN_AUTO_DECLARING_ASSIGN_OR_THROW_ON_ERROR(retVar, retExpr, methodInfo, ip)         \
    decltype((retExpr).unwrap()) retVar;                                                                 \
    do                                                                                                   \
    {                                                                                                    \
        auto&& __result = (retExpr);                                                                     \
        if (__result.is_err())                                                                           \
        {                                                                                                \
            leanclr::vm::Exception::raise_aot_error_as_exception(__result.unwrap_err(), methodInfo, ip); \
            return leanclr::RtErr::ManagedException;                                                     \
        }                                                                                                \
        retVar = __result.unwrap();                                                                      \
    } while (0)

#define LEANCLR_CODEGEN_DECLARING_ASSIGN_OR_THROW_ON_ERROR(retType, retVar, retExpr, methodInfo, ip)     \
    retType retVar;                                                                                      \
    do                                                                                                   \
    {                                                                                                    \
        auto&& __result = (retExpr);                                                                     \
        if (__result.is_err())                                                                           \
        {                                                                                                \
            leanclr::vm::Exception::raise_aot_error_as_exception(__result.unwrap_err(), methodInfo, ip); \
            return leanclr::RtErr::ManagedException;                                                     \
        }                                                                                                \
        retVar = (retType)__result.unwrap();                                                             \
    } while (0)

#define LEANCLR_CODEGEN_RETURN(value) return value

#define LEANCLR_CODEGEN_RETURN_ERR(err) return err

#define LEANCLR_CODEGEN_RETURN_VOID() \
    do                                \
    {                                 \
        return leanclr::Unit{};       \
    } while (0)

#define LEANCLR_CODEGEN_THROW_RUNTIME_ERROR(err, methodInfo, ip)                   \
    do                                                                             \
    {                                                                              \
        leanclr::vm::Exception::raise_aot_error_as_exception(err, methodInfo, ip); \
        return leanclr::RtErr::ManagedException;                                   \
    } while (0)

#define LEANCLR_CODEGEN_CHECK_NOT_NULL_OR_THROW_NULL_REFERENCE_EXCEPTION(checkVar, methodInfo, ip)               \
    do                                                                                                           \
    {                                                                                                            \
        if (!(checkVar))                                                                                         \
        {                                                                                                        \
            leanclr::vm::Exception::raise_aot_error_as_exception(leanclr::RtErr::NullReference, methodInfo, ip); \
            return leanclr::RtErr::ManagedException;                                                             \
        }                                                                                                        \
    } while (0)

#define LEANCLR_CODEGEN_THROW_EXCEPTION(ex, methodInfo, ip)                                         \
    do                                                                                              \
    {                                                                                               \
        LEANCLR_CODEGEN_CHECK_NOT_NULL_OR_THROW_NULL_REFERENCE_EXCEPTION(ex, methodInfo, ip);       \
        leanclr::vm::Exception::raise_aot_exception((leanclr::vm::RtException*)ex, methodInfo, ip); \
        return leanclr::RtErr::ManagedException;                                                    \
    } while (0)

#if LEANCLR_FATAL_ON_RAISE_NOT_IMPLEMENTED_ERROR
#define LEANCLR_CODEGEN_RETURN_NOT_IMPLEMENTED_ERROR() LEANCLR_CODEGEN_RETURN(leanclr::fatal_on_not_implemented_error())
#else
#define LEANCLR_CODEGEN_RETURN_NOT_IMPLEMENTED_ERROR() LEANCLR_CODEGEN_RETURN(leanclr::RtErr::NotImplemented)
#endif

namespace leanclr
{
namespace codegen
{

template <typename T>
static T select_arch(T v32, T v64)
{
#if LEANCLR_ARCH_64BIT
    return v64;
#else
    return v32;
#endif
}

using vm::RT_OBJECT_HEADER_SIZE;

inline metadata::RtModuleDef* get_module(const char* module_name)
{
    return metadata::RtModuleDef::find_module(module_name);
}

inline RtResult<vm::RtObject*> new_object(const metadata::RtClass* klass)
{
    return vm::Object::new_object(klass);
}

void* resolve_metadata_token(metadata::RtModuleDef* mod, uint32_t token, const metadata::RtMethodInfo* generic_method_info);
void resolve_metadata_tokens(metadata::RtModuleDef* mod, const uint32_t* tokens, size_t count, void** resolved_metadatas);
void resolve_generic_metadata_tokens(metadata::RtModuleDef* mod, const uint32_t* tokens, size_t count, const metadata::RtMethodInfo* generic_method_info, void** resolved_metadatas);
vm::RtString* resolve_string_literal(metadata::RtModuleDef* mod, uint32_t token);

template <typename ArgType>
void expand_argument_to_eval_stack(const ArgType arg, interp::RtStackObject* ret) noexcept
{
    *(ArgType*)ret = arg;
}

inline void expand_argument_to_eval_stack(const bool arg, interp::RtStackObject* ret) noexcept
{
    *(int32_t*)ret = arg ? 1 : 0;
}

inline void expand_argument_to_eval_stack(const int8_t arg, interp::RtStackObject* ret) noexcept
{
    *(int32_t*)ret = arg;
}

inline void expand_argument_to_eval_stack(const uint8_t arg, interp::RtStackObject* ret) noexcept
{
    *(int32_t*)ret = arg;
}

inline void expand_argument_to_eval_stack(const int16_t arg, interp::RtStackObject* ret) noexcept
{
    *(int32_t*)ret = arg;
}

inline void expand_argument_to_eval_stack(const uint16_t arg, interp::RtStackObject* ret) noexcept
{
    *(int32_t*)ret = arg;
}

inline void expand_argument_to_eval_stack(const int32_t arg, interp::RtStackObject* ret) noexcept
{
    *(int32_t*)ret = arg;
}

inline void expand_argument_to_eval_stack(const uint32_t arg, interp::RtStackObject* ret) noexcept
{
    *(int32_t*)ret = static_cast<int32_t>(arg);
}

template <typename T>
constexpr size_t get_stack_object_size_for_type()
{
    return (sizeof(T) + sizeof(interp::RtStackObject) - 1) / sizeof(interp::RtStackObject);
}

template <typename T>
RtResultVoid set_ret_or_return_error(const RtResult<T>& result, interp::RtStackObject* ret) noexcept
{
    if (result.is_ok())
    {
        expand_argument_to_eval_stack(result.unwrap(), ret);
        RET_VOID_OK();
    }
    else
    {
        return result.unwrap_err();
    }
}

template <typename T>
T get_eval_stack_value_as_type(const interp::RtStackObject* ret) noexcept
{
    return *(T*)ret;
}

inline bool is_cctor_not_finished(const metadata::RtClass* klass)
{
    return vm::Class::is_cctor_not_finished(klass);
}

inline RtResultVoid run_class_static_constructor(const metadata::RtClass* klass)
{
    return vm::Runtime::run_class_static_constructor(klass);
}

inline RtResultVoid invoke_with_run_class_static_constructor(const metadata::RtMethodInfo* method, interp::RtStackObject* arg_buff,
                                                             interp::RtStackObject* ret_buff)
{
    if (vm::Method::is_static(method) && vm::Class::is_cctor_not_finished(method->parent))
    {
        RET_ERR_ON_FAIL(vm::Runtime::run_class_static_constructor(method->parent));
    }
    return vm::Runtime::invoke_stackobject_arguments_without_run_cctor(method, arg_buff, ret_buff);
}

inline RtResultVoid invoke_without_run_class_static_constructor(const metadata::RtMethodInfo* method, interp::RtStackObject* arg_buff,
                                                                interp::RtStackObject* ret_buff)
{
    return vm::Runtime::invoke_stackobject_arguments_without_run_cctor(method, arg_buff, ret_buff);
}

inline RtResultVoid virtual_invoke_without_run_class_static_constructor(const metadata::RtMethodInfo* method, interp::RtStackObject* arg_buff,
                                                                        interp::RtStackObject* ret_buff)
{
    return vm::Runtime::virtual_invoke_stackobject_arguments_without_run_cctor(method, arg_buff, ret_buff);
}

inline RtResult<const metadata::RtMethodInfo*> get_virtual_method_impl(vm::RtObject* obj, const metadata::RtMethodInfo* virtual_method)
{
    return vm::Method::get_virtual_method_impl(obj, virtual_method);
}

inline vm::RtObject* is_inst(vm::RtObject* obj, const metadata::RtClass* klass)
{
    return vm::Object::is_inst(obj, klass);
}

inline bool is_assignable_from(const metadata::RtClass* fromClass, const metadata::RtClass* toClass)
{
    return vm::Class::is_assignable_from(fromClass, toClass);
}

inline vm::RtObject* cast_class(vm::RtObject* obj, const metadata::RtClass* klass)
{
    return vm::Object::cast_class(obj, klass);
}

inline RtResult<vm::RtObject*> box_object(const metadata::RtClass* klass, const void* value)
{
    return vm::Object::box_object(klass, value);
}

inline RtResultVoid unbox_any(const vm::RtObject* obj, const metadata::RtClass* klass, void* dst, bool extend_to_stack)
{
    return vm::Object::unbox_any(obj, klass, dst, extend_to_stack);
}

// Unbox with exact type checking
inline RtResult<const void*> unbox_ex(const vm::RtObject* obj, const metadata::RtClass* unbox_class)
{
    return vm::Object::unbox_ex(obj, unbox_class);
}

inline bool is_value_type(const metadata::RtClass* klass)
{
    return vm::Class::is_value_type(klass);
}

inline RtResult<vm::RtArray*> new_szarray_from_ele_class(const metadata::RtClass* ele_class, int32_t length)
{
    return vm::Array::new_szarray_from_ele_klass(const_cast<metadata::RtClass*>(ele_class), length);
}

inline RtResult<vm::RtArray*> new_szarray_from_array_class(const metadata::RtClass* klass, int32_t length)
{
    return vm::Array::new_szarray_from_array_klass(const_cast<metadata::RtClass*>(klass), length);
}

inline RtResult<vm::RtArray*> new_mdarray_from_array_class(const metadata::RtClass* arr_klass, const int32_t* lengths, const int32_t* lower_bounds)
{
    return vm::Array::new_mdarray_from_array_klass(const_cast<metadata::RtClass*>(arr_klass), lengths, lower_bounds);
}

inline RtResult<vm::RtArray*> new_mdarray_from_ele_class(const metadata::RtClass* ele_klass, int32_t rank, const int32_t* lengths, const int32_t* lower_bounds)
{
    return vm::Array::new_mdarray_from_ele_klass(const_cast<metadata::RtClass*>(ele_klass), rank, lengths, lower_bounds);
}

inline int32_t get_array_length(const vm::RtArray* array)
{
    return vm::Array::get_array_length(array);
}

inline metadata::RtClass* get_array_element_class(const vm::RtArray* array)
{
    return const_cast<metadata::RtClass*>(vm::Array::get_array_element_class(array));
}

inline bool is_array_index_out_of_range(const vm::RtArray* array, int32_t index)
{
    return vm::Array::is_out_of_range(array, index);
}

inline bool is_pointer_element_compatible_with(const metadata::RtClass* fromClass, const metadata::RtClass* toClass)
{
    return vm::Class::is_pointer_element_compatible_with(fromClass, toClass);
}

template <typename T>
inline T* get_array_element_data_start_as(vm::RtArray* array)
{
    return vm::Array::get_array_data_start_as<T>(array);
}

template <typename T>
inline T get_array_element_data_at(vm::RtArray* array, int32_t index)
{
    return vm::Array::get_array_data_at<T>(array, index);
}

template <typename T>
inline T* get_array_element_address(vm::RtArray* array, int32_t index)
{
    return vm::Array::get_array_element_address<T>(array, index);
}

template <typename T>
inline void set_array_element_data_at(vm::RtArray* array, int32_t index, T value)
{
    vm::Array::set_array_data_at<T>(array, index, value);
}

inline RtResult<int32_t> get_mdarray_global_index_from_indices(vm::RtArray* arr, int32_t* indices)
{
    return vm::Array::get_mdarray_global_index_from_indices3(arr, indices);
}

inline RtResult<vm::RtMulticastDelegate*> new_delegate(const metadata::RtClass* delelgate_type, vm::RtObject* target, const metadata::RtMethodInfo* method)
{
    return vm::Delegate::new_delegate(delelgate_type, target, method);
}

inline RtResult<const uint8_t*> get_field_rva_data(const metadata::RtFieldInfo* field)
{
    return vm::Field::get_field_rva_data(field);
}

inline uint32_t get_field_offset_includes_object_header(const metadata::RtFieldInfo* field)
{
    return vm::Field::get_field_offset_includes_object_header_for_all_type(field);
}

inline uint32_t get_field_offset_includes_object_header_for_reference_type(const metadata::RtFieldInfo* field)
{
    return vm::Field::get_field_offset_includes_object_header_for_reference_type(field);
}

inline uint32_t get_class_instance_size_with_object_header(const metadata::RtClass* klass)
{
    return vm::Class::get_instance_size_with_object_header(klass);
}

inline uint32_t get_class_instance_size_without_object_header(const metadata::RtClass* klass)
{
    return vm::Class::get_instance_size_without_object_header(klass);
}

inline uint32_t get_class_field_count(const metadata::RtClass* klass)
{
    return klass->field_count;
}

inline uint32_t get_class_static_size(const metadata::RtClass* klass)
{
    return klass->static_size;
}

inline RtResult<vm::RtReflectionAssembly*> get_assembly_reflection_object(const metadata::RtModuleDef* mod)
{
    return vm::Reflection::get_assembly_reflection_object(mod->get_assembly());
}

inline RtResult<vm::RtReflectionMethod*> get_method_reflection_object(const metadata::RtMethodInfo* method)
{
    return vm::Reflection::get_method_reflection_object(method, method->parent);
}

template <typename Src, typename Dst>
inline int32_t cast_float_to_small_int(Src value)
{
    return interp::cast_float_to_small_int<Src, Dst>(value);
}

template <typename Src, typename Dst>
inline int32_t cast_float_to_i32(Src value)
{
    return interp::cast_float_to_i32<Src, Dst>(value);
}

template <typename Src, typename Dst>
inline int64_t cast_float_to_i64(Src value)
{
    return interp::cast_float_to_i64<Src, Dst>(value);
}

template <typename Src, typename Dst>
inline intptr_t cast_float_to_intptr(Src value)
{
    return interp::cast_float_to_intptr<Src, Dst>(value);
}

inline vm::InternalCallFunction resolve_internal_call(const char* name)
{
    return (vm::InternalCallFunction)vm::InternalCalls::get_lite_internal_call(name);
}

RtErr raise_internal_call_entry_not_found_error(const char* name);

using vm::PInvokeFunction;
inline PInvokeFunction resolve_pinvoke_function(const char* dll_name_no_ext, const char* function_name)
{
    return vm::PInvokes::get_pinvoke_function(dll_name_no_ext, function_name);
}

RtErr raise_pinvoke_entry_not_found_error(const char* dll_name_no_ext, const char* function_name);

using vm::TempUtf16StringToUtf8Converter;

inline vm::RtString* marshal_utf8_string_to_utf16(const char* str)
{
    return str ? vm::String::create_string_from_utf8cstr(str) : nullptr;
}

inline void free_pinvoke_returned_utf8_cstr(const char* str)
{
    if (str != nullptr)
    {
        std::free(const_cast<char*>(str));
    }
}

} // namespace codegen
} // namespace leanclr

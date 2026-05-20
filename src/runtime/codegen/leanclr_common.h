#pragma once

#include <cstdlib>
#include <cstring>
#include <limits>
#include "vm/rt_managed_types.h"
#include "vm/rt_exception.h"
#include "vm/method.h"
#include "vm/class.h"
#include "vm/object.h"
#include "gc/garbage_collector.h"
#include "gc/gc_config.h"
#include "vm/field.h"
#include "vm/runtime.h"
#include "vm/rt_array.h"
#include "vm/delegate.h"
#include "vm/reflection.h"
#include "vm/internal_calls.h"
#include "vm/pinvoke.h"
#include "vm/rt_string.h"
#include "vm/marshal.h"
#include "vmutils/stringbuilder.h"
#include "metadata/module_def.h"
#include "interp/interp_defs.h"
#include "interp/execution_helper.h"
#include "utils/string_builder.h"

#define LEANCLR_CODEGEN_DEBUG LEANCLR_DEBUG

#define LEANCLR_CODEGEN_LIKELY(expr) LEANCLR_LIKELY(expr)
#define LEANCLR_CODEGEN_UNLIKELY(expr) LEANCLR_UNLIKELY(expr)

#define LEANCLR_CODEGEN_ASSUME(expr) LEANCLR_ASSUME(expr)
#define LEANCLR_CODEGEN_ASSUME_NOT_NULL(ptr) LEANCLR_ASSUME_NOT_NULL(ptr)

#define LEANCLR_CODEGEN_THROW_ON_ERROR(retExpr, methodInfo, ip)                                          \
    do                                                                                                   \
    {                                                                                                    \
        auto&& __result = (retExpr);                                                                     \
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                                                 \
        {                                                                                                \
            leanclr::vm::Exception::raise_aot_error_as_exception(__result.unwrap_err(), methodInfo, ip); \
            return leanclr::RtErr::ManagedException;                                                     \
        }                                                                                                \
    } while (0)

#define LEANCLR_CODEGEN_ASSIGN_OR_THROW_ON_ERROR(retVar, retExpr, methodInfo, ip)                        \
    do                                                                                                   \
    {                                                                                                    \
        auto&& __result = (retExpr);                                                                     \
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                                                 \
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
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                                                 \
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
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                                                 \
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
        if (LEANCLR_CODEGEN_UNLIKELY(!(checkVar)))                                                               \
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

#define LEANCLR_CODEGEN_THROW_ON_ERROR(retExpr, methodInfo, ip)                                          \
    do                                                                                                   \
    {                                                                                                    \
        auto&& __result = (retExpr);                                                                     \
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                                                 \
        {                                                                                                \
            leanclr::vm::Exception::raise_aot_error_as_exception(__result.unwrap_err(), methodInfo, ip); \
            return leanclr::RtErr::ManagedException;                                                     \
        }                                                                                                \
    } while (0)

#define LEANCLR_CODEGEN_ASSIGN_OR_THROW_ON_ERROR(retVar, retExpr, methodInfo, ip)                        \
    do                                                                                                   \
    {                                                                                                    \
        auto&& __result = (retExpr);                                                                     \
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                                                 \
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
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                                                 \
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
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                                                 \
        {                                                                                                \
            leanclr::vm::Exception::raise_aot_error_as_exception(__result.unwrap_err(), methodInfo, ip); \
            return leanclr::RtErr::ManagedException;                                                     \
        }                                                                                                \
        retVar = (retType)__result.unwrap();                                                             \
    } while (0)

#define LEANCLR_CODEGEN_DECLARING_ASSIGN_OR_THROW_ON_ERROR_WITHOUT_IP(retType, retVar, retExpr, methodInfo) \
    retType retVar;                                                                                         \
    do                                                                                                      \
    {                                                                                                       \
        auto&& __result = (retExpr);                                                                        \
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                                                    \
        {                                                                                                   \
            leanclr::vm::Exception::raise_aot_error_as_exception(__result.unwrap_err(), methodInfo, 0);     \
            return leanclr::RtErr::ManagedException;                                                        \
        }                                                                                                   \
        retVar = (retType)__result.unwrap();                                                                \
    } while (0)

#if LEANCLR_FATAL_ON_RAISE_NOT_IMPLEMENTED_ERROR
#define LEANCLR_CODEGEN_RETURN_NOT_IMPLEMENTED_ERROR() LEANCLR_CODEGEN_RETURN(leanclr::fatal_on_not_implemented_error())
#else
#define LEANCLR_CODEGEN_RETURN_NOT_IMPLEMENTED_ERROR() LEANCLR_CODEGEN_RETURN(leanclr::RtErr::NotImplemented)
#endif

#define LEANCLR_CODEGEN_ABORT_WHEN_RAISE_EXCEPTION_IN_MONO_PINVOKE_CALLBACK() leanclr::fatal_on_not_implemented_error()

#define LEANCLR_CODEGEN_DECLARING_ASSIGN_OR_ABORT_ON_ERROR(retType, retVar, retExpr, methodInfo) \
    retType retVar;                                                                              \
    do                                                                                           \
    {                                                                                            \
        auto&& __result = (retExpr);                                                             \
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                                         \
        {                                                                                        \
            LEANCLR_CODEGEN_ABORT_WHEN_RAISE_EXCEPTION_IN_MONO_PINVOKE_CALLBACK();               \
        }                                                                                        \
        retVar = (retType)__result.unwrap();                                                     \
    } while (0)

#define LEANCLR_CODEGEN_GOTO_HANDLE_RETURN_VALUE() goto ___label_handle_return_value
#define LEANCLR_CODEGEN_GOTO_HANDLE_RETURN_ERROR() goto ___label_handle_return_error

#define LEANCLR_CODEGEN_GOTO_RETURN(value)          \
    do                                              \
    {                                               \
        ___ret = decltype(___ret)(value);           \
        LEANCLR_CODEGEN_GOTO_HANDLE_RETURN_VALUE(); \
    } while (0)

#define LEANCLR_CODEGEN_GOTO_RETURN_ERR(err)        \
    do                                              \
    {                                               \
        ___ret_err = err;                           \
        LEANCLR_CODEGEN_GOTO_HANDLE_RETURN_ERROR(); \
    } while (0)

#define LEANCLR_CODEGEN_GOTO_RETURN_VOID()          \
    do                                              \
    {                                               \
        LEANCLR_CODEGEN_GOTO_HANDLE_RETURN_VALUE(); \
    } while (0)

#define LEANCLR_CODEGEN_GOTO_THROW_RUNTIME_ERROR(err, methodInfo, ip) \
    do                                                                \
    {                                                                 \
        ___ret_err = err;                                             \
        LEANCLR_CODEGEN_GOTO_HANDLE_RETURN_ERROR();                   \
    } while (0)

#define LEANCLR_CODEGEN_GOTO_CHECK_NOT_NULL_OR_THROW_NULL_REFERENCE_EXCEPTION(checkVar, methodInfo, ip) \
    do                                                                                                  \
    {                                                                                                   \
        if (LEANCLR_CODEGEN_UNLIKELY(!(checkVar)))                                                      \
        {                                                                                               \
            ___ret_err = leanclr::RtErr::NullReference;                                                 \
            LEANCLR_CODEGEN_GOTO_HANDLE_RETURN_ERROR();                                                 \
        }                                                                                               \
    } while (0)

#define LEANCLR_CODEGEN_GOTO_THROW_ON_ERROR(retExpr, methodInfo, ip) \
    do                                                               \
    {                                                                \
        auto&& __result = (retExpr);                                 \
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))             \
        {                                                            \
            ___ret_err = __result.unwrap_err();                      \
            LEANCLR_CODEGEN_GOTO_HANDLE_RETURN_ERROR();              \
        }                                                            \
    } while (0)

#define LEANCLR_CODEGEN_GOTO_ASSIGN_OR_THROW_ON_ERROR(retVar, retExpr, methodInfo, ip) \
    do                                                                                 \
    {                                                                                  \
        auto&& __result = (retExpr);                                                   \
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                               \
        {                                                                              \
            ___ret_err = __result.unwrap_err();                                        \
            LEANCLR_CODEGEN_GOTO_HANDLE_RETURN_ERROR();                                \
        }                                                                              \
        retVar = (decltype(retVar))__result.unwrap();                                  \
    } while (0)

#define LEANCLR_CODEGEN_GOTO_AUTO_DECLARING_ASSIGN_OR_THROW_ON_ERROR(retVar, retExpr, methodInfo, ip) \
    decltype((retExpr).unwrap()) retVar;                                                              \
    do                                                                                                \
    {                                                                                                 \
        auto&& __result = (retExpr);                                                                  \
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                                              \
        {                                                                                             \
            ___ret_err = __result.unwrap_err();                                                       \
            LEANCLR_CODEGEN_GOTO_HANDLE_RETURN_ERROR();                                               \
        }                                                                                             \
        retVar = __result.unwrap();                                                                   \
    } while (0)

#define LEANCLR_CODEGEN_GOTO_DECLARING_ASSIGN_OR_THROW_ON_ERROR(retType, retVar, retExpr, methodInfo, ip) \
    retType retVar;                                                                                       \
    do                                                                                                    \
    {                                                                                                     \
        auto&& __result = (retExpr);                                                                      \
        if (LEANCLR_CODEGEN_UNLIKELY(__result.is_err()))                                                  \
        {                                                                                                 \
            ___ret_err = __result.unwrap_err();                                                           \
            LEANCLR_CODEGEN_GOTO_HANDLE_RETURN_ERROR();                                                   \
        }                                                                                                 \
        retVar = (retType)__result.unwrap();                                                              \
    } while (0)

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

#define LEANCLR_CODEGEN_NEWOBJ(klass, managed_method) LEANCLR_NEWOBJ(klass, ::leanclr::gc::GcAllocSite::make_codegen(__FILE__, __LINE__, managed_method))
#define LEANCLR_CODEGEN_BOX_OBJECT(klass, value, managed_method) LEANCLR_BOX_OBJECT(klass, value, ::leanclr::gc::GcAllocSite::make_codegen(__FILE__, __LINE__, managed_method))

#define LEANCLR_CODEGEN_NEW_EMPTY_SZARRAY_BY_ELE_KLASS(arr_klass, managed_method) LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS(arr_klass, ::leanclr::gc::GcAllocSite::make_codegen(__FILE__, __LINE__, managed_method))
#define LEANCLR_CODEGEN_NEW_SZARRAY_FROM_ARRAY_KLASS(arr_klass, length, managed_method) LEANCLR_NEW_SZARRAY_FROM_ARRAY_KLASS(arr_klass, length, ::leanclr::gc::GcAllocSite::make_codegen(__FILE__, __LINE__, managed_method))
#define LEANCLR_CODEGEN_NEW_SZARRAY_FROM_ELE_KLASS(ele_class, length, managed_method) LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS(ele_class, length, ::leanclr::gc::GcAllocSite::make_codegen(__FILE__, __LINE__, managed_method))
#define LEANCLR_CODEGEN_NEW_MDARRAY_FROM_ARRAY_KLASS(arr_klass, lengths, lower_bounds, managed_method) LEANCLR_NEW_MDARRAY_FROM_ARRAY_KLASS(arr_klass, lengths, lower_bounds, ::leanclr::gc::GcAllocSite::make_codegen(__FILE__, __LINE__, managed_method))
#define LEANCLR_CODEGEN_NEW_MDARRAY_FROM_ELE_KLASS(ele_klass, rank, lengths, lower_bounds, managed_method) LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(ele_klass, rank, lengths, lower_bounds, ::leanclr::gc::GcAllocSite::make_codegen(__FILE__, __LINE__, managed_method))

void* resolve_metadata_token(metadata::RtModuleDef* mod, uint32_t token, const metadata::RtMethodInfo* generic_method_info);
void resolve_metadata_tokens(metadata::RtModuleDef* mod, const uint32_t* tokens, size_t count, void** resolved_metadatas);
void resolve_generic_metadata_tokens(metadata::RtModuleDef* mod, const uint32_t* tokens, size_t count, const metadata::RtMethodInfo* generic_method_info,
                                     void** resolved_metadatas);
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

inline bool is_cctor_not_finished(const metadata::RtClass* klass) noexcept
{
    return vm::Class::is_cctor_not_finished(klass);
}

inline RtResultVoid run_class_static_constructor(const metadata::RtClass* klass) noexcept
{
    return vm::Runtime::run_class_static_constructor(klass);
}

inline bool is_aot_method(const metadata::RtMethodInfo* method) noexcept
{
    return method->invoker_type == metadata::RtInvokerType::Aot;
}

inline RtResultVoid invoke_with_run_class_static_constructor(const metadata::RtMethodInfo* method, interp::RtStackObject* arg_buff,
                                                             interp::RtStackObject* ret_buff) noexcept
{
    if (vm::Method::is_static(method) && vm::Class::is_cctor_not_finished(method->parent))
    {
        RET_ERR_ON_FAIL(vm::Runtime::run_class_static_constructor(method->parent));
    }
    return vm::Runtime::invoke_stackobject_arguments_without_run_cctor(method, arg_buff, ret_buff);
}

inline RtResultVoid invoke_without_run_class_static_constructor(const metadata::RtMethodInfo* method, interp::RtStackObject* arg_buff,
                                                                interp::RtStackObject* ret_buff) noexcept
{
    return vm::Runtime::invoke_stackobject_arguments_without_run_cctor(method, arg_buff, ret_buff);
}

inline RtResultVoid virtual_invoke_without_run_class_static_constructor(const metadata::RtMethodInfo* method, interp::RtStackObject* arg_buff,
                                                                        interp::RtStackObject* ret_buff) noexcept
{
    return vm::Runtime::virtual_invoke_stackobject_arguments_without_run_cctor(method, arg_buff, ret_buff);
}

inline RtResult<const metadata::RtMethodInfo*> get_virtual_method_impl(vm::RtObject* obj, const metadata::RtMethodInfo* virtual_method) noexcept
{
    return vm::Method::get_virtual_method_impl(obj, virtual_method);
}

inline vm::RtObject* is_inst(vm::RtObject* obj, const metadata::RtClass* klass) noexcept
{
    return vm::Object::is_inst(obj, klass);
}

inline bool is_assignable_from(const metadata::RtClass* fromClass, const metadata::RtClass* toClass) noexcept
{
    return vm::Class::is_assignable_from(fromClass, toClass);
}

inline vm::RtObject* cast_class(vm::RtObject* obj, const metadata::RtClass* klass) noexcept
{
    return vm::Object::cast_class(obj, klass);
}

inline RtResultVoid unbox_any(const vm::RtObject* obj, const metadata::RtClass* klass, void* dst, bool extend_to_stack) noexcept
{
    return vm::Object::unbox_any(obj, klass, dst, extend_to_stack);
}

// Unbox with exact type checking
inline RtResult<const void*> unbox_ex(const vm::RtObject* obj, const metadata::RtClass* unbox_class) noexcept
{
    return vm::Object::unbox_ex(obj, unbox_class);
}

inline bool is_value_type(const metadata::RtClass* klass) noexcept
{
    return vm::Class::is_value_type(klass);
}

inline int32_t get_array_length(const vm::RtArray* array) noexcept
{
    return vm::Array::get_array_length(array);
}

inline const metadata::RtClass* get_array_element_class(const vm::RtArray* array) noexcept
{
    return vm::Array::get_array_element_class(array);
}

inline bool is_array_index_out_of_range(const vm::RtArray* array, int32_t index) noexcept
{
    return vm::Array::is_out_of_range(array, index);
}

inline bool is_pointer_element_compatible_with(const metadata::RtClass* fromClass, const metadata::RtClass* toClass) noexcept
{
    return vm::Class::is_pointer_element_compatible_with(fromClass, toClass);
}

template <typename T>
inline T* get_array_element_data_start_as(vm::RtArray* array) noexcept
{
    return vm::Array::get_array_data_start_as<T>(array);
}

template <typename T>
inline T get_array_element_data_at(vm::RtArray* array, int32_t index) noexcept
{
    return vm::Array::get_array_data_at<T>(array, index);
}

template <typename T>
inline T* get_array_element_address(vm::RtArray* array, int32_t index) noexcept
{
    return vm::Array::get_array_element_address<T>(array, index);
}

template <typename T>
inline void set_array_element_data_at(vm::RtArray* array, int32_t index, T value) noexcept
{
    vm::Array::set_array_data_at<T>(array, index, value);
}

inline RtResult<int32_t> get_mdarray_global_index_from_indices(vm::RtArray* arr, int32_t* indices) noexcept
{
    return vm::Array::get_mdarray_global_index_from_indices3(arr, indices);
}

inline RtResult<vm::RtMulticastDelegate*> new_delegate(const metadata::RtClass* delelgate_type, vm::RtObject* target,
                                                       const metadata::RtMethodInfo* method) noexcept
{
    return vm::Delegate::new_delegate(delelgate_type, target, method);
}

inline RtResult<const uint8_t*> get_field_rva_data(const metadata::RtFieldInfo* field) noexcept
{
    return vm::Field::get_field_rva_data(field);
}

inline uint32_t get_field_offset_includes_object_header(const metadata::RtFieldInfo* field) noexcept
{
    return vm::Field::get_field_offset_includes_object_header_for_all_type(field);
}

inline uint32_t get_field_offset_includes_object_header_for_reference_type(const metadata::RtFieldInfo* field) noexcept
{
    return vm::Field::get_field_offset_includes_object_header_for_reference_type(field);
}

inline uint32_t get_class_instance_size_with_object_header(const metadata::RtClass* klass) noexcept
{
    return vm::Class::get_instance_size_with_object_header(klass);
}

inline uint32_t get_class_instance_size_without_object_header(const metadata::RtClass* klass) noexcept
{
    return vm::Class::get_instance_size_without_object_header(klass);
}

inline uint32_t get_class_field_count(const metadata::RtClass* klass) noexcept
{
    return klass->field_count;
}

inline uint32_t get_class_static_size(const metadata::RtClass* klass) noexcept
{
    return klass->static_size;
}

inline RtResult<vm::RtReflectionAssembly*> get_assembly_reflection_object(const metadata::RtModuleDef* mod) noexcept
{
    return vm::Reflection::get_assembly_reflection_object(mod->get_assembly());
}

inline RtResult<vm::RtReflectionMethod*> get_method_reflection_object(const metadata::RtMethodInfo* method) noexcept
{
    return vm::Reflection::get_method_reflection_object(method, method->parent);
}

template <typename Src, typename Dst>
inline int32_t cast_float_to_small_int(Src value) noexcept
{
    return interp::cast_float_to_small_int<Src, Dst>(value);
}

template <typename Src, typename Dst>
inline int32_t cast_float_to_i32(Src value) noexcept
{
    return interp::cast_float_to_i32<Src, Dst>(value);
}

template <typename Src, typename Dst>
inline int64_t cast_float_to_i64(Src value) noexcept
{
    return interp::cast_float_to_i64<Src, Dst>(value);
}

template <typename Src, typename Dst>
inline intptr_t cast_float_to_intptr(Src value) noexcept
{
    return interp::cast_float_to_intptr<Src, Dst>(value);
}

inline vm::InternalCallFunction resolve_internal_call(const char* name) noexcept
{
    return (vm::InternalCallFunction)vm::InternalCalls::get_lite_internal_call(name);
}

RtErr raise_internal_call_entry_not_found_error(const char* name) noexcept;

using vm::PInvokeFunction;

inline PInvokeFunction resolve_pinvoke_function(const char* dll_name_no_ext, const char* function_name) noexcept
{
    return vm::PInvokes::get_pinvoke_function(dll_name_no_ext, function_name);
}

RtErr raise_pinvoke_entry_not_found_error(const char* dll_name_no_ext, const char* function_name) noexcept;

using utils::AnsiStringBuilder;
using utils::Utf16StringBuilder;
using utils::Utf8StringBuilder;
typedef leanclr::metadata::RtMarshalHandle RtMarshalHandle;
typedef leanclr::metadata::RtMarshalUTF8Str RtMarshalUTF8Str;
typedef leanclr::metadata::RtMarshalUTF16Str RtMarshalUTF16Str;
typedef leanclr::metadata::RtMarshalAnsiStr RtMarshalAnsiStr;

inline Utf16Char marshal_utf8_char_to_managed_char(char c) noexcept
{
    return static_cast<Utf16Char>(c);
}

inline char marshal_managed_char_to_utf8_char(Utf16Char c) noexcept
{
    return static_cast<char>(c);
}

inline Utf16Char marshal_ansi_cstr_to_managed_char(AnsiChar c) noexcept
{
    return static_cast<Utf16Char>(static_cast<unsigned char>(c));
}

inline AnsiChar marshal_managed_char_to_ansi_char(Utf16Char c) noexcept
{
    return static_cast<AnsiChar>(c);
}

inline vm::RtString* marshal_utf8_string_to_managed_string(const char* str) noexcept
{
    return str ? vm::String::create_string_from_utf8cstr(str) : nullptr;
}

inline RtMarshalUTF8Str marshal_managed_string_to_utf8_string(vm::RtString* str, Utf8StringBuilder& temp) noexcept
{
    if (!str)
    {
        return nullptr;
    }
    temp.append_utf16_str(vm::String::get_chars_ptr(str), static_cast<size_t>(vm::String::get_length(str)));
    return (RtMarshalUTF8Str)temp.get_mut_chars();
}

inline RtMarshalUTF8Str marshal_managed_string_to_utf8_string(vm::RtString* str) noexcept
{
    if (!str)
    {
        return nullptr;
    }
    utils::Utf8StringBuilder temp(vm::String::get_chars_ptr(str), static_cast<size_t>(vm::String::get_length(str)));
    return (RtMarshalUTF8Str)temp.dup_zero_terminated_chars();
}

inline RtMarshalUTF16Str marshal_managed_string_to_utf16_string(vm::RtString* str) noexcept
{
    if (!str)
    {
        return nullptr;
    }
    size_t length = static_cast<size_t>(vm::String::get_length(str));
    Utf16Char* result = static_cast<Utf16Char*>(alloc::GeneralAllocation::calloc(length + 1, sizeof(Utf16Char)));
    std::memcpy(result, vm::String::get_chars_ptr(str), length * sizeof(Utf16Char));
    result[length] = 0;
    return result;
}

inline vm::RtString* marshal_utf16_string_to_managed_string(const Utf16Char* str) noexcept
{
    int32_t length = utils::StringUtil::get_utf16chars_length(str);
    return vm::String::create_string_from_utf16chars(str, length);
}

RtMarshalAnsiStr marshal_managed_string_to_ansi_string(vm::RtString* str, AnsiStringBuilder& temp) noexcept;
RtMarshalAnsiStr marshal_managed_string_to_ansi_string(vm::RtString* str) noexcept;
vm::RtString* marshal_ansi_string_to_managed_string(const RtMarshalAnsiStr str) noexcept;
RtMarshalUTF8Str marshal_managed_string_builder_to_utf8_string(vm::RtObject* sb, Utf8StringBuilder& temp) noexcept;
RtMarshalUTF16Str marshal_managed_string_builder_to_utf16_string(vm::RtObject* sb) noexcept;
RtMarshalAnsiStr marshal_managed_string_builder_to_ansi_string(vm::RtObject* sb, AnsiStringBuilder& temp) noexcept;
void sync_managed_string_builder_from_utf8_buffer(vm::RtObject* sb, const Utf8Char* str) noexcept;
void sync_managed_string_builder_from_utf16_buffer(vm::RtObject* sb, const Utf16Char* str) noexcept;
void sync_managed_string_builder_from_ansi_buffer(vm::RtObject* sb, const RtMarshalAnsiStr str) noexcept;

using metadata::RtNativeMethodPointer;

inline RtResult<RtNativeMethodPointer> marshal_delegate_to_fn_ptr(vm::RtDelegate* del) noexcept
{
    return vm::Marshal::get_function_pointer_for_delegate(del);
}

inline RtResult<vm::RtDelegate*> marshal_fn_ptr_to_delegate(RtNativeMethodPointer fn_ptr, const metadata::RtTypeSig* del_typesig) noexcept
{
    if (fn_ptr == nullptr)
    {
        RET_OK(nullptr);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(del_typesig));
    return vm::Marshal::marshal_function_pointer_to_delegate(fn_ptr, klass);
}

RtMarshalHandle marshal_safe_handle_to_handle(vm::RtObject* obj) noexcept;
RtResult<vm::RtObject*> marshal_handle_to_safe_handle(RtMarshalHandle handle, const metadata::RtTypeSig* handle_typesig) noexcept;

void* marshal_managed_array_to_native_array(vm::RtArray* managed_array, size_t& native_element_count) noexcept;
RtResult<vm::RtArray*> marshal_native_array_to_managed_array(void* native_array, size_t native_element_count,
                                                             const metadata::RtTypeSig* array_param_typesig) noexcept;
void free_native_array(void* native_array) noexcept;

void marshal_managed_array_to_native_val_array(vm::RtArray* managed_array, void* native_array, size_t native_element_count) noexcept;
RtResult<vm::RtArray*> marshal_native_val_array_to_managed_array(void* native_array, size_t native_element_count,
                                                                 const metadata::RtTypeSig* array_typesig) noexcept;

} // namespace codegen
} // namespace leanclr

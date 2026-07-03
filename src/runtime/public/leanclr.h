#pragma once

#include <stdint.h>
#include <stddef.h>

#ifndef LEANCLR_EXPORT
#ifdef _MSC_VER
#define LEANCLR_EXPORT __declspec(dllexport)
#define LEANCLR_IMPORT __declspec(dllimport)
#else
#define LEANCLR_EXPORT __attribute__((visibility("default")))
#define LEANCLR_IMPORT
#endif
#endif

#ifdef LEANCLR_DLL_EXPORTS
#define LEANCLR_API LEANCLR_EXPORT
#else
#define LEANCLR_API LEANCLR_IMPORT
#endif

struct LeanclrTypeSig;
struct LeanclrAssembly;
struct LeanclrModuleDef;
struct LeanclrClass;
struct LeanclrMethodInfo;
struct LeanclrFieldInfo;
struct LeanclrPropertyInfo;
struct LeanclrEventInfo;
struct LeanclrException;
struct LeanclrStackObject;

#define LEANCLR_STACK_OBJECT_SIZE 8

#define leanclr_get_stack_object_size_of_byte_size(byte_size) (((byte_size) + LEANCLR_STACK_OBJECT_SIZE - 1) / LEANCLR_STACK_OBJECT_SIZE)

#ifdef __cplusplus
extern "C"
{
#endif

    LEANCLR_API int32_t leanclr_initialize_runtime();
    LEANCLR_API void leanclr_shutdown_runtime();

    LEANCLR_API size_t leanclr_get_assembly_count();
    LEANCLR_API size_t leanclr_get_assemblies(LeanclrAssembly** out_assemblies, size_t out_assemblies_capacity, LeanclrException** out_exception);
    LEANCLR_API LeanclrAssembly* leanclr_get_assembly(const char* assembly_name);
    LEANCLR_API LeanclrAssembly* leanclr_load_assembly(const char* assembly_name, LeanclrException** out_exception);
    LEANCLR_API LeanclrAssembly* leanclr_get_assembly_by_module(LeanclrModuleDef* mod);
    LEANCLR_API LeanclrModuleDef* leanclr_get_module_by_assembly(LeanclrAssembly* ass);
    LEANCLR_API bool leanclr_is_corlib(LeanclrModuleDef* mod);
    LEANCLR_API const char* leanclr_get_module_name_noext(LeanclrModuleDef* mod);

    LEANCLR_API size_t leanclr_get_class_count(LeanclrModuleDef* mod);
    LEANCLR_API void leanclr_get_classes(LeanclrModuleDef* mod, bool export_only, LeanclrClass**& class_buf, size_t& class_buf_capacity,
                                         LeanclrException** out_exception);
    LEANCLR_API LeanclrClass* leanclr_get_class_by_name(LeanclrModuleDef* mod, const char* full_name, bool ignore_case, LeanclrException** out_exception);

    LEANCLR_API LeanclrModuleDef* leanclr_get_class_module(LeanclrClass* klass);
    LEANCLR_API const char* leanclr_get_class_namespace(LeanclrClass* klass);
    LEANCLR_API const char* leanclr_get_class_name(LeanclrClass* klass);
    LEANCLR_API const LeanclrTypeSig* leanclr_get_class_byval_typesig(LeanclrClass* klass);
    LEANCLR_API const LeanclrTypeSig* leanclr_get_class_byref_typesig(LeanclrClass* klass);
    LEANCLR_API LeanclrClass* leanclr_get_class_parent(LeanclrClass* klass);
    LEANCLR_API void leanclr_get_class_interfaces(LeanclrClass* klass, const LeanclrClass**& interfaces, size_t& count, LeanclrException** out_exception);

    LEANCLR_API LeanclrClass* leanclr_get_class_enclosing_class(LeanclrClass* klass);
    LEANCLR_API void leanclr_get_class_nested_classes(LeanclrClass* klass, const LeanclrClass**& nested_classes, size_t& count,
                                                      LeanclrException** out_exception);

    LEANCLR_API void leanclr_get_class_methods(LeanclrClass* klass, const LeanclrMethodInfo**& methods, size_t& count, LeanclrException** out_exception);
    LEANCLR_API const LeanclrMethodInfo* leanclr_get_class_method_by_name(LeanclrClass* klass, const char* method_name, LeanclrException** out_exception);

    LEANCLR_API void leanclr_get_class_field(LeanclrClass* klass, const LeanclrFieldInfo*& fields, size_t& count, LeanclrException** out_exception);
    LEANCLR_API const LeanclrFieldInfo* leanclr_get_class_field_by_name(LeanclrClass* klass, const char* field_name, LeanclrException** out_exception);
    LEANCLR_API void leanclr_get_class_properties(LeanclrClass* klass, const LeanclrPropertyInfo*& properties, size_t& count, LeanclrException** out_exception);
    LEANCLR_API const LeanclrPropertyInfo* leanclr_get_class_property_by_name(LeanclrClass* klass, const char* property_name, LeanclrException** out_exception);

    LEANCLR_API void leanclr_get_class_events(LeanclrClass* klass, const LeanclrEventInfo*& events, size_t& count, LeanclrException** out_exception);

    LEANCLR_API const LeanclrEventInfo* leanclr_get_class_event_by_name(LeanclrClass* klass, const char* event_name, LeanclrException** out_exception);

    LEANCLR_API void leanclr_get_argument(const LeanclrStackObject* eval_stack, size_t* offset, void* data, size_t data_size);
    LEANCLR_API void leanclr_push_argument(LeanclrStackObject* eval_stack, size_t* offset, const void* data, size_t data_size);

    LEANCLR_API void leanclr_get_return_value(const LeanclrStackObject* ret_buff, void* data, size_t data_size);
    LEANCLR_API void leanclr_set_return_value(LeanclrStackObject* eval_stack, const void* data, size_t data_size);

    LEANCLR_API size_t leanclr_get_total_arg_stack_object_size(const LeanclrMethodInfo* method);
    LEANCLR_API size_t leanclr_get_return_value_stack_object_size(const LeanclrMethodInfo* method);

    typedef void (*LeanclrMethodPointer)();
    typedef void (*LeanclrMethodInvoker)(LeanclrMethodPointer method_ptr, const LeanclrMethodInfo* method, const LeanclrStackObject* args,
                                         LeanclrStackObject* ret, LeanclrException** exception);

    LEANCLR_API void leanclr_register_pinvoke_func(const char* name, LeanclrMethodPointer func, LeanclrMethodInvoker invoker);
    LEANCLR_API void leanclr_register_internal_call_func(const char* name, LeanclrMethodPointer func, LeanclrMethodInvoker invoker);
    LEANCLR_API void leanclr_register_newobj_internal_call_func(const char* name, LeanclrMethodInvoker invoker);
    LEANCLR_API void leanclr_register_intrinsic_func(const char* name, LeanclrMethodPointer func, LeanclrMethodInvoker invoker);
    LEANCLR_API void leanclr_register_newobj_intrinsic_func(const char* name, LeanclrMethodInvoker invoker);

    LEANCLR_API void leanclr_invoke_with_buffer(const LeanclrMethodInfo* method, const LeanclrStackObject* arg_buff, LeanclrStackObject* ret_buff,
                                                LeanclrException** out_exception);

#define LEANCLR_DECLARING_ALLOC_METHOD_ARGUMENT_BUFFER(arg_buff_name, offset, method)                                                             \
    LeanclrStackObject* arg_buff_name = (LeanclrStackObject*)alloca(leanclr_get_total_arg_stack_object_size(method) * LEANCLR_STACK_OBJECT_SIZE); \
    size_t offset = 0;

#define LEANCLR_DECLARING_ALLOC_METHOD_RETURN_BUFFER(ret_buff_name, method) \
    LeanclrStackObject* ret_buff_name = (LeanclrStackObject*)alloca(leanclr_get_return_value_stack_object_size(method) * LEANCLR_STACK_OBJECT_SIZE);

#ifdef __cplusplus
}
#endif
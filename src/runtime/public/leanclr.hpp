#pragma once

#include <vector>

#include "core/rt_base.h"
#include "vm/rt_managed_types.h"
#include "vm/pinvoke.h"
#include "vm/intrinsics.h"
#include "vm/internal_calls.h"

namespace leanclr
{
class RuntimeApi
{
  public:
    static size_t get_stack_object_size_of_byte_size(size_t byte_size)
    {
        return (byte_size + sizeof(interp::RtStackObject) - 1) / sizeof(interp::RtStackObject);
    }

    template <typename T>
    static constexpr size_t get_stack_object_size_for_type()
    {
        return (sizeof(T) + sizeof(interp::RtStackObject) - 1) / sizeof(interp::RtStackObject);
    }

    template <typename T>
    static void push_argument(interp::RtStackObject* eval_stack, size_t& offset, const T& value)
    {
        *(T*)(eval_stack + offset) = value;
        offset += get_stack_object_size_for_type<T>();
    }

    static void push_argument(interp::RtStackObject* eval_stack, size_t& offset, void* obj, size_t byte_size)
    {
        std::memcpy(eval_stack + offset, obj, byte_size);
        offset += get_stack_object_size_of_byte_size(byte_size);
    }

    template <typename T>
    static void get_argument(const interp::RtStackObject* eval_stack, size_t& offset, T& value)
    {
        value = *(T*)(eval_stack + offset);
        offset += get_stack_object_size_for_type<T>();
    }

    template <typename T>
    static T get_argument(const interp::RtStackObject* eval_stack, size_t& offset)
    {
        size_t old_offset = offset;
        offset += get_stack_object_size_for_type<T>();
        return *(T*)(eval_stack + old_offset);
    }

    static void get_argument(const interp::RtStackObject* eval_stack, size_t& offset, void* obj, size_t byte_size)
    {
        std::memcpy(obj, eval_stack + offset, byte_size);
        offset += get_stack_object_size_of_byte_size(byte_size);
    }

    template <typename T>
    static const T& get_return_value(interp::RtStackObject* ret_buff_name)
    {
        return *(T*)(ret_buff_name);
    }

    static void get_return_value(interp::RtStackObject* ret_buff_name, void* obj, size_t byte_size)
    {
        std::memcpy(obj, ret_buff_name, byte_size);
    }

    template <typename T>
    static void set_return_value(interp::RtStackObject* eval_stack, const T& value)
    {
        *(T*)eval_stack = value;
    }

    static void set_return_value(interp::RtStackObject* eval_stack, void* obj, size_t byte_size)
    {
        std::memcpy(eval_stack, obj, byte_size);
    }

    static size_t get_total_arg_stack_object_size(const metadata::RtMethodInfo* method);
    static size_t get_return_value_stack_object_size(const metadata::RtMethodInfo* method);

    static RtResultVoid invoke_with_buffer(const metadata::RtMethodInfo* method, interp::RtStackObject* arg_buff, interp::RtStackObject* ret_buff);

    static void register_pinvoke_func(const char* name, vm::PInvokeFunction func, vm::PInvokeInvoker invoker);
    static void register_internal_call_func(const char* name, vm::InternalCallFunction func, vm::InternalCallInvoker invoker);
    static void register_newobj_internal_call_func(const char* name, vm::InternalCallInvoker invoker);
    static void register_intrinsic_func(const char* name, vm::IntrinsicFunction func, vm::IntrinsicInvoker invoker);
    static void register_newobj_intrinsic_func(const char* name, vm::IntrinsicInvoker invoker);

    static RtErr initialize_runtime();
    static void shutdown_runtime();

    static RtResultVoid get_assemblies(utils::Vector<metadata::RtAssembly*>& out_assemblies);
    static RtResult<metadata::RtAssembly*> get_assembly(const char* assembly_name);
    static RtResult<metadata::RtAssembly*> load_assembly(const char* assembly_name);
    static metadata::RtModuleDef* get_assembly_by_module(metadata::RtAssembly* ass);
    static metadata::RtAssembly* get_module_by_assembly(metadata::RtModuleDef* mod);
    static bool is_corlib(metadata::RtModuleDef* mod);
    static const char* get_module_name_noext(metadata::RtModuleDef* mod);

    static RtResultVoid get_classes(metadata::RtModuleDef* mod, bool export_only, utils::Vector<metadata::RtClass*>& classes);
    static RtResult<metadata::RtClass*> get_class_by_name(metadata::RtModuleDef* ass, const char* full_name, bool ignore_case);

    static metadata::RtModuleDef* get_class_module(const metadata::RtClass* klass);
    static const char* get_class_namespace(const metadata::RtClass* klass);
    static const char* get_class_name(const metadata::RtClass* klass);
    static const metadata::RtTypeSig* get_class_byval_typesig(const metadata::RtClass* klass);
    static const metadata::RtTypeSig* get_class_byref_typesig(const metadata::RtClass* klass);
    static metadata::RtClass* get_class_parent(const metadata::RtClass* klass);
    static RtResultVoid get_class_interfaces(const metadata::RtClass* klass, const metadata::RtClass**& interfaces, size_t& count);

    static metadata::RtClass* get_class_enclosing_class(const metadata::RtClass* klass);
    static RtResultVoid get_class_nested_classes(const metadata::RtClass* klass, const metadata::RtClass**& nested_classes, size_t& count);

    static RtResultVoid get_class_methods(const metadata::RtClass* klass, const metadata::RtMethodInfo**& methods, size_t& count);
    static RtResult<const metadata::RtMethodInfo*> get_class_method_by_name(const metadata::RtClass* klass, const char* method_name);

    static RtResultVoid get_class_field(const metadata::RtClass* klass, const metadata::RtFieldInfo*& fields, size_t& count);
    static RtResult<const metadata::RtFieldInfo*> get_class_field_by_name(const metadata::RtClass* klass, const char* field_name);

    static RtResultVoid get_class_properties(const metadata::RtClass* klass, const metadata::RtPropertyInfo*& properties, size_t& count);
    static RtResult<const metadata::RtPropertyInfo*> get_class_property_by_name(const metadata::RtClass* klass, const char* property_name);

    static RtResultVoid get_class_events(const metadata::RtClass* klass, const metadata::RtEventInfo*& events, size_t& count);
    static RtResult<const metadata::RtEventInfo*> get_class_event_by_name(const metadata::RtClass* klass, const char* event_name);
};

#define DECLARING_ALLOC_METHOD_ARGUMENT_BUFFER(arg_buff_name, offset, method)                                                         \
    interp::RtStackObject* arg_buff_name =                                                                                            \
        (interp::RtStackObject*)alloca(leanclr::RuntimeApi::get_total_arg_stack_object_size(method) * sizeof(interp::RtStackObject)); \
    size_t offset = 0;

#define DECLARING_ALLOC_METHOD_RETURN_BUFFER(ret_buff_name, method) \
    interp::RtStackObject* ret_buff_name =                          \
        (interp::RtStackObject*)alloca(leanclr::RuntimeApi::get_return_value_stack_object_size(method) * sizeof(interp::RtStackObject));
} // namespace leanclr
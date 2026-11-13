
#include "public/leanclr.h"
#include "invoker_bridge.h"

#include "vm/pinvoke.h"
#include "vm/method.h"
#include "vm/runtime.h"
#include "vm/rt_exception.h"
#include "vm/pinvoke.h"
#include "vm/internal_calls.h"
#include "vm/intrinsics.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "metadata/module_def.h"

using namespace leanclr;

struct LeanclrStackObject
{
    uint64_t __value;
};

static_assert(sizeof(LeanclrStackObject) == sizeof(interp::RtStackObject), "Public stack slot size must match the interpreter ABI");
static_assert(alignof(LeanclrStackObject) == alignof(interp::RtStackObject), "Public stack slot alignment must match the interpreter ABI");

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t leanclr_initialize_runtime()
    {
        auto ret = vm::Runtime::initialize();
        if (ret.is_ok())
            return 0;
        else
            return (int32_t)ret.unwrap_err();
    }

    void leanclr_shutdown_runtime()
    {
        vm::Runtime::shutdown();
    }

    size_t leanclr_get_assembly_count()
    {
        utils::Vector<metadata::RtModuleDef*> modules;
        metadata::RtModuleDef::get_registered_modules(modules);
        return modules.size();
    }

    size_t leanclr_get_assemblies(LeanclrAssembly** out_assemblies, size_t out_assemblies_capacity, LeanclrException** out_exception)
    {
        if (out_exception)
            *out_exception = nullptr;
        if (!out_assemblies && out_assemblies_capacity != 0)
        {
            if (out_exception)
                *out_exception = reinterpret_cast<LeanclrException*>(vm::Exception::raise_error_as_exception(RtErr::ArgumentNull, nullptr, nullptr));
            return 0;
        }
        utils::Vector<metadata::RtModuleDef*> modules;
        metadata::RtModuleDef::get_registered_modules(modules);
        size_t to_copy = std::min(out_assemblies_capacity, modules.size());
        for (size_t i = 0; i < to_copy; i++)
        {
            out_assemblies[i] = reinterpret_cast<LeanclrAssembly*>(modules[i]->get_assembly());
        }
        return to_copy;
    }

    LeanclrAssembly* leanclr_get_assembly(const char* assembly_name)
    {
        return reinterpret_cast<LeanclrAssembly*>(vm::Assembly::find_by_name(assembly_name));
    }

    LeanclrAssembly* leanclr_load_assembly(const char* assembly_name, LeanclrException** out_exception)
    {
        auto ret = vm::Assembly::load_by_name(assembly_name);
        if (ret.is_ok())
        {
            if (out_exception)
                *out_exception = nullptr;
            return reinterpret_cast<LeanclrAssembly*>(ret.unwrap());
        }
        else
        {
            if (out_exception)
            {
                *out_exception = reinterpret_cast<LeanclrException*>(vm::Exception::raise_error_as_exception(ret.unwrap_err(), nullptr, nullptr));
            }
            return nullptr;
        }
    }

    LeanclrAssembly* leanclr_get_assembly_by_module(LeanclrModuleDef* mod)
    {
        return reinterpret_cast<LeanclrAssembly*>(((metadata::RtModuleDef*)mod)->get_assembly());
    }

    LeanclrModuleDef* leanclr_get_module_by_assembly(LeanclrAssembly* ass)
    {
        return reinterpret_cast<LeanclrModuleDef*>(((metadata::RtAssembly*)ass)->mod);
    }

    bool leanclr_is_corlib(LeanclrModuleDef* mod)
    {
        return ((metadata::RtModuleDef*)mod)->is_corlib();
    }

    const char* leanclr_get_module_name_noext(LeanclrModuleDef* mod)
    {
        return ((metadata::RtModuleDef*)mod)->get_name_no_ext();
    }

    size_t leanclr_get_class_count(LeanclrModuleDef* mod)
    {
        return reinterpret_cast<metadata::RtModuleDef*>(mod)->get_class_count();
    }

#define SET_EXCEPTION_ON_ERR(ret, out_ex_ptr)                                                                                               \
    if (ret.is_err())                                                                                                                       \
    {                                                                                                                                       \
        if (out_ex_ptr)                                                                                                                     \
        {                                                                                                                                   \
            *out_ex_ptr = reinterpret_cast<LeanclrException*>(vm::Exception::raise_error_as_exception(ret.unwrap_err(), nullptr, nullptr)); \
        }                                                                                                                                   \
        return;                                                                                                                             \
    }                                                                                                                                       \
    else                                                                                                                                    \
    {                                                                                                                                       \
        if (out_ex_ptr)                                                                                                                     \
        {                                                                                                                                   \
            *out_ex_ptr = nullptr;                                                                                                          \
        }                                                                                                                                   \
    }

#define SET_EXCEPTION_RETURN_INVALID_ON_ERR(ret, out_ex_ptr, invalid_value)                                                                 \
    if (ret.is_err())                                                                                                                       \
    {                                                                                                                                       \
        if (out_ex_ptr)                                                                                                                     \
        {                                                                                                                                   \
            *out_ex_ptr = reinterpret_cast<LeanclrException*>(vm::Exception::raise_error_as_exception(ret.unwrap_err(), nullptr, nullptr)); \
        }                                                                                                                                   \
        return invalid_value;                                                                                                               \
    }                                                                                                                                       \
    else                                                                                                                                    \
    {                                                                                                                                       \
        if (out_ex_ptr)                                                                                                                     \
        {                                                                                                                                   \
            *out_ex_ptr = nullptr;                                                                                                          \
        }                                                                                                                                   \
    }

    void leanclr_get_classes(LeanclrModuleDef* mod, bool export_only, LeanclrClass*** out_class_buf, size_t* out_class_buf_capacity,
                             LeanclrException** out_exception)
    {
        if (!out_class_buf || !out_class_buf_capacity)
        {
            if (out_exception)
                *out_exception = reinterpret_cast<LeanclrException*>(vm::Exception::raise_error_as_exception(RtErr::ArgumentNull, nullptr, nullptr));
            return;
        }
        auto& class_buf = *out_class_buf;
        auto& class_buf_capacity = *out_class_buf_capacity;
        metadata::RtModuleDef* rt_mod = reinterpret_cast<metadata::RtModuleDef*>(mod);
        const size_t capacity = class_buf_capacity;
        utils::Vector<metadata::RtClass*> classes;
        auto ret = rt_mod->get_types(export_only, classes);
        class_buf_capacity = 0;
        SET_EXCEPTION_ON_ERR(ret, out_exception);
        const size_t copy_count = std::min(capacity, classes.size());
        for (size_t i = 0; i < copy_count; i++)
        {
            class_buf[i] = reinterpret_cast<LeanclrClass*>(classes[i]);
        }
        class_buf_capacity = copy_count;
    }

    LeanclrClass* leanclr_get_class_by_name(LeanclrModuleDef* mod, const char* full_name, bool ignore_case, LeanclrException** out_exception)
    {
        metadata::RtModuleDef* rt_mod = reinterpret_cast<metadata::RtModuleDef*>(mod);
        auto ret = rt_mod->get_class_by_name(full_name, ignore_case, false);
        SET_EXCEPTION_RETURN_INVALID_ON_ERR(ret, out_exception, nullptr);
        return reinterpret_cast<LeanclrClass*>(ret.unwrap());
    }

    LeanclrModuleDef* leanclr_get_class_module(LeanclrClass* klass)
    {
        return reinterpret_cast<LeanclrModuleDef*>(((metadata::RtClass*)klass)->image);
    }

    const char* leanclr_get_class_namespace(LeanclrClass* klass)
    {
        return ((metadata::RtClass*)klass)->namespaze;
    }

    const char* leanclr_get_class_name(LeanclrClass* klass)
    {
        return ((metadata::RtClass*)klass)->name;
    }

    const LeanclrTypeSig* leanclr_get_class_byval_typesig(LeanclrClass* klass)
    {
        return reinterpret_cast<const LeanclrTypeSig*>(vm::Class::get_by_val_type_sig(reinterpret_cast<metadata::RtClass*>(klass)));
    }

    const LeanclrTypeSig* leanclr_get_class_byref_typesig(LeanclrClass* klass)
    {
        return reinterpret_cast<const LeanclrTypeSig*>(vm::Class::get_by_ref_type_sig(reinterpret_cast<metadata::RtClass*>(klass)));
    }

    LeanclrClass* leanclr_get_class_parent(LeanclrClass* klass)
    {
        return const_cast<LeanclrClass*>(reinterpret_cast<const LeanclrClass*>(((const metadata::RtClass*)klass)->parent));
    }

    void leanclr_get_class_interfaces(LeanclrClass* klass, const LeanclrClass*** out_interfaces, size_t* out_count, LeanclrException** out_exception)
    {
        if (!out_interfaces || !out_count)
        {
            if (out_exception)
                *out_exception = reinterpret_cast<LeanclrException*>(vm::Exception::raise_error_as_exception(RtErr::ArgumentNull, nullptr, nullptr));
            return;
        }
        auto& interfaces = *out_interfaces;
        auto& count = *out_count;
        auto rt_klass = reinterpret_cast<metadata::RtClass*>(klass);
        auto ret = vm::Class::initialize_interfaces(rt_klass);
        SET_EXCEPTION_ON_ERR(ret, out_exception);
        count = rt_klass->interface_count;
        interfaces = (const LeanclrClass**)(rt_klass->interfaces);
    }

    LeanclrClass* leanclr_get_class_enclosing_class(LeanclrClass* klass)
    {
        return const_cast<LeanclrClass*>(reinterpret_cast<const LeanclrClass*>(((const metadata::RtClass*)klass)->declaring_class));
    }

    void leanclr_get_class_nested_classes(LeanclrClass* klass, const LeanclrClass*** out_nested_classes, size_t* out_count, LeanclrException** out_exception)
    {
        if (!out_nested_classes || !out_count)
        {
            if (out_exception)
                *out_exception = reinterpret_cast<LeanclrException*>(vm::Exception::raise_error_as_exception(RtErr::ArgumentNull, nullptr, nullptr));
            return;
        }
        auto& nested_classes = *out_nested_classes;
        auto& count = *out_count;
        auto rt_klass = reinterpret_cast<metadata::RtClass*>(klass);
        auto ret = vm::Class::initialize_nested_classes(rt_klass);
        SET_EXCEPTION_ON_ERR(ret, out_exception);
        count = rt_klass->nested_class_count;
        nested_classes = (const LeanclrClass**)(rt_klass->nested_classes);
    }

    void leanclr_get_class_methods(LeanclrClass* klass, const LeanclrMethodInfo*** out_methods, size_t* out_count, LeanclrException** out_exception)
    {
        if (!out_methods || !out_count)
        {
            if (out_exception)
                *out_exception = reinterpret_cast<LeanclrException*>(vm::Exception::raise_error_as_exception(RtErr::ArgumentNull, nullptr, nullptr));
            return;
        }
        auto& methods = *out_methods;
        auto& count = *out_count;
        auto rt_klass = reinterpret_cast<metadata::RtClass*>(klass);
        auto ret = vm::Class::initialize_methods(rt_klass);
        SET_EXCEPTION_ON_ERR(ret, out_exception);
        count = rt_klass->method_count;
        methods = (const LeanclrMethodInfo**)(rt_klass->methods);
    }

    const LeanclrMethodInfo* leanclr_get_class_method_by_name(LeanclrClass* klass, const char* method_name, LeanclrException** out_exception)
    {
        auto rt_klass = reinterpret_cast<metadata::RtClass*>(klass);
        auto ret = vm::Class::initialize_methods(rt_klass);
        SET_EXCEPTION_RETURN_INVALID_ON_ERR(ret, out_exception, nullptr);
        return reinterpret_cast<const LeanclrMethodInfo*>(vm::Method::find_matched_method_in_class_by_name(rt_klass, method_name));
    }

    void leanclr_get_class_field(LeanclrClass* klass, const LeanclrFieldInfo** out_fields, size_t* out_count, LeanclrException** out_exception)
    {
        if (!out_fields || !out_count)
        {
            if (out_exception)
                *out_exception = reinterpret_cast<LeanclrException*>(vm::Exception::raise_error_as_exception(RtErr::ArgumentNull, nullptr, nullptr));
            return;
        }
        auto& fields = *out_fields;
        auto& count = *out_count;
        auto rt_klass = reinterpret_cast<metadata::RtClass*>(klass);
        auto ret = vm::Class::initialize_fields(rt_klass);
        SET_EXCEPTION_ON_ERR(ret, out_exception);
        fields = (const LeanclrFieldInfo*)(rt_klass->fields);
        count = rt_klass->field_count;
    }

    const LeanclrFieldInfo* leanclr_get_class_field_by_name(LeanclrClass* klass, const char* field_name, LeanclrException** out_exception)
    {
        auto rt_klass = reinterpret_cast<metadata::RtClass*>(klass);
        auto ret = vm::Class::initialize_fields(rt_klass);
        SET_EXCEPTION_RETURN_INVALID_ON_ERR(ret, out_exception, nullptr);
        return reinterpret_cast<const LeanclrFieldInfo*>(vm::Class::get_field_for_name(rt_klass, field_name, false));
    }

    void leanclr_get_class_properties(LeanclrClass* klass, const LeanclrPropertyInfo** out_properties, size_t* out_count, LeanclrException** out_exception)
    {
        if (!out_properties || !out_count)
        {
            if (out_exception)
                *out_exception = reinterpret_cast<LeanclrException*>(vm::Exception::raise_error_as_exception(RtErr::ArgumentNull, nullptr, nullptr));
            return;
        }
        auto& properties = *out_properties;
        auto& count = *out_count;
        auto rt_klass = reinterpret_cast<metadata::RtClass*>(klass);
        auto ret = vm::Class::initialize_properties(rt_klass);
        SET_EXCEPTION_ON_ERR(ret, out_exception);
        properties = (const LeanclrPropertyInfo*)(rt_klass->properties);
        count = rt_klass->property_count;
    }

    const LeanclrPropertyInfo* leanclr_get_class_property_by_name(LeanclrClass* klass, const char* property_name, LeanclrException** out_exception)
    {
        auto rt_klass = reinterpret_cast<metadata::RtClass*>(klass);
        auto ret = vm::Class::initialize_properties(rt_klass);
        SET_EXCEPTION_RETURN_INVALID_ON_ERR(ret, out_exception, nullptr);
        return reinterpret_cast<const LeanclrPropertyInfo*>(vm::Class::get_property_for_name(rt_klass, property_name, false));
    }

    void leanclr_get_class_events(LeanclrClass* klass, const LeanclrEventInfo** out_events, size_t* out_count, LeanclrException** out_exception)
    {
        if (!out_events || !out_count)
        {
            if (out_exception)
                *out_exception = reinterpret_cast<LeanclrException*>(vm::Exception::raise_error_as_exception(RtErr::ArgumentNull, nullptr, nullptr));
            return;
        }
        auto& events = *out_events;
        auto& count = *out_count;
        auto rt_klass = reinterpret_cast<metadata::RtClass*>(klass);
        auto ret = vm::Class::initialize_events(rt_klass);
        SET_EXCEPTION_ON_ERR(ret, out_exception);
        events = (const LeanclrEventInfo*)(rt_klass->events);
        count = rt_klass->event_count;
    }

    const LeanclrEventInfo* leanclr_get_class_event_by_name(LeanclrClass* klass, const char* event_name, LeanclrException** out_exception)
    {
        auto rt_klass = reinterpret_cast<metadata::RtClass*>(klass);
        auto ret = vm::Class::initialize_events(rt_klass);
        SET_EXCEPTION_RETURN_INVALID_ON_ERR(ret, out_exception, nullptr);
        return reinterpret_cast<const LeanclrEventInfo*>(vm::Class::get_event_for_name(rt_klass, event_name, false));
    }

    void leanclr_get_argument(const LeanclrStackObject* eval_stack, size_t* offset, void* data, size_t data_size)
    {
        std::memcpy(data, eval_stack + *offset, data_size);
        *offset += leanclr_get_stack_object_size_of_byte_size(data_size);
    }

    void leanclr_push_argument(LeanclrStackObject* eval_stack, size_t* offset, const void* data, size_t data_size)
    {
        std::memcpy(eval_stack + *offset, data, data_size);
        *offset += leanclr_get_stack_object_size_of_byte_size(data_size);
    }

    void leanclr_get_return_value(const LeanclrStackObject* ret_buff, void* data, size_t data_size)
    {
        std::memcpy(data, ret_buff, data_size);
    }

    void leanclr_set_return_value(LeanclrStackObject* eval_stack, const void* data, size_t data_size)
    {
        std::memcpy(eval_stack, data, data_size);
    }

    size_t leanclr_get_total_arg_stack_object_size(const LeanclrMethodInfo* method)
    {
        return vm::Method::get_total_arg_stack_object_size((const metadata::RtMethodInfo*)method);
    }

    size_t leanclr_get_return_value_stack_object_size(const LeanclrMethodInfo* method)
    {
        return vm::Method::get_return_value_stack_object_size((const metadata::RtMethodInfo*)method);
    }

    void leanclr_register_pinvoke_func(const char* name, LeanclrMethodPointer func, LeanclrMethodInvoker invoker)
    {
        const char* owned_name;
        metadata::RtInvokeMethodPointer bridge;
        if (public_impl::prepare_invoker(public_impl::InvokerKind::PInvoke, name, invoker, owned_name, bridge))
            vm::PInvokes::register_pinvoke(owned_name, func, bridge);
    }

    void leanclr_register_internal_call_func(const char* name, LeanclrMethodPointer func, LeanclrMethodInvoker invoker)
    {
        const char* owned_name;
        metadata::RtInvokeMethodPointer bridge;
        if (public_impl::prepare_invoker(public_impl::InvokerKind::InternalCall, name, invoker, owned_name, bridge))
            vm::InternalCalls::register_internal_call(owned_name, func, bridge);
    }

    void leanclr_register_newobj_internal_call_func(const char* name, LeanclrMethodInvoker invoker)
    {
        const char* owned_name;
        metadata::RtInvokeMethodPointer bridge;
        if (public_impl::prepare_invoker(public_impl::InvokerKind::NewObjectInternalCall, name, invoker, owned_name, bridge))
            vm::InternalCalls::register_newobj_internal_call(owned_name, bridge);
    }

    void leanclr_register_intrinsic_func(const char* name, LeanclrMethodPointer func, LeanclrMethodInvoker invoker)
    {
        const char* owned_name;
        metadata::RtInvokeMethodPointer bridge;
        if (public_impl::prepare_invoker(public_impl::InvokerKind::Intrinsic, name, invoker, owned_name, bridge))
            vm::Intrinsics::register_intrinsic(owned_name, func, bridge);
    }

    void leanclr_register_newobj_intrinsic_func(const char* name, LeanclrMethodInvoker invoker)
    {
        const char* owned_name;
        metadata::RtInvokeMethodPointer bridge;
        if (public_impl::prepare_invoker(public_impl::InvokerKind::NewObjectIntrinsic, name, invoker, owned_name, bridge))
            vm::Intrinsics::register_newobj_intrinsic(owned_name, bridge);
    }

    void leanclr_invoke_with_buffer(const LeanclrMethodInfo* method, const LeanclrStackObject* arg_buff, LeanclrStackObject* ret_buff,
                                    LeanclrException** out_exception)
    {
        auto* target = reinterpret_cast<const metadata::RtMethodInfo*>(method);
        const bool invalid = !target || (target->total_arg_stack_object_size && !arg_buff) || (target->ret_stack_object_size && !ret_buff);
        auto ret = invalid ? RtResultVoid(RtErr::ArgumentNull)
                           : vm::Runtime::invoke_stackobject_arguments_with_run_cctor(target, reinterpret_cast<const interp::RtStackObject*>(arg_buff),
                                                                                      reinterpret_cast<interp::RtStackObject*>(ret_buff));
        if (ret.is_ok())
        {
            if (out_exception)
                *out_exception = nullptr;
        }
        else
        {
            if (out_exception)
            {
                *(vm::RtException**)out_exception = vm::Exception::raise_error_as_exception(ret.unwrap_err(), nullptr, nullptr);
            }
        }
    }

#ifdef __cplusplus
}
#endif


#include "public/leanclr.hpp"

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

namespace leanclr
{

size_t RuntimeApi::get_total_arg_stack_object_size(const metadata::RtMethodInfo* method)
{
    return vm::Method::get_total_arg_stack_object_size(method);
}

size_t RuntimeApi::get_return_value_stack_object_size(const metadata::RtMethodInfo* method)
{
    return vm::Method::get_return_value_stack_object_size(method);
}

RtResultVoid RuntimeApi::invoke_with_buffer(const metadata::RtMethodInfo* method, interp::RtStackObject* arg_buff, interp::RtStackObject* ret_buff)
{
    return vm::Runtime::invoke_stackobject_arguments_with_run_cctor(method, arg_buff, ret_buff);
}

void RuntimeApi::register_pinvoke_func(const char* name, vm::PInvokeFunction func, vm::PInvokeInvoker invoker)
{
    vm::PInvokes::register_pinvoke(name, func, invoker);
}

void RuntimeApi::register_internal_call_func(const char* name, vm::InternalCallFunction func, vm::InternalCallInvoker invoker)
{
    vm::InternalCalls::register_internal_call(name, func, invoker);
}

void RuntimeApi::register_newobj_internal_call_func(const char* name, vm::InternalCallInvoker invoker)
{
    vm::InternalCalls::register_newobj_internal_call(name, invoker);
}

void RuntimeApi::register_intrinsic_func(const char* name, vm::IntrinsicFunction func, vm::IntrinsicInvoker invoker)
{
    vm::Intrinsics::register_intrinsic(name, func, invoker);
}

void RuntimeApi::register_newobj_intrinsic_func(const char* name, vm::IntrinsicInvoker invoker)
{
    vm::Intrinsics::register_newobj_intrinsic(name, invoker);
}

RtErr RuntimeApi::initialize_runtime()
{
    auto ret = vm::Runtime::initialize();
    if (ret.is_ok())
    {
        return RtErr::None;
    }
    else
    {
        return ret.unwrap_err();
    }
}

void RuntimeApi::shutdown_runtime()
{
    vm::Runtime::shutdown();
}

RtResultVoid RuntimeApi::get_assemblies(utils::Vector<metadata::RtAssembly*>& out_assemblies)
{
    out_assemblies.clear();
    auto modules = metadata::RtModuleDef::get_registered_modules();
    for (auto* mod : modules)
    {
        out_assemblies.push_back(mod->get_assembly());
    }
    RET_VOID_OK();
}

RtResult<metadata::RtAssembly*> RuntimeApi::get_assembly(const char* assembly_name)
{
    auto* mod = metadata::RtModuleDef::find_module(assembly_name);
    auto ass = mod ? mod->get_assembly() : nullptr;
    RET_OK(ass);
}

RtResult<metadata::RtAssembly*> RuntimeApi::load_assembly(const char* assembly_name)
{
    return vm::Assembly::load_by_name(assembly_name);
}

metadata::RtModuleDef* RuntimeApi::get_assembly_by_module(metadata::RtAssembly* ass)
{
    return ass->mod;
}

metadata::RtAssembly* RuntimeApi::get_module_by_assembly(metadata::RtModuleDef* mod)
{
    return mod->get_assembly();
}

bool RuntimeApi::is_corlib(metadata::RtModuleDef* mod)
{
    return mod->is_corlib();
}

const char* RuntimeApi::get_module_name_noext(metadata::RtModuleDef* mod)
{
    return mod->get_name_no_ext();
}

RtResultVoid RuntimeApi::get_classes(metadata::RtModuleDef* mod, bool export_only, utils::Vector<metadata::RtClass*>& classes)
{
    return mod->get_types(export_only, classes);
}

RtResult<metadata::RtClass*> RuntimeApi::get_class_by_name(metadata::RtModuleDef* mod, const char* full_name, bool ignore_case)
{
    return mod->get_class_by_name(full_name, ignore_case, false);
}

metadata::RtModuleDef* RuntimeApi::get_class_module(const metadata::RtClass* klass)
{
    return klass->image;
}

const char* RuntimeApi::get_class_namespace(const metadata::RtClass* klass)
{
    return klass->namespaze;
}

const char* RuntimeApi::get_class_name(const metadata::RtClass* klass)
{
    return klass->name;
}

const metadata::RtTypeSig* RuntimeApi::get_class_byval_typesig(const metadata::RtClass* klass)
{
    return klass->by_val;
}

const metadata::RtTypeSig* RuntimeApi::get_class_byref_typesig(const metadata::RtClass* klass)
{
    return klass->by_ref;
}

metadata::RtClass* RuntimeApi::get_class_parent(const metadata::RtClass* klass)
{
    return const_cast<metadata::RtClass*>(klass->parent);
}

RtResultVoid RuntimeApi::get_class_interfaces(const metadata::RtClass* klass, const metadata::RtClass**& interfaces, size_t& count)
{
    RET_ERR_ON_FAIL(vm::Class::initialize_interfaces(const_cast<metadata::RtClass*>(klass)));
    count = klass->interface_count;
    interfaces = (const metadata::RtClass**)klass->interfaces;
    RET_VOID_OK();
}

metadata::RtClass* RuntimeApi::get_class_enclosing_class(const metadata::RtClass* klass)
{
    return const_cast<metadata::RtClass*>(klass->declaring_class);
}

RtResultVoid RuntimeApi::get_class_nested_classes(const metadata::RtClass* klass, const metadata::RtClass**& nested_classes, size_t& count)
{
    RET_ERR_ON_FAIL(vm::Class::initialize_nested_classes(const_cast<metadata::RtClass*>(klass)));
    count = klass->nested_class_count;
    nested_classes = (const metadata::RtClass**)klass->nested_classes;
    RET_VOID_OK();
}

RtResultVoid RuntimeApi::get_class_methods(const metadata::RtClass* klass, const metadata::RtMethodInfo**& methods, size_t& count)
{
    RET_ERR_ON_FAIL(vm::Class::initialize_methods(const_cast<metadata::RtClass*>(klass)));
    count = klass->method_count;
    methods = (const metadata::RtMethodInfo**)klass->methods;
    RET_VOID_OK();
}

RtResult<const metadata::RtMethodInfo*> RuntimeApi::get_class_method_by_name(const metadata::RtClass* klass, const char* method_name)
{
    RET_ERR_ON_FAIL(vm::Class::initialize_methods(const_cast<metadata::RtClass*>(klass)));
    RET_OK(vm::Class::get_method_for_name(klass, method_name, -1, false));
}

RtResultVoid RuntimeApi::get_class_field(const metadata::RtClass* klass, const metadata::RtFieldInfo*& fields, size_t& count)
{
    RET_ERR_ON_FAIL(vm::Class::initialize_fields(const_cast<metadata::RtClass*>(klass)));
    count = klass->field_count;
    fields = klass->fields;
    RET_VOID_OK();
}

RtResult<const metadata::RtFieldInfo*> RuntimeApi::get_class_field_by_name(const metadata::RtClass* klass, const char* field_name)
{
    RET_ERR_ON_FAIL(vm::Class::initialize_fields(const_cast<metadata::RtClass*>(klass)));
    RET_OK(vm::Class::get_field_for_name(klass, field_name, false));
}

RtResultVoid RuntimeApi::get_class_properties(const metadata::RtClass* klass, const metadata::RtPropertyInfo*& properties, size_t& count)
{
    RET_ERR_ON_FAIL(vm::Class::initialize_properties(const_cast<metadata::RtClass*>(klass)));
    count = klass->property_count;
    properties = klass->properties;
    RET_VOID_OK();
}

RtResult<const metadata::RtPropertyInfo*> RuntimeApi::get_class_property_by_name(const metadata::RtClass* klass, const char* property_name)
{
    RET_ERR_ON_FAIL(vm::Class::initialize_properties(const_cast<metadata::RtClass*>(klass)));
    RET_OK(vm::Class::get_property_for_name(klass, property_name, false));
}

RtResultVoid RuntimeApi::get_class_events(const metadata::RtClass* klass, const metadata::RtEventInfo*& events, size_t& count)
{
    RET_ERR_ON_FAIL(vm::Class::initialize_events(const_cast<metadata::RtClass*>(klass)));
    count = klass->event_count;
    events = klass->events;
    RET_VOID_OK();
}

RtResult<const metadata::RtEventInfo*> RuntimeApi::get_class_event_by_name(const metadata::RtClass* klass, const char* event_name)
{
    RET_ERR_ON_FAIL(vm::Class::initialize_events(const_cast<metadata::RtClass*>(klass)));
    RET_OK(vm::Class::get_event_for_name(klass, event_name, false));
}
} // namespace leanclr

#include "delegate.h"
#include "rt_managed_types.h"
#include "method.h"
#include "object.h"
#include "rt_array.h"
#include "class.h"
#include "reflection.h"
#include "runtime.h"
#include "type.h"
#include "assembly.h"
#include "generic_class.h"
#include "interp/eval_stack_op.h"
#include "interp/machine_state.h"
#include "metadata/metadata_cache.h"
#include "metadata/module_def.h"
#include "utils/hashmap.h"

#include <cstdio>
#include <cstdlib>

namespace leanclr
{
namespace vm
{
constexpr size_t MAX_DELEGATE_RESULT_OBJECT_SIZE = 1024;

namespace
{

struct AsyncDelegateResult
{
    uint16_t stack_object_size = 0;
    interp::RtStackObject ret_buffer[MAX_DELEGATE_RESULT_OBJECT_SIZE];
};

utils::HashMap<RtObject*, AsyncDelegateResult> s_asyncDelegateResults;
static interp::RtStackObject s_tempReturnValueBuffer[MAX_DELEGATE_RESULT_OBJECT_SIZE];

static RtResultVoid invoke_delegate_target_with_roots(const metadata::RtMethodInfo* target_method,
                                                      interp::RtStackObject* args) noexcept
{
    interp::MachineState& machine_state = interp::MachineState::get_global_machine_state();
    uint32_t old_frame_top = machine_state.enter_frame_from_icall_or_intrinsic(
        target_method, args, static_cast<uint32_t>(target_method->total_arg_stack_object_size),
        interp::InterpFrameRootScanMode::MethodArguments);
    RtResultVoid result = CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(target_method->invoke_method_ptr)(
        target_method->method_ptr, target_method, args, s_tempReturnValueBuffer);
    machine_state.leave_frame_from_icall_or_intrinsic(old_frame_top);
    return result;
}

static bool is_object_array_func_invoke_target(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr || method->parent == nullptr || method->parameter_count != 1 || std::strcmp(method->name, "Invoke") != 0)
    {
        return false;
    }
    if (std::strcmp(method->parent->namespaze, "System") != 0 || std::strcmp(method->parent->name, "Func`2") != 0)
    {
        return false;
    }

    auto param_class_ret = Class::get_class_from_typesig(method->parameters[0]);
    if (param_class_ret.is_err())
    {
        return false;
    }
    metadata::RtClass* param_class = param_class_ret.unwrap();
    return Class::is_szarray_class(param_class) && param_class->element_class == Class::get_corlib_types().cls_object;
}

static RtResult<RtArray*> create_object_array_for_delegate_args(const metadata::RtMethodInfo* delegate_invoke_method,
                                                               const interp::RtStackObject* args) noexcept
{
    const int32_t arg_count = static_cast<int32_t>(delegate_invoke_method->parameter_count);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
        RtArray*, arg_array,
        LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(Class::get_corlib_types().cls_object, arg_count, "Delegate::create_object_array_for_delegate_args"));

    for (int32_t i = 0; i < arg_count; ++i)
    {
        const metadata::RtTypeSig* param_type_sig = delegate_invoke_method->parameters[i];
        metadata::RtTypeSig byval_type_sig;
        if (param_type_sig->by_ref)
        {
            byval_type_sig = param_type_sig->to_canonized_without_byref();
            param_type_sig = &byval_type_sig;
        }

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, param_class, Class::get_class_from_typesig(param_type_sig));

        RtObject* boxed_arg = nullptr;
        if (Class::is_value_type(param_class))
        {
            const void* value_ptr = delegate_invoke_method->parameters[i]->by_ref ? args[i + 1].ptr : static_cast<const void*>(&args[i + 1]);
            if (value_ptr == nullptr)
            {
                RET_ERR(RtErr::NullReference);
            }
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, boxed_value,
                                                    LEANCLR_BOX_OBJECT_INTERNAL(param_class, value_ptr, "Delegate::create_object_array_for_delegate_args"));
            boxed_arg = boxed_value;
        }
        else
        {
            if (delegate_invoke_method->parameters[i]->by_ref)
            {
                void* value_ptr = args[i + 1].ptr;
                boxed_arg = value_ptr != nullptr ? *reinterpret_cast<RtObject**>(value_ptr) : nullptr;
            }
            else
            {
                boxed_arg = args[i + 1].obj;
            }
        }

        Array::set_array_data_at<RtObject*>(arg_array, i, boxed_arg);
    }

    RET_OK(arg_array);
}

static RtResultVoid unbox_object_array_thunk_return(const metadata::RtMethodInfo* delegate_invoke_method) noexcept
{
    if (delegate_invoke_method->ret_stack_object_size == 0)
    {
        RET_VOID_OK();
    }

    const metadata::RtTypeSig* return_type_sig = delegate_invoke_method->return_type;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, return_class, Class::get_class_from_typesig(return_type_sig));
    RtObject* result_obj = s_tempReturnValueBuffer[0].obj;
    if (Class::is_value_type(return_class))
    {
        if (result_obj == nullptr)
        {
            RET_ERR(RtErr::NullReference);
        }
        RET_ERR_ON_FAIL(Object::unbox_any(result_obj, return_class, s_tempReturnValueBuffer, true));
    }
    else
    {
        s_tempReturnValueBuffer[0].obj = result_obj;
    }

    RET_VOID_OK();
}

RtResult<metadata::RtClass*> get_completed_task_class_for_result(const metadata::RtTypeSig* result_type)
{
    metadata::RtModuleDef* corlib = Assembly::get_corlib()->mod;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, task_def,
                                            corlib->get_class_by_name("System.Threading.Tasks.Task`1", false, true));
    const metadata::RtTypeSig* generic_args[1] = {result_type};
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtGenericInst*, task_inst,
                                            metadata::MetadataCache::get_pooled_generic_inst(generic_args, 1));
    return GenericClass::get_class(Class::get_type_def_gid(task_def), task_inst);
}

RtResult<RtObject*> create_completed_task_for_result(const metadata::RtMethodInfo* invoke_method, interp::RtStackObject* result_buffer)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, task_klass,
                                            get_completed_task_class_for_result(invoke_method->return_type));
    RET_ERR_ON_FAIL(Class::initialize_all(task_klass));
    const metadata::RtMethodInfo* ctor = Class::get_method_for_name(task_klass, ".ctor", 1, false);
    if (ctor == nullptr)
    {
        RET_ERR(RtErr::MissingMethod);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, task_obj, LEANCLR_NEWOBJ_INTERNAL(task_klass, "Delegate::create_completed_task_for_result"));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_value_type, Type::is_value_type(invoke_method->return_type));
    const void* ctor_args[1] = {is_value_type ? static_cast<const void*>(result_buffer)
                                              : static_cast<const void*>(interp::EvalStackOp::get_param<RtObject*>(result_buffer, 0))};
    RET_ERR_ON_FAIL(Runtime::invoke_with_run_cctor(ctor, task_obj, ctor_args));
    RET_OK(task_obj);
}

} // namespace

RtResultVoid Delegate::initialize()
{
    RET_VOID_OK();
}

const metadata::RtMethodInfo* Delegate::get_target_method(const RtDelegate* del) noexcept
{
    return del == nullptr ? nullptr : reinterpret_cast<const metadata::RtMethodInfo*>(del->method_ptr);
}

void Delegate::set_target_method(RtDelegate* del, const metadata::RtMethodInfo* method) noexcept
{
    assert(del != nullptr);
    del->method_ptr = reinterpret_cast<uintptr_t>(method);
    del->method_ptr_aux = del->target != nullptr && Method::is_instance(method) ? 0 : reinterpret_cast<uintptr_t>(method);
}

RtResult<RtMulticastDelegate*> Delegate::create_delegate_from_reflection(RtReflectionType* delegate_type, RtObject* target,
                                                                         const metadata::RtMethodInfo* method, bool throw_on_bind) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig,
                                            Reflection::get_type_sig_from_reflection_type_object(delegate_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, delegate_klass, vm::Class::get_class_from_typesig(type_sig));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, del_obj, LEANCLR_NEWOBJ_INTERNAL(delegate_klass, "Delegate::create_delegate_from_reflection"));
    RtMulticastDelegate* del = reinterpret_cast<RtMulticastDelegate*>(del_obj);

    RET_ERR_ON_FAIL(constructor_delegate(del, target, method));
    bool is_vir_method = !Method::is_devirtualed(method);
    if (is_vir_method && target)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, virtual_method, Method::get_virtual_method_impl(target, method));
        set_target_method(&del->dele, virtual_method);
    }

    RET_OK(del);
}

RtResultVoid Delegate::constructor_delegate(RtMulticastDelegate* del, RtObject* target, const metadata::RtMethodInfo* method) noexcept
{
    del->invocation_list = nullptr;
    del->invocation_count = 0;
    auto& sub_del = del->dele;
    sub_del.target = target;
    sub_del.method_base = nullptr;
    set_target_method(&sub_del, method);
    RET_VOID_OK();
}

RtResult<RtMulticastDelegate*> Delegate::new_delegate(const metadata::RtClass* delelgate_type, RtObject* target, const metadata::RtMethodInfo* method) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, del_obj, LEANCLR_NEWOBJ_INTERNAL(delelgate_type, "Delegate::new_delegate"));
    RtMulticastDelegate* del = reinterpret_cast<RtMulticastDelegate*>(del_obj);
    RET_ERR_ON_FAIL(constructor_delegate(del, target, method));
    RET_OK(del);
}

// Placeholder delegate invokers (to be implemented)
RtResultVoid Delegate::call_delegate_ctor_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    RtMulticastDelegate* del_obj = interp::EvalStackOp::get_param<RtMulticastDelegate*>(params, 0);
    RtObject* target = interp::EvalStackOp::get_param<RtObject*>(params, 1);
    auto method_arg = interp::EvalStackOp::get_param<const void*>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method_info,
                                            Reflection::get_method_info_from_handle_arg(method_arg));
    RET_ERR_ON_FAIL(constructor_delegate(del_obj, target, method_info));
    interp::EvalStackOp::set_return(ret, del_obj);
    RET_VOID_OK();
}

RtResultVoid Delegate::newobj_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    RtObject* target = interp::EvalStackOp::get_param<RtObject*>(params, 0);
    auto method_arg = interp::EvalStackOp::get_param<const void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, method_info,
                                            Reflection::get_method_info_from_handle_arg(method_arg));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtMulticastDelegate*, del, new_delegate(method->parent, target, method_info));
    interp::EvalStackOp::set_return(ret, del);
    RET_VOID_OK();
}

RtResultVoid Delegate::invoke_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    interp::RtStackObject* args = const_cast<interp::RtStackObject*>(params);
    RtMulticastDelegate* del = interp::EvalStackOp::get_param<RtMulticastDelegate*>(args, 0);
    if (!del)
    {
        RET_ERR(RtErr::NullReference);
    }

    RtDelegate* temp_delegate_arr[1];
    RtDelegate** del_arr;
    size_t del_count;
    if (del->invocation_list != nullptr && Class::is_array_or_szarray(del->invocation_list->klass))
    {
        RtArray* invocation_array = reinterpret_cast<RtArray*>(del->invocation_list);
        del_arr = Array::get_array_data_start_as<RtDelegate*>(invocation_array);
        int32_t array_length = Array::get_array_length(invocation_array);
        int32_t invocation_count = del->invocation_count > 0 && del->invocation_count <= array_length ? static_cast<int32_t>(del->invocation_count)
                                                                                                      : array_length;
        del_count = static_cast<size_t>(invocation_count);
    }
    else if (del->invocation_list != nullptr)
    {
        temp_delegate_arr[0] = reinterpret_cast<RtDelegate*>(del->invocation_list);
        del_arr = temp_delegate_arr;
        del_count = 1;
    }
    else
    {
        temp_delegate_arr[0] = &del->dele;
        del_arr = temp_delegate_arr;
        del_count = 1;
    }

    int32_t delegate_param_count = method->parameter_count;
    for (size_t i = 0; i < del_count; ++i)
    {
        RtDelegate* curr_del = del_arr[i];
        const metadata::RtMethodInfo* target_method = get_target_method(curr_del);
        if (target_method == nullptr)
        {
            RET_ERR(RtErr::ExecutionEngine);
        }
        RtObject* target_obj = curr_del->target;
        if (target_obj != nullptr && delegate_param_count > target_method->parameter_count && is_object_array_func_invoke_target(target_method))
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, arg_array, create_object_array_for_delegate_args(method, args));
            interp::RtStackObject thunk_args[2] = {};
            interp::EvalStackOp::set_param(thunk_args, 0, target_obj);
            interp::EvalStackOp::set_param(thunk_args, 1, reinterpret_cast<RtObject*>(arg_array));
            if (std::getenv("LEANCLR_DYNAMIC_TRACE") != nullptr)
            {
                std::fprintf(stderr,
                             "leanclr-delegate: object-array-thunk invoke=%s.%s::%s delegate_params=%d target=%s.%s::%s args=%p array=%p\n",
                             method->parent->namespaze, method->parent->name, method->name, delegate_param_count,
                             target_method->parent->namespaze, target_method->parent->name, target_method->name, static_cast<void*>(thunk_args),
                             static_cast<void*>(arg_array));
            }
            RET_ERR_ON_FAIL(invoke_delegate_target_with_roots(target_method, thunk_args));
            RET_ERR_ON_FAIL(unbox_object_array_thunk_return(method));
            continue;
        }
        interp::RtStackObject* final_args;
        switch (delegate_param_count - (int32_t)target_method->parameter_count)
        {
        case 0:
        {
            if (Method::is_instance(target_method))
            {
                if (!target_obj)
                {
                    RET_ERR(RtErr::NullReference);
                }
                if (Class::is_value_type(target_method->parent))
                {
                    // adjust this pointer
                    target_obj += 1;
                }
                interp::EvalStackOp::set_param(args, 0, target_obj);
                final_args = args;
            }
            else
            {
                final_args = args + 1; // Skip the first delegate parameter
            }
            break;
        }
        case 1:
        {
            assert(Method::is_instance(target_method));
            RtObject* this_obj = interp::EvalStackOp::get_param<RtObject*>(args, 1);
            if (!this_obj)
            {
                RET_ERR(RtErr::NullReference);
            }
            final_args = args + 1; // Skip the first delegate parameter
            break;
        }
        case -1:
        {
            assert(!Method::is_instance(target_method));
            interp::EvalStackOp::set_param(args, 0, target_obj);
            final_args = args;
            break;
        }
        default:
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
        if (std::getenv("LEANCLR_DYNAMIC_TRACE") != nullptr)
        {
            std::fprintf(stderr,
                         "leanclr-delegate: invoke=%s.%s::%s delegate_params=%d target=%s.%s::%s target_params=%u final_args=%p arg0=%p arg1=%p arg2=%p\n",
                         method->parent->namespaze, method->parent->name, method->name, delegate_param_count,
                         target_method->parent->namespaze, target_method->parent->name, target_method->name, target_method->parameter_count,
                         static_cast<void*>(final_args),
                         final_args != nullptr ? final_args[0].ptr : nullptr,
                         final_args != nullptr ? final_args[1].ptr : nullptr,
                         final_args != nullptr ? final_args[2].ptr : nullptr);
        }
        RET_ERR_ON_FAIL(invoke_delegate_target_with_roots(target_method, final_args));
    }
    // If there is a return value, set it to the ret buffer
    if (method->ret_stack_object_size > 0)
    {
        std::memcpy(ret, s_tempReturnValueBuffer, method->ret_stack_object_size * sizeof(interp::RtStackObject));
    }
    RET_VOID_OK();
}

RtResultVoid Delegate::begin_invoke_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)method_pointer;
    RET_ERR_ON_FAIL(Class::initialize_methods(const_cast<metadata::RtClass*>(method->parent)));
    const metadata::RtMethodInfo* invoke_method = Class::get_method_for_name(method->parent, "Invoke", -1, false);
    if (invoke_method == nullptr)
    {
        RET_ERR(RtErr::MissingMethod);
    }
    if (invoke_method->ret_stack_object_size > MAX_DELEGATE_RESULT_OBJECT_SIZE)
    {
        RET_ERR(RtErr::NotSupported);
    }

    interp::RtStackObject result_buffer[MAX_DELEGATE_RESULT_OBJECT_SIZE];
    std::memset(result_buffer, 0, sizeof(result_buffer));
    RET_ERR_ON_FAIL(invoke_delegate_invoker(nullptr, invoke_method, params, result_buffer));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, task_obj, create_completed_task_for_result(invoke_method, result_buffer));
    AsyncDelegateResult stored_result{};
    stored_result.stack_object_size = invoke_method->ret_stack_object_size;
    if (invoke_method->ret_stack_object_size > 0)
    {
        std::memcpy(stored_result.ret_buffer, result_buffer, invoke_method->ret_stack_object_size * sizeof(interp::RtStackObject));
    }
    s_asyncDelegateResults[task_obj] = stored_result;
    interp::EvalStackOp::set_return(ret, task_obj);
    RET_VOID_OK();
}

RtResultVoid Delegate::end_invoke_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)method_pointer;
    if (method->parameter_count <= 0)
    {
        RET_ERR(RtErr::Argument);
    }
    RtObject* async_result = interp::EvalStackOp::get_param<RtObject*>(params, method->parameter_count);
    auto it = s_asyncDelegateResults.find(async_result);
    if (it == s_asyncDelegateResults.end())
    {
        RET_ERR(RtErr::Argument);
    }
    const AsyncDelegateResult& result = it->second;
    if (method->ret_stack_object_size > 0)
    {
        if (result.stack_object_size < method->ret_stack_object_size)
        {
            RET_ERR(RtErr::ExecutionEngine);
        }
        std::memcpy(ret, result.ret_buffer, method->ret_stack_object_size * sizeof(interp::RtStackObject));
    }
    RET_VOID_OK();
}
} // namespace vm
} // namespace leanclr

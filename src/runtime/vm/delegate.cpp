#include "delegate.h"
#include "rt_managed_types.h"
#include "method.h"
#include "object.h"
#include "rt_array.h"
#include "class.h"
#include "interp/eval_stack_op.h"

namespace leanclr
{
namespace vm
{
RtResultVoid Delegate::initialize()
{
    RET_VOID_OK();
}

RtResult<RtMulticastDelegate*> Delegate::create_delegate_from_reflection(RtReflectionType* delegate_type, RtObject* target,
                                                                         const metadata::RtMethodInfo* method, bool throw_on_bind) noexcept
{
    const metadata::RtTypeSig* type_sig = delegate_type->type_handle;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, delegate_klass, vm::Class::get_class_from_typesig(type_sig));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, del_obj, Object::new_object(delegate_klass));
    RtDelegate& sub_del = reinterpret_cast<RtMulticastDelegate*>(del_obj)->dele;

    sub_del.target = target;
    bool is_vir_method = !Method::is_devirtualed(method);
    if (is_vir_method && target)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(sub_del.method, Method::get_virtual_method_impl(target, method));
        sub_del.method_is_virtual = false;
    }
    else
    {
        sub_del.method = method;
        sub_del.method_is_virtual = is_vir_method;
    }

    RET_OK(reinterpret_cast<RtMulticastDelegate*>(del_obj));
}

RtResultVoid Delegate::constructor_delegate(RtMulticastDelegate* del, RtObject* target, const metadata::RtMethodInfo* method) noexcept
{
    assert(!del->deles);
    auto& sub_del = del->dele;
    sub_del.target = target;
    sub_del.method = method;
    sub_del.method_is_virtual = false;
    RET_VOID_OK();
}

RtResult<RtMulticastDelegate*> Delegate::new_delegate(const metadata::RtClass* delelgate_type, RtObject* target, const metadata::RtMethodInfo* method) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, del_obj, Object::new_object(delelgate_type));
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
    const metadata::RtMethodInfo* method_info = interp::EvalStackOp::get_param<const metadata::RtMethodInfo*>(params, 2);
    RET_ERR_ON_FAIL(constructor_delegate(del_obj, target, method_info));
    interp::EvalStackOp::set_return(ret, del_obj);
    RET_VOID_OK();
}

RtResultVoid Delegate::newobj_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    RtObject* target = interp::EvalStackOp::get_param<RtObject*>(params, 0);
    const metadata::RtMethodInfo* method_info = interp::EvalStackOp::get_param<const metadata::RtMethodInfo*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtMulticastDelegate*, del, new_delegate(method->parent, target, method_info));
    interp::EvalStackOp::set_return(ret, del);
    RET_VOID_OK();
}

constexpr size_t MAX_DELEGATE_RESULT_OBJECT_SIZE = 1024;
static interp::RtStackObject s_tempReturnValueBuffer[MAX_DELEGATE_RESULT_OBJECT_SIZE];

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
    if (del->deles)
    {
        del_arr = Array::get_array_data_start_as<RtDelegate*>(del->deles);
        del_count = static_cast<size_t>(Array::get_array_length(del->deles));
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
        const metadata::RtMethodInfo* target_method = curr_del->method;
        RtObject* target_obj = curr_del->target;
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
        RET_ERR_ON_FAIL(CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(target_method->invoke_method_ptr)(target_method->method_ptr, target_method, final_args,
                                                                                                s_tempReturnValueBuffer));
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
    RETURN_NOT_IMPLEMENTED_ERROR();
}

RtResultVoid Delegate::end_invoke_delegate_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}
} // namespace vm
} // namespace leanclr

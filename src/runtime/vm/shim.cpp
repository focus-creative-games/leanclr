#include "shim.h"
#include "method.h"
#include "rt_array.h"
#include "class.h"
#include "delegate.h"
#include "internal_calls.h"
#include "intrinsics.h"
#include "pinvoke.h"
#include "const_strs.h"
#include "method.h"
#include "interp/interpreter.h"
#include "utils/string_builder.h"
#include "metadata/metadata_name.h"
#include "metadata/module_def.h"
#include "metadata/aot_module.h"

namespace leanclr
{
namespace vm
{

// Static function pointers for invokers
namespace
{

// Interpreter invoker
RtResultVoid fn_interpreter_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                    interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const interp::RtStackObject*, result, interp::Interpreter::execute(method, params));
    if (method->ret_stack_object_size > 0)
    {
        std::memcpy(ret, result, method->ret_stack_object_size * sizeof(interp::RtStackObject));
    }
    RET_VOID_OK();
}

// Interpreter virtual adjust thunk invoker
RtResultVoid fn_interpreter_virtual_adjust_thunk_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    const_cast<interp::RtStackObject*>(params)[0].obj = params[0].obj + 1;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const interp::RtStackObject*, result, interp::Interpreter::execute(method, params));
    if (method->ret_stack_object_size > 0)
    {
        std::memcpy(ret, result, method->ret_stack_object_size * sizeof(interp::RtStackObject));
    }
    RET_VOID_OK();
}

// Not implemented internal call invoker
RtResultVoid fn_not_implemented_internal_call_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                      const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
#if LEANCLR_DEBUG
    utils::StringBuilder sb;
    RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method));
    printf("Internal call invoker not implemented for method: %s token:0x%0x\n", sb.as_cstr(), method->token);
#endif
    RETURN_NOT_IMPLEMENTED_ERROR();
}

// Not implemented intrinsic invoker
RtResultVoid fn_not_implemented_intrinsic_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
#if LEANCLR_DEBUG
    utils::StringBuilder sb;
    RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method));
    printf("Intrinsic invoker not implemented for method: %s token:0x%0x\n", sb.as_cstr(), method->token);
#endif
    RETURN_NOT_IMPLEMENTED_ERROR();
}

// PInvoke invoker (not implemented)
RtResultVoid fn_pinvoke_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
#if LEANCLR_DEBUG
    utils::StringBuilder sb;
    RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method));
    printf("P/Invoke invoker not implemented for method: %s token:0x%0x\n", sb.as_cstr(), method->token);
#endif
    RETURN_NOT_IMPLEMENTED_ERROR();
}

// Not implemented PInvoke invoker
RtResultVoid fn_not_implemented_pinvoke_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
#if LEANCLR_DEBUG
    utils::StringBuilder sb;
    RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method));
    printf("P/Invoke invoker not implemented for method: %s token:0x%0x\n", sb.as_cstr(), method->token);
#endif
    RETURN_NOT_IMPLEMENTED_ERROR();
}

// Not implemented runtime impl invoker
RtResultVoid fn_not_implemented_runtime_impl_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
#if LEANCLR_DEBUG
    utils::StringBuilder sb;
    RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method));
    printf("Runtime impl invoker not implemented for method: %s token:0x%0x\n", sb.as_cstr(), method->token);
#endif
    // Placeholder implementation
    RETURN_NOT_IMPLEMENTED_ERROR();
}

// Not implemented generic invoker
RtResultVoid fn_not_implemented_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
#if LEANCLR_DEBUG
    utils::StringBuilder sb;
    RET_ERR_ON_FAIL(metadata::MetadataName::append_method_full_name_without_params(sb, method));
    printf("Not implemented invoker not implemented for method: %s token:0x%0x\n", sb.as_cstr(), method->token);
#endif
    // Placeholder implementation
    RETURN_NOT_IMPLEMENTED_ERROR();
}

// Method pointer placeholder
void fn_not_implemented_method_pointer() noexcept
{
    // Placeholder - would panic in debug builds
}

} // anonymous namespace

// Helper function to try setting up array or szarray invokers
static metadata::RtInvokeMethodPointer try_setup_array_or_szarray_invoke(const metadata::RtMethodInfo* method)
{
    const metadata::RtClass* klass = method->parent;
    const char* method_name = method->name;
    size_t param_count = method->parameter_count;

    if (Class::is_szarray_class(klass))
    {
        // Single-dimensional array methods
        if (std::strcmp(method_name, STR_CTOR) == 0)
        {
            return Array::szarray_new_invoker;
        }
        else if (std::strcmp(method_name, STR_ARRAY_GET_NZ) == 0)
        {
            assert(param_count == 1);
            return Array::szarray_get_invoker;
        }
        else if (std::strcmp(method_name, STR_ARRAY_SET_NZ) == 0)
        {
            assert(param_count == 2);
            return Array::szarray_set_invoker;
        }
        else if (std::strcmp(method_name, STR_ARRAY_ADDRESS_NZ) == 0)
        {
            assert(param_count == 1);
            return Array::szarray_address_invoker;
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        // Multi-dimensional array methods
        uint8_t rank = klass->by_val->data.array_type->rank;

        if (std::strcmp(method_name, STR_CTOR) == 0)
        {
            assert(param_count == rank || param_count == rank * 2);
            if (param_count == rank)
            {
                return Array::newmdarray_lengths_invoker;
            }
            else
            {
                return Array::newmdarray_lengths_lower_bounds_invoker;
            }
        }
        else if (std::strcmp(method_name, STR_ARRAY_GET_NZ) == 0)
        {
            assert(param_count == rank);
            return Array::mdarray_get_invoker;
        }
        else if (std::strcmp(method_name, STR_ARRAY_SET_NZ) == 0)
        {
            assert(param_count == rank + 1);
            return Array::mdarray_set_invoker;
        }
        else if (std::strcmp(method_name, STR_ARRAY_ADDRESS_NZ) == 0)
        {
            assert(param_count == rank);
            return Array::mdarray_address_invoker;
        }
        else
        {
            return nullptr;
        }
    }
}

// Get invoker for a method
RtResult<InvokeTypeAndMethod> Shim::get_invoker(const metadata::RtMethodInfo* method)
{
    const metadata::RtClass* klass = method->parent;

    // Check for array/szarray methods
    if (Class::is_array_or_szarray(klass))
    {
        if (auto result = try_setup_array_or_szarray_invoke(method))
        {
            RET_OK(InvokeTypeAndMethod(RtInvokerType::CustomInstrinsic, result));
        }
    }

    // Determine invoker based on method implementation type
    metadata::RtMethodImplAttribute code_type = Method::get_code_type(method);

    switch (code_type)
    {
    case metadata::RtMethodImplAttribute::IlOrManaged:
    {
        // Try internal call first

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(const InternalCallRegistry*, icall_entry, InternalCalls::get_internal_call_by_method(method));
        if (icall_entry)
        {
            RET_OK(InvokeTypeAndMethod(RtInvokerType::InternalCall, icall_entry->invoker));
        }

        // Try intrinsic

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(const IntrinsicRegistry*, intrinsic_entry, Intrinsics::get_intrinsic_by_method(method));
        if (intrinsic_entry)
        {
            RET_OK(InvokeTypeAndMethod(RtInvokerType::Intrinsic, intrinsic_entry->invoker));
        }

        std::optional<metadata::RtAotMethodImplData> aot_data = metadata::AotModule::find_aot_method_impl(method);
        if (aot_data.has_value())
        {
            RET_OK(InvokeTypeAndMethod(RtInvokerType::Aot, aot_data->invoke_method_ptr, aot_data->virtual_invoke_method_ptr));
        }

        if (Method::is_internal_call(method))
        {
            RET_OK(InvokeTypeAndMethod(RtInvokerType::InternalCall, fn_not_implemented_internal_call_invoker));
        }

        // DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(bool, is_intrinsic_method, Method::is_intrinsic(method));
        // if (is_intrinsic_method)
        // {
        //     RET_OK(InvokeTypeAndMethod(RtInvokerType::Intrinsic, fn_not_implemented_intrinsic_invoker));
        // }

        if (Method::is_pinvoke(method))
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(const PInvokeRegistry*, entry, PInvokes::get_pinvoke_by_method(method));
            if (entry)
            {
                RET_OK(InvokeTypeAndMethod(RtInvokerType::PInvoke, entry->invoker));
            }
            else
            {
                RET_OK(InvokeTypeAndMethod(RtInvokerType::PInvoke, fn_not_implemented_pinvoke_invoker));
            }
        }

        auto virtual_invoker =
            Method::is_static(method) || !Class::is_value_type(method->parent) ? fn_interpreter_invoker : fn_interpreter_virtual_adjust_thunk_invoker;
        RET_OK(InvokeTypeAndMethod(RtInvokerType::Interpreter, fn_interpreter_invoker, virtual_invoker));
    }

    case metadata::RtMethodImplAttribute::Native:
        RET_OK(InvokeTypeAndMethod(RtInvokerType::NotImplemented, fn_not_implemented_invoker));

    case metadata::RtMethodImplAttribute::Optil:
        RET_OK(InvokeTypeAndMethod(RtInvokerType::NotImplemented, fn_not_implemented_invoker));

    case metadata::RtMethodImplAttribute::Runtime:
    {
        // Check if it's a delegate method
        if (Class::is_multicastdelegate_subclass(klass))
        {
            const char* method_name = method->name;

            if (std::strcmp(method_name, STR_CTOR) == 0)
            {
                RET_OK(InvokeTypeAndMethod(RtInvokerType::RuntimeImpl, Delegate::call_delegate_ctor_invoker));
            }
            else if (std::strcmp(method_name, STR_INVOKE) == 0)
            {
                RET_OK(InvokeTypeAndMethod(RtInvokerType::RuntimeImpl, Delegate::invoke_delegate_invoker));
            }
            else if (std::strcmp(method_name, STR_BEGININVOKE) == 0)
            {
                RET_OK(InvokeTypeAndMethod(RtInvokerType::RuntimeImpl, Delegate::begin_invoke_delegate_invoker));
            }
            else if (std::strcmp(method_name, STR_ENDINVOKE) == 0)
            {
                RET_OK(InvokeTypeAndMethod(RtInvokerType::RuntimeImpl, Delegate::end_invoke_delegate_invoker));
            }
            else
            {
                RET_OK(InvokeTypeAndMethod(RtInvokerType::RuntimeImpl, fn_not_implemented_runtime_impl_invoker));
            }
        }
        else
        {
            RET_OK(InvokeTypeAndMethod(RtInvokerType::RuntimeImpl, fn_not_implemented_runtime_impl_invoker));
        }
    }

    default:
        RET_OK(InvokeTypeAndMethod(RtInvokerType::NotImplemented, fn_not_implemented_invoker));
    }
}

// Get method pointer (for native/unmanaged methods)
metadata::RtManagedMethodPointer Shim::get_method_pointer(const metadata::RtMethodInfo* method)
{
    const metadata::RtAotModuleData* aotModuleData = method->parent->image->get_aot_module_data();
    if (aotModuleData != nullptr)
    {
        std::optional<metadata::RtAotMethodImplData> aot_data = metadata::AotModule::find_aot_method_impl(method);
        if (aot_data.has_value())
        {
            return aot_data->method_ptr;
        }
    }
    // Placeholder - returns not implemented method pointer
    return reinterpret_cast<metadata::RtManagedMethodPointer>(fn_not_implemented_method_pointer);
}

} // namespace vm
} // namespace leanclr

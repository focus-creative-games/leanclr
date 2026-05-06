#include "system_runtime_compilerservices_runtimehelpers.h"

#include "icall_base.h"
#include "vm/rt_array.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/object.h"
#include "vm/runtime.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace icalls
{

RtResultVoid SystemRuntimeCompilerServicesRuntimeHelpers::initialize_array(vm::RtArray* arr, size_t runtime_field_handle) noexcept
{
    if (runtime_field_handle == 0)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    const metadata::RtFieldInfo* field = reinterpret_cast<const metadata::RtFieldInfo*>(runtime_field_handle);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const uint8_t*, rva_data, vm::Field::get_field_rva_data(field));

    if (rva_data == nullptr)
    {
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }

    size_t array_byte_length = vm::Array::get_array_byte_length(arr);
    uint8_t* array_data = vm::Array::get_array_data_start_as<uint8_t>(arr);

    std::memcpy(array_data, rva_data, array_byte_length);

    RET_VOID_OK();
}

RtResult<int32_t> SystemRuntimeCompilerServicesRuntimeHelpers::get_offset_to_string_data() noexcept
{
    RET_OK(vm::String::get_offset_to_string_data());
}

RtResult<vm::RtObject*> SystemRuntimeCompilerServicesRuntimeHelpers::get_object_value(vm::RtObject* obj) noexcept
{
    if (obj == nullptr || !vm::Class::is_value_type(obj->klass))
    {
        RET_OK(obj);
    }
    else
    {
        return vm::Object::clone(obj);
    }
}

RtResultVoid SystemRuntimeCompilerServicesRuntimeHelpers::run_class_constructor(intptr_t type_handle) noexcept
{
    const metadata::RtTypeSig* type_sig = reinterpret_cast<const metadata::RtTypeSig*>(type_handle);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
    return vm::Runtime::run_class_static_constructor(klass);
}

RtResult<bool> SystemRuntimeCompilerServicesRuntimeHelpers::sufficient_execution_stack() noexcept
{
    RET_OK(true);
}

RtResultVoid SystemRuntimeCompilerServicesRuntimeHelpers::run_module_constructor(intptr_t module_handle) noexcept
{
    metadata::RtModuleDef* module = reinterpret_cast<metadata::RtModuleDef*>(module_handle);
    return vm::Runtime::run_module_static_constructor(module);
}

/// @icall: System.Runtime.CompilerServices.RuntimeHelpers::InitializeArray
RtResultVoid initialize_array_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    vm::RtArray* arr = EvalStackOp::get_param<vm::RtArray*>(params, 0);
    size_t runtime_field_handle = EvalStackOp::get_param<size_t>(params, 1);
    return SystemRuntimeCompilerServicesRuntimeHelpers::initialize_array(arr, runtime_field_handle);
}

/// @icall: System.Runtime.CompilerServices.RuntimeHelpers::get_OffsetToStringData
RtResultVoid get_offset_to_string_data_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                               const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, offset, SystemRuntimeCompilerServicesRuntimeHelpers::get_offset_to_string_data());
    EvalStackOp::set_return(ret, offset);
    RET_VOID_OK();
}

/// @icall: System.Runtime.CompilerServices.RuntimeHelpers::GetObjectValue(System.Object)
RtResultVoid get_object_value_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                      interp::RtStackObject* ret) noexcept
{
    vm::RtObject* obj = EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result, SystemRuntimeCompilerServicesRuntimeHelpers::get_object_value(obj));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Runtime.CompilerServices.RuntimeHelpers::RunClassConstructor(System.IntPtr)
RtResultVoid run_class_constructor_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    intptr_t type_handle = EvalStackOp::get_param<intptr_t>(params, 0);
    return SystemRuntimeCompilerServicesRuntimeHelpers::run_class_constructor(type_handle);
}

/// @icall: System.Runtime.CompilerServices.RuntimeHelpers::SufficientExecutionStack
RtResultVoid sufficient_execution_stack_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeCompilerServicesRuntimeHelpers::sufficient_execution_stack());
    EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @icall: System.Runtime.CompilerServices.RuntimeHelpers::RunModuleConstructor(System.IntPtr)
RtResultVoid run_module_constructor_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    intptr_t module_handle = EvalStackOp::get_param<intptr_t>(params, 0);
    return SystemRuntimeCompilerServicesRuntimeHelpers::run_module_constructor(module_handle);
}

// Internal call registry
static vm::InternalCallEntry s_internal_call_entries_system_runtime_compilerservices_runtimehelpers[] = {
    {"System.Runtime.CompilerServices.RuntimeHelpers::InitializeArray",
     (vm::InternalCallFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::initialize_array, initialize_array_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::get_OffsetToStringData",
     (vm::InternalCallFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::get_offset_to_string_data, get_offset_to_string_data_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::GetObjectValue(System.Object)",
     (vm::InternalCallFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::get_object_value, get_object_value_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::RunClassConstructor(System.IntPtr)",
     (vm::InternalCallFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::run_class_constructor, run_class_constructor_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::SufficientExecutionStack",
     (vm::InternalCallFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::sufficient_execution_stack, sufficient_execution_stack_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::RunModuleConstructor(System.IntPtr)",
     (vm::InternalCallFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::run_module_constructor, run_module_constructor_invoker},
};

utils::Span<vm::InternalCallEntry> SystemRuntimeCompilerServicesRuntimeHelpers::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_runtime_compilerservices_runtimehelpers,
                                              sizeof(s_internal_call_entries_system_runtime_compilerservices_runtimehelpers) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

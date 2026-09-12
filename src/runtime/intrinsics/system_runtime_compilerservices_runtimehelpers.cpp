#include "system_runtime_compilerservices_runtimehelpers.h"

#include <climits>
#include <cstring>

#include "interp/interp_defs.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/reflection.h"
#include "vm/rt_array.h"

namespace leanclr
{
namespace intrinsics
{

static RtResult<const metadata::RtTypeSig*> get_single_method_generic_arg(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr || method->generic_method == nullptr || method->generic_method->generic_context.method_inst == nullptr)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    const metadata::RtGenericInst* method_inst = method->generic_method->generic_context.method_inst;
    if (method_inst->generic_arg_count != 1)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    RET_OK(method_inst->generic_args[0]);
}

static RtResult<bool> is_reference_or_contains_references_by_typesig(const metadata::RtTypeSig* type_sig) noexcept
{
    if (type_sig->by_ref)
    {
        RET_OK(false);
    }

    switch (type_sig->ele_type)
    {
    case metadata::RtElementType::Object:
    case metadata::RtElementType::String:
    case metadata::RtElementType::Class:
    case metadata::RtElementType::Array:
    case metadata::RtElementType::SZArray:
        RET_OK(true);

    case metadata::RtElementType::ValueType:
    case metadata::RtElementType::GenericInst:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
        if (vm::Class::is_reference_type(klass))
        {
            RET_OK(true);
        }
        RET_ERR_ON_FAIL(vm::Class::initialize_fields(klass));
        RET_OK(vm::Class::get_has_references(klass));
    }

    default:
        RET_OK(false);
    }
}

static RtResult<bool> is_bitwise_equatable_by_typesig(const metadata::RtTypeSig* type_sig) noexcept
{
    if (type_sig->by_ref)
    {
        RET_OK(false);
    }

    switch (type_sig->ele_type)
    {
    case metadata::RtElementType::Boolean:
    case metadata::RtElementType::Char:
    case metadata::RtElementType::I1:
    case metadata::RtElementType::U1:
    case metadata::RtElementType::I2:
    case metadata::RtElementType::U2:
    case metadata::RtElementType::I4:
    case metadata::RtElementType::U4:
    case metadata::RtElementType::I8:
    case metadata::RtElementType::U8:
    case metadata::RtElementType::I:
    case metadata::RtElementType::U:
        RET_OK(true);

    case metadata::RtElementType::ValueType:
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Class::get_class_from_typesig(type_sig));
        RET_OK(vm::Class::is_enum_type(klass));
    }

    default:
        RET_OK(false);
    }
}

RtResult<const void*> SystemRuntimeCompilerServicesRuntimeHelpers::get_method_table(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    return vm::Reflection::get_net10_method_table(vm::Class::get_by_val_type_sig(obj->klass));
}

RtResult<void*> SystemRuntimeCompilerServicesRuntimeHelpers::get_raw_data(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    if (vm::Class::is_array_or_szarray(obj->klass))
    {
        return &reinterpret_cast<vm::RtArray*>(obj)->length;
    }
    if (vm::Class::is_string_class(obj->klass))
    {
        return &reinterpret_cast<vm::RtString*>(obj)->length;
    }

    RET_OK(reinterpret_cast<uint8_t*>(obj) + sizeof(vm::RtObject));
}

RtResult<bool> SystemRuntimeCompilerServicesRuntimeHelpers::object_has_component_size(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    RET_OK(vm::Class::is_array_or_szarray(obj->klass) || vm::Class::is_string_class(obj->klass));
}

RtResultVoid SystemRuntimeCompilerServicesRuntimeHelpers::initialize_array(vm::RtArray* array, const metadata::RtFieldInfo* field) noexcept
{
    if (array == nullptr || field == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const uint8_t*, rva_data, vm::Field::get_field_rva_data(field));
    if (rva_data == nullptr)
    {
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }

    size_t array_byte_length = vm::Array::get_array_byte_length(array);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, field_size, vm::Field::get_field_size(field));
    if (array_byte_length > field_size)
    {
        RET_ERR(RtErr::Argument);
    }

    uint8_t* array_data = vm::Array::get_array_data_start_as<uint8_t>(array);
    std::memcpy(array_data, rva_data, array_byte_length);
    RET_VOID_OK();
}

RtResult<uint32_t> SystemRuntimeCompilerServicesRuntimeHelpers::get_num_instance_field_bytes(const void* method_table) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtClass*, klass,
                                            vm::Reflection::get_class_from_net10_method_table(method_table));
    RET_ERR_ON_FAIL(vm::Class::initialize_fields(const_cast<metadata::RtClass*>(klass)));
    RET_OK(vm::Class::get_instance_size_without_object_header(klass));
}

RtResult<metadata::RtElementType> SystemRuntimeCompilerServicesRuntimeHelpers::get_primitive_cor_element_type(const void* method_table) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtClass*, klass,
                                            vm::Reflection::get_class_from_net10_method_table(method_table));

    if (vm::Class::is_enum_type(klass))
    {
        RET_OK(klass->element_class->by_val->ele_type);
    }

    RET_OK(klass->by_val->ele_type);
}

RtResult<vm::RtReadOnlySpan<uint8_t>> SystemRuntimeCompilerServicesRuntimeHelpers::create_span(const metadata::RtMethodInfo* method,
                                                                                               const metadata::RtFieldInfo* field) noexcept
{
    if (method == nullptr || method->generic_method == nullptr || method->generic_method->generic_context.method_inst == nullptr)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    const metadata::RtGenericInst* method_inst = method->generic_method->generic_context.method_inst;
    if (method_inst->generic_arg_count != 1)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    if (field == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    const metadata::RtTypeSig* element_type = method_inst->generic_args[0];
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, type_and_size, interp::InterpDefs::get_reduce_type_and_size_by_typesig(element_type));
    if (type_and_size.byte_size == 0)
    {
        RET_ERR(RtErr::Argument);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const uint8_t*, rva_data, vm::Field::get_field_rva_data(field));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, field_size, vm::Field::get_field_size(field));
    if ((field_size % type_and_size.byte_size) != 0)
    {
        RET_ERR(RtErr::Argument);
    }

    size_t length = field_size / type_and_size.byte_size;
    if (length > static_cast<size_t>(INT32_MAX))
    {
        RET_ERR(RtErr::ArgumentOutOfRange);
    }

    vm::RtReadOnlySpan<uint8_t> span{rva_data, static_cast<int32_t>(length)};
    RET_OK(span);
}

RtResult<vm::RtReadOnlySpan<uint8_t>> SystemRuntimeCompilerServicesRuntimeHelpers::inline_array_as_span(void* buffer, int32_t length) noexcept
{
    if (length < 0)
    {
        RET_ERR(RtErr::ArgumentOutOfRange);
    }

    if (buffer == nullptr && length != 0)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    vm::RtReadOnlySpan<uint8_t> span{reinterpret_cast<const uint8_t*>(buffer), length};
    RET_OK(span);
}

RtResult<void*> SystemRuntimeCompilerServicesRuntimeHelpers::inline_array_first_element_ref(void* buffer) noexcept
{
    if (buffer == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    RET_OK(buffer);
}

RtResult<void*> SystemRuntimeCompilerServicesRuntimeHelpers::inline_array_element_ref(const metadata::RtMethodInfo* method, void* buffer,
                                                                                     int32_t index) noexcept
{
    if (buffer == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    if (index < 0 || method == nullptr || method->generic_method == nullptr || method->generic_method->generic_context.method_inst == nullptr)
    {
        RET_ERR(RtErr::ArgumentOutOfRange);
    }

    const metadata::RtGenericInst* method_inst = method->generic_method->generic_context.method_inst;
    if (method_inst->generic_arg_count != 2)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, element_type_and_size,
                                            interp::InterpDefs::get_reduce_type_and_size_by_typesig(method_inst->generic_args[1]));
    auto* element = reinterpret_cast<uint8_t*>(buffer) + (static_cast<size_t>(index) * element_type_and_size.byte_size);
    RET_OK(element);
}

RtResult<bool> SystemRuntimeCompilerServicesRuntimeHelpers::is_bitwise_equatable(const metadata::RtMethodInfo* method) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig, get_single_method_generic_arg(method));
    return is_bitwise_equatable_by_typesig(type_sig);
}

RtResult<bool> SystemRuntimeCompilerServicesRuntimeHelpers::is_reference_or_contains_references(const metadata::RtMethodInfo* method) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig, get_single_method_generic_arg(method));
    return is_reference_or_contains_references_by_typesig(type_sig);
}

/// @intrinsic: System.Runtime.CompilerServices.RuntimeHelpers::GetMethodTable(System.Object)
static RtResultVoid get_method_table_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    vm::RtObject* obj = interp::EvalStackOp::get_param<vm::RtObject*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const void*, method_table,
                                            SystemRuntimeCompilerServicesRuntimeHelpers::get_method_table(obj));
    interp::EvalStackOp::set_return(ret, method_table);
    RET_VOID_OK();
}

/// @intrinsic: System.Runtime.CompilerServices.RuntimeHelpers::GetRawData(System.Object)
static RtResultVoid get_raw_data_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    vm::RtObject* obj = interp::EvalStackOp::get_param<vm::RtObject*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, raw_data,
                                            SystemRuntimeCompilerServicesRuntimeHelpers::get_raw_data(obj));
    interp::EvalStackOp::set_return(ret, raw_data);
    RET_VOID_OK();
}

/// @intrinsic: System.Runtime.CompilerServices.RuntimeHelpers::InitializeArray(System.Array,System.RuntimeFieldHandle)
static RtResultVoid initialize_array_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    (void)ret;
    auto array = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);
    auto field_arg = interp::EvalStackOp::get_param<const void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field,
                                            vm::Reflection::get_field_info_from_handle_arg(field_arg));

    return SystemRuntimeCompilerServicesRuntimeHelpers::initialize_array(array, field);
}

/// @intrinsic: System.Runtime.CompilerServices.RuntimeHelpers::ObjectHasComponentSize(System.Object)
static RtResultVoid object_has_component_size_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                      const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    vm::RtObject* obj = interp::EvalStackOp::get_param<vm::RtObject*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result,
                                            SystemRuntimeCompilerServicesRuntimeHelpers::object_has_component_size(obj));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @intrinsic: System.Runtime.CompilerServices.MethodTable::GetNumInstanceFieldBytes()
static RtResultVoid get_num_instance_field_bytes_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const void* method_table = interp::EvalStackOp::get_param<const void*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, size,
                                            SystemRuntimeCompilerServicesRuntimeHelpers::get_num_instance_field_bytes(method_table));
    interp::EvalStackOp::set_return(ret, size);
    RET_VOID_OK();
}

/// @intrinsic: System.Runtime.CompilerServices.MethodTable::GetPrimitiveCorElementType()
static RtResultVoid get_primitive_cor_element_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const void* method_table = interp::EvalStackOp::get_param<const void*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtElementType, element_type,
                                            SystemRuntimeCompilerServicesRuntimeHelpers::get_primitive_cor_element_type(method_table));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(element_type));
    RET_VOID_OK();
}

/// @intrinsic: System.Runtime.CompilerServices.RuntimeHelpers::CreateSpan<>(System.RuntimeFieldHandle)
static RtResultVoid create_span_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    auto field_arg = interp::EvalStackOp::get_param<const void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field,
                                            vm::Reflection::get_field_info_from_handle_arg(field_arg));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReadOnlySpan<uint8_t>, span,
                                            SystemRuntimeCompilerServicesRuntimeHelpers::create_span(method, field));
    interp::EvalStackOp::set_return(ret, span);
    RET_VOID_OK();
}

/// @intrinsic: System.Runtime.CompilerServices.RuntimeHelpers::InlineArrayAsSpan<,>(TBuffer&,System.Int32)
/// @intrinsic: System.Runtime.CompilerServices.RuntimeHelpers::InlineArrayAsReadOnlySpan<,>(TBuffer&,System.Int32)
static RtResultVoid inline_array_as_span_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    void* buffer = interp::EvalStackOp::get_param<void*>(params, 0);
    int32_t length = interp::EvalStackOp::get_param<int32_t>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReadOnlySpan<uint8_t>, span,
                                            SystemRuntimeCompilerServicesRuntimeHelpers::inline_array_as_span(buffer, length));
    interp::EvalStackOp::set_return(ret, span);
    RET_VOID_OK();
}

/// @intrinsic: <PrivateImplementationDetails>::InlineArrayFirstElementRef<,>(TBuffer&)
static RtResultVoid inline_array_first_element_ref_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                          const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    void* buffer = interp::EvalStackOp::get_param<void*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, element, SystemRuntimeCompilerServicesRuntimeHelpers::inline_array_first_element_ref(buffer));
    interp::EvalStackOp::set_return(ret, element);
    RET_VOID_OK();
}

/// @intrinsic: <PrivateImplementationDetails>::InlineArrayElementRef<,>(TBuffer&,System.Int32)
static RtResultVoid inline_array_element_ref_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                    const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    void* buffer = interp::EvalStackOp::get_param<void*>(params, 0);
    int32_t index = interp::EvalStackOp::get_param<int32_t>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, element,
                                            SystemRuntimeCompilerServicesRuntimeHelpers::inline_array_element_ref(method, buffer, index));
    interp::EvalStackOp::set_return(ret, element);
    RET_VOID_OK();
}

/// @intrinsic: System.Runtime.CompilerServices.RuntimeHelpers::IsBitwiseEquatable<>()
static RtResultVoid is_bitwise_equatable_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)params;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result,
                                            SystemRuntimeCompilerServicesRuntimeHelpers::is_bitwise_equatable(method));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

/// @intrinsic: System.Runtime.CompilerServices.RuntimeHelpers::IsReferenceOrContainsReferences<>()
static RtResultVoid is_reference_or_contains_references_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)params;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result,
                                            SystemRuntimeCompilerServicesRuntimeHelpers::is_reference_or_contains_references(method));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_runtime_compilerservices_runtimehelpers[] = {
    {"System.Runtime.CompilerServices.RuntimeHelpers::GetMethodTable(System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::get_method_table, get_method_table_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::GetRawData(System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::get_raw_data, get_raw_data_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::GetRawData()",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::get_raw_data, get_raw_data_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::InitializeArray(System.Array,System.RuntimeFieldHandle)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::initialize_array, initialize_array_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::ObjectHasComponentSize(System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::object_has_component_size,
     object_has_component_size_invoker},
    {"System.Runtime.CompilerServices.MethodTable::GetNumInstanceFieldBytes()",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::get_num_instance_field_bytes,
     get_num_instance_field_bytes_invoker},
    {"System.Runtime.CompilerServices.MethodTable::GetPrimitiveCorElementType()",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::get_primitive_cor_element_type, get_primitive_cor_element_type_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::CreateSpan<>(System.RuntimeFieldHandle)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::create_span, create_span_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::InlineArrayAsSpan<,>",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::inline_array_as_span, inline_array_as_span_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::InlineArrayAsReadOnlySpan<,>",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::inline_array_as_span, inline_array_as_span_invoker},
    {"<PrivateImplementationDetails>::InlineArrayAsSpan<,>",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::inline_array_as_span, inline_array_as_span_invoker},
    {"<PrivateImplementationDetails>::InlineArrayAsReadOnlySpan<,>",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::inline_array_as_span, inline_array_as_span_invoker},
    {"<PrivateImplementationDetails>::InlineArrayFirstElementRef<,>",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::inline_array_first_element_ref,
     inline_array_first_element_ref_invoker},
    {"<PrivateImplementationDetails>::InlineArrayElementRef<,>",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::inline_array_element_ref,
     inline_array_element_ref_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::IsBitwiseEquatable<>()",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::is_bitwise_equatable, is_bitwise_equatable_invoker},
    {"System.Runtime.CompilerServices.RuntimeHelpers::IsReferenceOrContainsReferences<>()",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesRuntimeHelpers::is_reference_or_contains_references,
     is_reference_or_contains_references_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemRuntimeCompilerServicesRuntimeHelpers::get_intrinsic_entries() noexcept
{
    constexpr size_t entry_count =
        sizeof(s_intrinsic_entries_system_runtime_compilerservices_runtimehelpers) / sizeof(s_intrinsic_entries_system_runtime_compilerservices_runtimehelpers[0]);
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_runtime_compilerservices_runtimehelpers, entry_count);
}

} // namespace intrinsics
} // namespace leanclr

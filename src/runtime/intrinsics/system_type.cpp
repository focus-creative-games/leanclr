#include "system_type.h"

#include <cstdio>
#include <cstdlib>

#include "metadata/metadata_compare.h"
#include "vm/class.h"
#include "vm/reflection.h"
#include "vm/type.h"

namespace leanclr
{
namespace intrinsics
{
namespace
{
void trace_type_sig_name(const char* side, const metadata::RtTypeSig* type_sig) noexcept
{
    auto klass_ret = vm::Class::get_class_from_typesig(type_sig);
    if (!klass_ret.is_ok())
    {
        std::fprintf(stderr, " %s=<class-error>", side);
        return;
    }

    metadata::RtClass* klass = klass_ret.unwrap();
    std::fprintf(stderr, " %s=%s.%s", side,
                 klass->namespaze != nullptr ? klass->namespaze : "",
                 klass->name != nullptr ? klass->name : "");
}
} // namespace

RtResult<vm::RtReflectionRuntimeType*> SystemType::get_type_from_handle(const void* type_handle) noexcept
{
    return vm::Reflection::get_runtime_type_from_handle_arg(type_handle);
}

RtResult<bool> SystemType::get_is_value_type(vm::RtReflectionRuntimeType* runtime_type) noexcept
{
    if (runtime_type == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig,
                                            vm::Reflection::get_type_sig_from_reflection_type_object(&runtime_type->reflection_type));
    return vm::Type::is_value_type(type_sig);
}

RtResult<bool> SystemType::equals(vm::RtReflectionRuntimeType* left, vm::RtReflectionRuntimeType* right) noexcept
{
    if (left == right)
    {
        RET_OK(true);
    }
    if (left == nullptr || right == nullptr)
    {
        RET_OK(false);
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, left_type_sig,
                                            vm::Reflection::get_type_sig_from_reflection_type_object(&left->reflection_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, right_type_sig,
                                            vm::Reflection::get_type_sig_from_reflection_type_object(&right->reflection_type));

    bool result = metadata::MetadataCompare::is_typesig_equal_ignore_attrs(left_type_sig, right_type_sig, false);
    if (std::getenv("LEANCLR_REFLECTION_TRACE") != nullptr)
    {
        std::fprintf(stderr, "leanclr-type-equals: left=%p right=%p left_sig=%p right_sig=%p result=%d\n",
                     left, right, left_type_sig, right_type_sig, result ? 1 : 0);
        std::fprintf(stderr, "leanclr-type-equals-names:");
        trace_type_sig_name("left", left_type_sig);
        trace_type_sig_name("right", right_type_sig);
        std::fprintf(stderr, "\n");
    }
    RET_OK(result);
}

RtResult<bool> SystemType::not_equals(vm::RtReflectionRuntimeType* left, vm::RtReflectionRuntimeType* right) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, equals(left, right));
    RET_OK(!result);
}

RtResult<bool> SystemType::is_assignable_to(vm::RtReflectionRuntimeType* source_type, vm::RtReflectionRuntimeType* target_type) noexcept
{
    if (source_type == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    if (target_type == nullptr)
    {
        RET_OK(false);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, source_type_sig,
                                            vm::Reflection::get_type_sig_from_reflection_type_object(&source_type->reflection_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, target_type_sig,
                                            vm::Reflection::get_type_sig_from_reflection_type_object(&target_type->reflection_type));
    if (source_type_sig->is_by_ref() != target_type_sig->is_by_ref())
    {
        RET_OK(false);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, source_class, vm::Class::get_class_from_typesig(source_type_sig));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, target_class, vm::Class::get_class_from_typesig(target_type_sig));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(source_class));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(target_class));
    RET_OK(vm::Class::is_assignable_from(source_class, target_class));
}

/// @intrinsic: System.Type::GetTypeFromHandle(System.RuntimeTypeHandle)
static RtResultVoid get_type_from_handle_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                 const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const void* type_handle = interp::EvalStackOp::get_param<const void*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionRuntimeType*, runtime_type, SystemType::get_type_from_handle(type_handle));
    interp::EvalStackOp::set_return(ret, runtime_type);
    RET_VOID_OK();
}

/// @intrinsic: System.Type::get_IsValueType
static RtResultVoid get_is_value_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                              const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto runtime_type = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_value_type, SystemType::get_is_value_type(runtime_type));
    interp::EvalStackOp::set_return(ret, is_value_type);
    RET_VOID_OK();
}

/// @intrinsic: System.Type::op_Equality(System.Type,System.Type)
static RtResultVoid equals_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto left = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto right = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemType::equals(left, right));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @intrinsic: System.Type::op_Inequality(System.Type,System.Type)
static RtResultVoid not_equals_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto left = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto right = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemType::not_equals(left, right));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @intrinsic: System.Type::IsAssignableTo(System.Type)
static RtResultVoid is_assignable_to_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto source_type = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto target_type = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemType::is_assignable_to(source_type, target_type));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_type[] = {
    {"System.Type::GetTypeFromHandle(System.RuntimeTypeHandle)", (vm::IntrinsicFunction)&SystemType::get_type_from_handle,
     get_type_from_handle_invoker},
    {"System.Type::get_IsValueType", (vm::IntrinsicFunction)&SystemType::get_is_value_type, get_is_value_type_invoker},
    {"System.RuntimeType::get_IsValueType", (vm::IntrinsicFunction)&SystemType::get_is_value_type, get_is_value_type_invoker},
    {"System.RuntimeType::IsValueTypeImpl()", (vm::IntrinsicFunction)&SystemType::get_is_value_type, get_is_value_type_invoker},
    {"System.Type::op_Equality(System.Type,System.Type)", (vm::IntrinsicFunction)&SystemType::equals, equals_invoker},
    {"System.Type::op_Equality", (vm::IntrinsicFunction)&SystemType::equals, equals_invoker},
    {"System.Type::op_Inequality(System.Type,System.Type)", (vm::IntrinsicFunction)&SystemType::not_equals, not_equals_invoker},
    {"System.Type::op_Inequality", (vm::IntrinsicFunction)&SystemType::not_equals, not_equals_invoker},
    {"System.Type::IsAssignableTo(System.Type)", (vm::IntrinsicFunction)&SystemType::is_assignable_to, is_assignable_to_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemType::get_intrinsic_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_intrinsic_entries_system_type) / sizeof(s_intrinsic_entries_system_type[0]);
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_type, entry_count);
}

} // namespace intrinsics
} // namespace leanclr

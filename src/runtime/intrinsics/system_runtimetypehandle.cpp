#include "system_runtimetypehandle.h"

#include "metadata/metadata_cache.h"
#include "vm/class.h"
#include "vm/generic_class.h"
#include "vm/reflection.h"
#include "vm/rt_array.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<vm::RtReflectionRuntimeType*> SystemRuntimeTypeHandle::get_runtime_type(const void* method_table) noexcept
{
    return vm::Reflection::get_runtime_type_from_handle_arg(method_table);
}

RtResult<vm::RtReflectionRuntimeType*> SystemRuntimeTypeHandle::get_runtime_type_from_handle(void* runtime_type_handle) noexcept
{
    return vm::Reflection::get_runtime_type_from_handle_arg(runtime_type_handle);
}

RtResult<bool> SystemRuntimeTypeHandle::can_cast_to(vm::RtReflectionRuntimeType* source_type, vm::RtReflectionRuntimeType* target_type) noexcept
{
    if (source_type == nullptr || target_type == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, source_type_sig,
                                            vm::Reflection::get_type_sig_from_runtime_type_object(source_type));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, target_type_sig,
                                            vm::Reflection::get_type_sig_from_runtime_type_object(target_type));
    if (source_type_sig == target_type_sig)
    {
        RET_OK(true);
    }

    if (source_type_sig->is_by_ref() || target_type_sig->is_by_ref())
    {
        RET_OK(false);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, source_class, vm::Class::get_class_from_typesig(source_type_sig));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, target_class, vm::Class::get_class_from_typesig(target_type_sig));
    RET_ERR_ON_FAIL(vm::Class::initialize_super_types(source_class));
    RET_ERR_ON_FAIL(vm::Class::initialize_interfaces(source_class));
    RET_ERR_ON_FAIL(vm::Class::initialize_super_types(target_class));
    RET_ERR_ON_FAIL(vm::Class::initialize_interfaces(target_class));
    RET_OK(vm::Class::is_assignable_from(source_class, target_class));
}

RtResult<vm::RtArray*> SystemRuntimeTypeHandle::copy_runtime_type_handles(vm::RtArray* types, int32_t* count) noexcept
{
    if (count == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    if (types == nullptr || vm::Array::get_array_length(types) == 0)
    {
        *count = 0;
        RET_OK(nullptr);
    }

    int32_t length = vm::Array::get_array_length(types);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, handles,
                                            LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(vm::Class::get_corlib_types().cls_intptr, length,
                                                                                       "SystemRuntimeTypeHandle::CopyRuntimeTypeHandles"));

    for (int32_t i = 0; i < length; ++i)
    {
        auto type = vm::Array::get_array_data_at<vm::RtReflectionType*>(types, i);
        if (type == nullptr)
        {
            RET_ERR(RtErr::NullReference);
        }

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig,
                                                vm::Reflection::get_type_sig_from_reflection_type_object(type));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_handle,
                                                vm::Reflection::get_net10_type_handle(type_sig));
        vm::Array::set_array_data_at<intptr_t>(handles, i, reinterpret_cast<intptr_t>(type_handle));
    }

    *count = length;
    RET_OK(handles);
}

RtResult<vm::RtReflectionRuntimeType*> SystemRuntimeTypeHandle::instantiate(void* runtime_type_handle, vm::RtArray* type_args) noexcept
{
    int32_t type_arg_count = type_args != nullptr ? vm::Array::get_array_length(type_args) : 0;
    if (type_arg_count < 0 || type_arg_count > static_cast<int32_t>(metadata::RT_MAX_GENERIC_PARAM_COUNT))
    {
        RET_ERR(RtErr::Argument);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, base_type_sig,
                                            vm::Reflection::get_type_sig_from_runtime_type_handle_arg(runtime_type_handle));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, base_klass, vm::Class::get_class_from_typesig(base_type_sig));

    uint32_t base_type_def_gid = 0;
    const metadata::RtClass* generic_definition_klass = base_klass;
    if (vm::Class::is_generic_inst(base_klass))
    {
        generic_definition_klass = vm::Class::get_generic_base_klass_of_generic_class(base_klass);
        base_type_def_gid = base_type_sig->data.generic_class->base_type_def_gid;
    }
    else if (!vm::Class::is_generic(base_klass))
    {
        RET_ERR(RtErr::Argument);
    }
    else
    {
        base_type_def_gid = vm::Class::get_type_def_gid(base_klass);
    }

    const metadata::RtGenericContainer* generic_container = generic_definition_klass->generic_container;
    if (generic_container == nullptr || type_arg_count != generic_container->generic_param_count)
    {
        RET_ERR(RtErr::Argument);
    }

    const metadata::RtTypeSig* generic_args[metadata::RT_MAX_GENERIC_PARAM_COUNT]{};
    for (int32_t i = 0; i < type_arg_count; ++i)
    {
        auto type = vm::Array::get_array_data_at<vm::RtReflectionType*>(type_args, i);
        if (type == nullptr)
        {
            RET_ERR(RtErr::NullReference);
        }

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, generic_arg,
                                                vm::Reflection::get_type_sig_from_reflection_type_object(type));
        generic_args[i] = generic_arg;
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtGenericInst*, generic_inst,
                                            metadata::MetadataCache::get_pooled_generic_inst(generic_args, static_cast<uint8_t>(type_arg_count)));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, instantiated_klass,
                                            vm::GenericClass::get_class(base_type_def_gid, generic_inst));
    return vm::Reflection::get_runtime_type_from_type_sig(instantiated_klass->by_val);
}

/// @intrinsic: System.RuntimeTypeHandle::GetRuntimeType(System.Runtime.CompilerServices.MethodTable*)
static RtResultVoid get_runtime_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    const void* method_table = interp::EvalStackOp::get_param<const void*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionRuntimeType*, runtime_type, SystemRuntimeTypeHandle::get_runtime_type(method_table));
    interp::EvalStackOp::set_return(ret, runtime_type);
    RET_VOID_OK();
}

/// @intrinsic: System.RuntimeTypeHandle::GetRuntimeType()
static RtResultVoid get_runtime_type_from_handle_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    void* runtime_type_handle = interp::EvalStackOp::get_param<void*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionRuntimeType*, runtime_type,
                                            SystemRuntimeTypeHandle::get_runtime_type_from_handle(runtime_type_handle));
    interp::EvalStackOp::set_return(ret, runtime_type);
    RET_VOID_OK();
}

/// @intrinsic: System.RuntimeTypeHandle::CanCastTo(System.RuntimeType,System.RuntimeType)
static RtResultVoid can_cast_to_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto source_type = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 0);
    auto target_type = interp::EvalStackOp::get_param<vm::RtReflectionRuntimeType*>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeTypeHandle::can_cast_to(source_type, target_type));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @intrinsic: System.RuntimeTypeHandle::CopyRuntimeTypeHandles(System.Type[],System.Int32&)
static RtResultVoid copy_runtime_type_handles_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                      const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    auto types = interp::EvalStackOp::get_param<vm::RtArray*>(params, 0);
    auto count = interp::EvalStackOp::get_param<int32_t*>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtArray*, handles, SystemRuntimeTypeHandle::copy_runtime_type_handles(types, count));
    interp::EvalStackOp::set_return(ret, handles);
    RET_VOID_OK();
}

/// @intrinsic: System.RuntimeTypeHandle::Instantiate(System.Type[])
static RtResultVoid instantiate_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    void* runtime_type_handle = interp::EvalStackOp::get_param<void*>(params, 0);
    auto type_args = interp::EvalStackOp::get_param<vm::RtArray*>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionRuntimeType*, runtime_type,
                                            SystemRuntimeTypeHandle::instantiate(runtime_type_handle, type_args));
    interp::EvalStackOp::set_return(ret, runtime_type);
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_runtimetypehandle[] = {
    {"System.RuntimeTypeHandle::GetRuntimeType(System.Runtime.CompilerServices.MethodTable*)",
     (vm::IntrinsicFunction)&SystemRuntimeTypeHandle::get_runtime_type, get_runtime_type_invoker},
    {"System.RuntimeTypeHandle::GetRuntimeType()", (vm::IntrinsicFunction)&SystemRuntimeTypeHandle::get_runtime_type_from_handle,
     get_runtime_type_from_handle_invoker},
    {"System.RuntimeTypeHandle::CanCastTo(System.RuntimeType,System.RuntimeType)",
     (vm::IntrinsicFunction)&SystemRuntimeTypeHandle::can_cast_to, can_cast_to_invoker},
    {"System.RuntimeTypeHandle::CanCastTo", (vm::IntrinsicFunction)&SystemRuntimeTypeHandle::can_cast_to, can_cast_to_invoker},
    {"System.RuntimeTypeHandle::CopyRuntimeTypeHandles(System.Type[],System.Int32&)",
     (vm::IntrinsicFunction)&SystemRuntimeTypeHandle::copy_runtime_type_handles, copy_runtime_type_handles_invoker},
    {"System.RuntimeTypeHandle::Instantiate(System.Type[])", (vm::IntrinsicFunction)&SystemRuntimeTypeHandle::instantiate, instantiate_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemRuntimeTypeHandle::get_intrinsic_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_intrinsic_entries_system_runtimetypehandle) / sizeof(s_intrinsic_entries_system_runtimetypehandle[0]);
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_runtimetypehandle, entry_count);
}

} // namespace intrinsics
} // namespace leanclr

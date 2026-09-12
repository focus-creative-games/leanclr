#include "system_runtime_compilerservices_casthelpers.h"

#include "vm/class.h"
#include "vm/reflection.h"

namespace leanclr
{
namespace intrinsics
{
namespace
{

static RtResult<bool> can_cast_to(const void* to_type_handle, vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        RET_OK(true);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, to_type_sig,
                                            vm::Reflection::get_type_sig_from_runtime_type_handle_arg(to_type_handle));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, to_class,
                                            vm::Class::get_class_from_typesig(to_type_sig));
    metadata::RtClass* from_class = const_cast<metadata::RtClass*>(obj->klass);
    metadata::RtClass* target_class = const_cast<metadata::RtClass*>(to_class);
    RET_ERR_ON_FAIL(vm::Class::initialize_all(from_class));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(target_class));
    RET_OK(vm::Class::is_assignable_from(from_class, target_class));
}

static RtResultVoid is_instance_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                        const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto to_type_handle = interp::EvalStackOp::get_param<const void*>(params, 0);
    auto obj = interp::EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result,
                                            SystemRuntimeCompilerServicesCastHelpers::is_instance_of(to_type_handle, obj));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static RtResultVoid chk_cast_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto to_type_handle = interp::EvalStackOp::get_param<const void*>(params, 0);
    auto obj = interp::EvalStackOp::get_param<vm::RtObject*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, result,
                                            SystemRuntimeCompilerServicesCastHelpers::chk_cast(to_type_handle, obj));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

} // namespace

RtResult<vm::RtObject*> SystemRuntimeCompilerServicesCastHelpers::is_instance_of(const void* to_type_handle, vm::RtObject* obj) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, can_cast, can_cast_to(to_type_handle, obj));
    RET_OK(can_cast ? obj : nullptr);
}

RtResult<vm::RtObject*> SystemRuntimeCompilerServicesCastHelpers::chk_cast(const void* to_type_handle, vm::RtObject* obj) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, can_cast, can_cast_to(to_type_handle, obj));
    if (!can_cast)
    {
        RET_ERR(RtErr::InvalidCast);
    }
    RET_OK(obj);
}

static vm::IntrinsicEntry s_intrinsic_entries_system_runtime_compilerservices_casthelpers[] = {
    {"System.Runtime.CompilerServices.CastHelpers::IsInstanceOfAny(System.Void*,System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesCastHelpers::is_instance_of, is_instance_invoker},
    {"System.Runtime.CompilerServices.CastHelpers::IsInstanceOfClass(System.Void*,System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesCastHelpers::is_instance_of, is_instance_invoker},
    {"System.Runtime.CompilerServices.CastHelpers::IsInstanceOfInterface(System.Void*,System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesCastHelpers::is_instance_of, is_instance_invoker},
    {"System.Runtime.CompilerServices.CastHelpers::IsInstance_Helper(System.Void*,System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesCastHelpers::is_instance_of, is_instance_invoker},
    {"System.Runtime.CompilerServices.CastHelpers::ChkCastAny(System.Void*,System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesCastHelpers::chk_cast, chk_cast_invoker},
    {"System.Runtime.CompilerServices.CastHelpers::ChkCastClass(System.Void*,System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesCastHelpers::chk_cast, chk_cast_invoker},
    {"System.Runtime.CompilerServices.CastHelpers::ChkCastClassSpecial(System.Void*,System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesCastHelpers::chk_cast, chk_cast_invoker},
    {"System.Runtime.CompilerServices.CastHelpers::ChkCastInterface(System.Void*,System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesCastHelpers::chk_cast, chk_cast_invoker},
    {"System.Runtime.CompilerServices.CastHelpers::ChkCast_Helper(System.Void*,System.Object)",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesCastHelpers::chk_cast, chk_cast_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemRuntimeCompilerServicesCastHelpers::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_runtime_compilerservices_casthelpers,
                                           sizeof(s_intrinsic_entries_system_runtime_compilerservices_casthelpers) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

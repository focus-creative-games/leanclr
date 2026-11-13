#include "bcl/coreclr-net10/managed_layouts.h"
#include "system_runtime_interopservices_marshal.h"

#include "interp/eval_stack_op.h"
#include "vm/gchandle.h"
#include "vm/class.h"
#include "vm/object.h"
#include "vm/struct_marshal.h"
#include "interp/machine_state.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<bool> SystemRuntimeInteropServicesMarshal::is_pinnable(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        RET_OK(true);
    }

    RET_OK(vm::GCHandle::is_type_pinned(obj->klass));
}

/// @intrinsic: System.Runtime.InteropServices.Marshal::IsPinnable(System.Object)
static RtResultVoid is_pinnable_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    vm::RtObject* obj = interp::EvalStackOp::get_param<vm::RtObject*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeInteropServicesMarshal::is_pinnable(obj));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

// CoreLib's generic entry points otherwise request JIT-generated marshal stubs.
// Keep their typed stack protocol, using the same layout engine as object APIs.
static RtResult<metadata::RtClass*> marshal_generic_class(const metadata::RtMethodInfo* method) noexcept
{
    if (!method->generic_method || !method->generic_method->generic_context.method_inst ||
        method->generic_method->generic_context.method_inst->generic_arg_count != 1)
        RET_ERR(RtErr::Argument);
    return vm::Class::get_class_from_typesig(method->generic_method->generic_context.method_inst->generic_args[0]);
}

static RtResultVoid generic_structure_to_ptr_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method,
                                                     const interp::RtStackObject* params, interp::RtStackObject*) noexcept
{
    interp::MethodArgumentRoots roots(method, params);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, marshal_generic_class(method));
    RET_ERR_ON_FAIL(vm::Class::initialize_fields(klass));
    if (vm::Class::is_value_type(klass))
    {
        const size_t slots = interp::InterpDefs::get_stack_object_size_by_byte_size(klass->instance_size_without_header);
        return vm::StructMarshal::to_native_data(klass, params, interp::EvalStackOp::get_param<void*>(params, slots),
                                                 interp::EvalStackOp::get_param<int32_t>(params, slots + 1) != 0);
    }
    return vm::StructMarshal::to_native(interp::EvalStackOp::get_param<vm::RtObject*>(params, 0), interp::EvalStackOp::get_param<void*>(params, 1),
                                        interp::EvalStackOp::get_param<int32_t>(params, 2) != 0);
}

static RtResultVoid generic_ptr_to_structure_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method,
                                                     const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    interp::MethodArgumentRoots roots(method, params);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, marshal_generic_class(method));
    const void* source = interp::EvalStackOp::get_param<void*>(params, 0);
    if (method->parameter_count == 2)
    {
        if (vm::Class::is_value_type(klass))
            RET_ERR(RtErr::Argument);
        return vm::StructMarshal::from_native(source, interp::EvalStackOp::get_param<vm::RtObject*>(params, 1));
    }
    if (!source && vm::Class::is_value_type(klass))
        RET_ERR(RtErr::NullReference);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, vm::StructMarshal::from_native(source, klass));
    if (vm::Class::is_value_type(klass))
        return vm::Object::unbox_any(obj, klass, ret, true);
    else
        interp::EvalStackOp::set_return(ret, obj);
    RET_VOID_OK();
}

static RtResultVoid generic_destroy_structure_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method,
                                                      const interp::RtStackObject* params, interp::RtStackObject*) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, marshal_generic_class(method));
    return vm::StructMarshal::destroy(interp::EvalStackOp::get_param<void*>(params, 0), klass);
}

static vm::IntrinsicEntry s_intrinsic_entries_system_runtime_interopservices_marshal[] = {
    {"System.Runtime.InteropServices.Marshal::StructureToPtr<>", nullptr, generic_structure_to_ptr_invoker},
    {"System.Runtime.InteropServices.Marshal::PtrToStructure<>", nullptr, generic_ptr_to_structure_invoker},
    {"System.Runtime.InteropServices.Marshal::DestroyStructure<>", nullptr, generic_destroy_structure_invoker},
    {"System.Runtime.InteropServices.Marshal::IsPinnable(System.Object)", (vm::IntrinsicFunction)&SystemRuntimeInteropServicesMarshal::is_pinnable,
     is_pinnable_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemRuntimeInteropServicesMarshal::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_runtime_interopservices_marshal,
                                           sizeof(s_intrinsic_entries_system_runtime_interopservices_marshal) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

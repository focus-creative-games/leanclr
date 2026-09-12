#include "system_object.h"
#include "interp/interp_defs.h"
#include "vm/object.h"
#include "vm/class.h"
#include "vm/reflection.h"

namespace leanclr
{
namespace intrinsics
{

RtResultVoid SystemObject::ctor(vm::RtObject* obj) noexcept
{
    RET_VOID_OK();
}

RtResult<vm::RtReflectionType*> SystemObject::get_type(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    return vm::Reflection::get_klass_reflection_object(obj->klass);
}

RtResult<vm::RtObject*> SystemObject::memberwise_clone(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }

    return LEANCLR_CLONE_INTERNAL(obj, "SystemObject::memberwise_clone");
}

/// @intrinsic: System.Object::.ctor()
RtResultVoid ctor_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                          interp::RtStackObject* ret) noexcept
{
    RET_VOID_OK();
}

/// @intrinsic: System.Object::GetType()
static RtResultVoid get_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    vm::RtObject* obj = interp::EvalStackOp::get_param<vm::RtObject*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, runtime_type, SystemObject::get_type(obj));
    interp::EvalStackOp::set_return(ret, runtime_type);
    RET_VOID_OK();
}

/// @intrinsic: System.Object::MemberwiseClone()
static RtResultVoid memberwise_clone_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)methodPtr;
    (void)method;
    vm::RtObject* obj = interp::EvalStackOp::get_param<vm::RtObject*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, clone, SystemObject::memberwise_clone(obj));
    interp::EvalStackOp::set_return(ret, clone);
    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemObject::newobj_ctor() noexcept
{
    return LEANCLR_NEWOBJ_INTERNAL(vm::Class::get_corlib_types().cls_object, "SystemObject::newobj_ctor");
}

/// @newobj: System.Object::.ctor()
RtResultVoid newobj_ctor_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                 interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, SystemObject::newobj_ctor());
    interp::EvalStackOp::set_return(ret, obj);
    RET_VOID_OK();
}

// Intrinsic registry
static vm::IntrinsicEntry s_intrinsic_entries_system_object[] = {
    {"System.Object::.ctor()", (vm::IntrinsicFunction)&SystemObject::ctor, ctor_invoker},
    {"System.Object::GetType()", (vm::IntrinsicFunction)&SystemObject::get_type, get_type_invoker},
    {"System.Object::MemberwiseClone()", (vm::IntrinsicFunction)&SystemObject::memberwise_clone, memberwise_clone_invoker},
};

// Newobj intrinsic registry
static vm::NewobjIntrinsicEntry s_newobj_intrinsic_entries_system_object[] = {
    {"System.Object::.ctor()", newobj_ctor_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemObject::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_object, sizeof(s_intrinsic_entries_system_object) / sizeof(vm::IntrinsicEntry));
}

utils::Span<vm::NewobjIntrinsicEntry> SystemObject::get_newobj_intrinsic_entries() noexcept
{
    return utils::Span<vm::NewobjIntrinsicEntry>(s_newobj_intrinsic_entries_system_object,
                                                 sizeof(s_newobj_intrinsic_entries_system_object) / sizeof(vm::NewobjIntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

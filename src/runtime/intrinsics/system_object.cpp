#include "system_object.h"
#include "interp/interp_defs.h"
#include "vm/object.h"
#include "vm/class.h"

namespace leanclr
{
namespace intrinsics
{

RtResultVoid SystemObject::ctor(vm::RtObject* obj) noexcept
{
    RET_VOID_OK();
}

/// @intrinsic: System.Object::.ctor()
RtResultVoid ctor_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                          interp::RtStackObject* ret) noexcept
{
    RET_VOID_OK();
}

RtResult<vm::RtObject*> SystemObject::newobj_ctor() noexcept
{
    return vm::Object::new_object(vm::Class::get_corlib_types().cls_object);
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

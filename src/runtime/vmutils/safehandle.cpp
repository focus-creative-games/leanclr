#include "safehandle.h"
#include "vm/object.h"
#include "vm/class.h"
#include "vm/runtime.h"

namespace leanclr
{
namespace vmutils
{

struct SafeHandleObject : public vm::RtObject
{
    intptr_t handle;
};

intptr_t SafeHandle::get_handle(vm::RtObject* o) noexcept
{
    return reinterpret_cast<SafeHandleObject*>(o)->handle;
}


void SafeHandle::set_handle(vm::RtObject* o, intptr_t handle) noexcept
{
    reinterpret_cast<SafeHandleObject*>(o)->handle = handle;
}

RtResult<vm::RtObject*> SafeHandle::new_safe_handle(const metadata::RtClass* klass, intptr_t handle) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, LEANCLR_NEWOBJ_INTERNAL(klass, "SafeHandle::new_safe_handle"));
    const metadata::RtMethodInfo* default_ctor = vm::Class::get_method_for_name(klass, ".ctor", 0, false);
    assert(default_ctor);
    interp::RtStackObject args[1];
    args[0].obj = obj;
    RET_ERR_ON_FAIL(vm::Runtime::invoke_stackobject_arguments_without_run_cctor(default_ctor, args, nullptr));
    set_handle(obj, handle);
    RET_OK(obj);
}
} // namespace vmutils
} // namespace leanclr
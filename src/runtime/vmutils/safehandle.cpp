#include "safehandle.h"
#include "vm/object.h"

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
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, vm::Object::new_object(klass));
    reinterpret_cast<SafeHandleObject*>(obj)->handle = handle;
    RET_OK(obj);
}
} // namespace vmutils
} // namespace leanclr
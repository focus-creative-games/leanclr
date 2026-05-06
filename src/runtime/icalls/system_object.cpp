#include "system_object.h"
#include "icall_base.h"

#include "vm/object.h"
#include "vm/reflection.h"

namespace leanclr
{
namespace icalls
{

/// @icall: System.Object::InternalGetHashCode
RtResult<int32_t> SystemObject::get_hash_code(vm::RtObject* obj) noexcept
{
    // Use object pointer as hash code
    // This matches Rust implementation behavior
    int32_t hash = static_cast<int32_t>(reinterpret_cast<uintptr_t>(obj));
    RET_OK(hash);
}

static RtResultVoid object_get_hash_code_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    auto obj = EvalStackOp::get_this(params);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hash_code, SystemObject::get_hash_code(obj));
    EvalStackOp::set_return(ret, hash_code);
    RET_VOID_OK();
}

/// @icall: System.Object::GetType
RtResult<vm::RtReflectionType*> SystemObject::get_type(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
        RET_ERR(RtErr::NullReference);

    // Get the class from the object and return its reflection type
    return vm::Reflection::get_klass_reflection_object(obj->klass);
}

static RtResultVoid get_type_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    auto obj = EvalStackOp::get_this(params);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionType*, type_obj, SystemObject::get_type(obj));
    EvalStackOp::set_return(ret, type_obj);
    RET_VOID_OK();
}

/// @icall: System.Object::MemberwiseClone
RtResult<vm::RtObject*> SystemObject::memberwise_clone(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
        RET_ERR(RtErr::NullReference);

    // Clone the object using VM's clone function
    return vm::Object::clone(obj);
}

static RtResultVoid memberwise_clone_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    auto obj = EvalStackOp::get_this(params);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, cloned_obj, SystemObject::memberwise_clone(obj));
    EvalStackOp::set_return(ret, cloned_obj);
    RET_VOID_OK();
}

// Internal call registry
static vm::InternalCallEntry s_internal_call_entries_system_object[] = {
    {"System.Object::InternalGetHashCode", (vm::InternalCallFunction)&SystemObject::get_hash_code, object_get_hash_code_invoker},
    {"System.Object::GetType", (vm::InternalCallFunction)&SystemObject::get_type, get_type_invoker},
    {"System.Object::MemberwiseClone", (vm::InternalCallFunction)&SystemObject::memberwise_clone, memberwise_clone_invoker},
};

utils::Span<vm::InternalCallEntry> SystemObject::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_object,
                                              sizeof(s_internal_call_entries_system_object) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

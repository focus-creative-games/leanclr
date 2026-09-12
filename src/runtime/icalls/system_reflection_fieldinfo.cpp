#include "system_reflection_fieldinfo.h"
#include "icall_base.h"
#include "vm/class.h"
#include "vm/reflection.h"
#include "vm/customattribute.h"
#include "vm/rt_array.h"

namespace leanclr
{
namespace icalls
{

// ========== Implementation Functions ==========

RtResult<vm::RtReflectionField*> SystemReflectionFieldInfo::internal_from_handle_type(const metadata::RtFieldInfo* field,
                                                                                      const metadata::RtTypeSig* type_sig) noexcept
{
    const metadata::RtClass* field_parent = field->parent;
    if (type_sig == nullptr)
    {
        return vm::Reflection::get_field_reflection_object(field, field_parent);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtClass*, cur_klass, vm::Class::get_class_from_typesig(type_sig));
    while (cur_klass != nullptr)
    {
        if (cur_klass == field_parent)
        {
            return vm::Reflection::get_field_reflection_object(field, cur_klass);
        }
        cur_klass = cur_klass->parent;
    }
    RET_OK(nullptr);
}

RtResult<vm::RtCustomAttribute*> SystemReflectionFieldInfo::get_marshal_info(vm::RtReflectionField* field) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field_info, vm::Reflection::get_field_info_from_reflection_object(field));
    return vm::CustomAttribute::get_marshal_info(field_info);
}

// ========== Invoker Functions ==========

/// @icall: System.Reflection.FieldInfo::internal_from_handle_type(System.IntPtr,System.IntPtr)
static RtResultVoid internal_from_handle_type_invoker_fieldinfo(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto field_arg = EvalStackOp::get_param<const void*>(params, 0);
    auto type_arg = EvalStackOp::get_param<const void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, field,
                                            vm::Reflection::get_field_info_from_handle_arg(field_arg));
    const metadata::RtTypeSig* type_sig = nullptr;
    if (type_arg != nullptr)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(type_sig, vm::Reflection::get_type_sig_from_runtime_type_handle_arg(type_arg));
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtReflectionField*, result, SystemReflectionFieldInfo::internal_from_handle_type(field, type_sig));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: System.Reflection.FieldInfo::get_marshal_info()
static RtResultVoid get_marshal_info_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    vm::RtReflectionField* field = EvalStackOp::get_param<vm::RtReflectionField*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtCustomAttribute*, result, SystemReflectionFieldInfo::get_marshal_info(field));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

// ========== Registration ==========

static vm::InternalCallEntry s_internal_call_entries_system_reflection_fieldinfo[] = {
    {"System.Reflection.FieldInfo::internal_from_handle_type(System.IntPtr,System.IntPtr)",
     (vm::InternalCallFunction)&SystemReflectionFieldInfo::internal_from_handle_type, internal_from_handle_type_invoker_fieldinfo},
    {"System.Reflection.FieldInfo::get_marshal_info()", (vm::InternalCallFunction)&SystemReflectionFieldInfo::get_marshal_info, get_marshal_info_invoker},
};

utils::Span<vm::InternalCallEntry> SystemReflectionFieldInfo::get_internal_call_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_internal_call_entries_system_reflection_fieldinfo) / sizeof(s_internal_call_entries_system_reflection_fieldinfo[0]);
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_system_reflection_fieldinfo, entry_count);
}

} // namespace icalls
} // namespace leanclr

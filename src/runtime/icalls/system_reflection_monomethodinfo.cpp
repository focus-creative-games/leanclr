#include "system_reflection_monomethodinfo.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/reflection.h"
#include "vm/rt_array.h"
#include "vm/customattribute.h"
#include "interp/eval_stack_op.h"

using namespace leanclr::core;
using namespace leanclr::vm;
using namespace leanclr::metadata;
using namespace leanclr::interp;

namespace leanclr
{
namespace icalls
{

// Implementation functions

RtResult<RtArray*> SystemReflectionMonoMethodInfo::get_parameter_info(const RtMethodInfo* method, RtReflectionMethod* member) noexcept
{
    RtReflectionType* ref_type = member->ref_type;
    const RtClass* klass = nullptr;
    if (ref_type != nullptr)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtClass*, resolved_klass, Class::get_class_from_typesig(ref_type->type_handle));
        klass = resolved_klass;
    }
    else
    {
        klass = method->parent;
    }
    return Reflection::get_param_objects(method, klass);
}

RtResult<int32_t> SystemReflectionMonoMethodInfo::get_method_attributes(const RtMethodInfo* method) noexcept
{
    RET_OK(static_cast<int32_t>(method->flags));
}

RtResultVoid SystemReflectionMonoMethodInfo::get_method_info(const RtMethodInfo* method, RtMonoMethodInfo* result) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, parent, Reflection::get_klass_reflection_object(method->parent));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, return_type, Reflection::get_type_reflection_object(method->return_type));

    result->parent = parent;
    result->return_type = return_type;
    result->attrs = method->flags;
    result->impl_attrs = method->iflags;

    // FIXME: handle impl_map properly
    result->call_conv = Method::is_instance(method) ? static_cast<uint32_t>(CallingConventions::HasThis) : static_cast<uint32_t>(CallingConventions::Standard);

    RET_VOID_OK();
}

RtResult<RtObject*> SystemReflectionMonoMethodInfo::get_retval_marshal(const RtMethodInfo* method) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(std::optional<uint32_t>, return_value_token_opt, Method::get_parameter_token(method, -1));

    if (!return_value_token_opt.has_value())
    {
        RET_OK(nullptr);
    }

    uint32_t return_value_token = return_value_token_opt.value();
    RtModuleDef* ass = method->parent->image;

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, customattributes, CustomAttribute::get_customattributes_data_on_target_token(ass, return_value_token));

    RtClass* cls_marshal_as = Class::get_corlib_types().cls_marshal_as;
    int32_t length = Array::get_array_length(customattributes);

    for (int32_t i = 0; i < length; i++)
    {
        RtObject* ca = Array::get_array_data_at<RtObject*>(customattributes, i);
        if (ca->klass == cls_marshal_as)
        {
            RET_OK(ca);
        }
    }

    RET_OK(nullptr);
}

// Invoker functions

/// @icall: System.Reflection.MonoMethodInfo::get_parameter_info
static RtResultVoid get_parameter_info_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    const RtMethodInfo* method = EvalStackOp::get_param<const RtMethodInfo*>(params, 0);
    RtReflectionMethod* member = EvalStackOp::get_param<RtReflectionMethod*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, params_array, SystemReflectionMonoMethodInfo::get_parameter_info(method, member));
    EvalStackOp::set_return(ret, params_array);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MonoMethodInfo::get_method_attributes
static RtResultVoid get_method_attributes_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    const RtMethodInfo* method = EvalStackOp::get_param<const RtMethodInfo*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, attrs, SystemReflectionMonoMethodInfo::get_method_attributes(method));
    EvalStackOp::set_return(ret, attrs);
    RET_VOID_OK();
}

/// @icall: System.Reflection.MonoMethodInfo::get_method_info
static RtResultVoid get_method_info_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    const RtMethodInfo* method = EvalStackOp::get_param<const RtMethodInfo*>(params, 0);
    RtMonoMethodInfo* result_info = EvalStackOp::get_param<RtMonoMethodInfo*>(params, 1);
    (void)ret; // Suppress unused parameter warning
    return SystemReflectionMonoMethodInfo::get_method_info(method, result_info);
}

/// @icall: System.Reflection.MonoMethodInfo::get_retval_marshal(System.IntPtr)
static RtResultVoid get_retval_marshal_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    const RtMethodInfo* method = EvalStackOp::get_param<const RtMethodInfo*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, retval_marshal, SystemReflectionMonoMethodInfo::get_retval_marshal(method));
    EvalStackOp::set_return(ret, retval_marshal);
    RET_VOID_OK();
}

// Internal call entries

static InternalCallEntry s_internal_call_entries_system_reflection_monomethodinfo[] = {
    {"System.Reflection.MonoMethodInfo::get_parameter_info", (InternalCallFunction)&SystemReflectionMonoMethodInfo::get_parameter_info,
     get_parameter_info_invoker},
    {"System.Reflection.MonoMethodInfo::get_method_attributes", (InternalCallFunction)&SystemReflectionMonoMethodInfo::get_method_attributes,
     get_method_attributes_invoker},
    {"System.Reflection.MonoMethodInfo::get_method_info", (InternalCallFunction)&SystemReflectionMonoMethodInfo::get_method_info, get_method_info_invoker},
    {"System.Reflection.MonoMethodInfo::get_retval_marshal(System.IntPtr)", (InternalCallFunction)&SystemReflectionMonoMethodInfo::get_retval_marshal,
     get_retval_marshal_invoker},
};

utils::Span<InternalCallEntry> SystemReflectionMonoMethodInfo::get_internal_call_entries() noexcept
{
    constexpr size_t entry_count =
        sizeof(s_internal_call_entries_system_reflection_monomethodinfo) / sizeof(s_internal_call_entries_system_reflection_monomethodinfo[0]);
    return utils::Span<InternalCallEntry>(s_internal_call_entries_system_reflection_monomethodinfo, entry_count);
}

} // namespace icalls
} // namespace leanclr

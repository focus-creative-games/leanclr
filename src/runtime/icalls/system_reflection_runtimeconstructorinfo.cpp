#include "system_reflection_runtimeconstructorinfo.h"
#include "vm/reflection.h"
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

RtResult<uint32_t> SystemReflectionRuntimeConstructorInfo::get_metadata_token(RtReflectionConstructor* constructor) noexcept
{
    const RtMethodInfo* method = constructor->method;
    RET_OK(method->token);
}

RtResult<RtObject*> SystemReflectionRuntimeConstructorInfo::internal_invoke(RtReflectionConstructor* constructor, RtObject* obj, RtArray* parameters,
                                                                            RtObject** out_exc) noexcept
{
    const RtMethodInfo* method = constructor->method;
    return Reflection::invoke_method(method, obj, parameters, out_exc);
}

// Invoker functions

/// @icall: System.Reflection.RuntimeConstructorInfo::get_metadata_token
static RtResultVoid get_metadata_token_invoker_system_reflection_runtimeconstructorinfo(RtManagedMethodPointer, const RtMethodInfo*,
                                                                                        const RtStackObject* params, RtStackObject* ret) noexcept
{
    RtReflectionConstructor* constructor = EvalStackOp::get_param<RtReflectionConstructor*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, token, SystemReflectionRuntimeConstructorInfo::get_metadata_token(constructor));
    EvalStackOp::set_return(ret, token);
    RET_VOID_OK();
}

/// @icall: System.Reflection.RuntimeConstructorInfo::InternalInvoke(System.Object,System.Object[],System.Exception&)
static RtResultVoid runtimeconstructioninfo_internal_invoke_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params,
                                                                    RtStackObject* ret) noexcept
{
    RtReflectionConstructor* constructor = EvalStackOp::get_param<RtReflectionConstructor*>(params, 0);
    RtObject* obj = EvalStackOp::get_param<RtObject*>(params, 1);
    RtArray* parameters = EvalStackOp::get_param<RtArray*>(params, 2);
    RtObject** exc_ptr = EvalStackOp::get_param<RtObject**>(params, 3);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, result, SystemReflectionRuntimeConstructorInfo::internal_invoke(constructor, obj, parameters, exc_ptr));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

// Internal call entries

static InternalCallEntry s_internal_call_entries_system_reflection_runtimeconstructorinfo[] = {
    {"System.Reflection.RuntimeConstructorInfo::get_metadata_token", (InternalCallFunction)&SystemReflectionRuntimeConstructorInfo::get_metadata_token,
     get_metadata_token_invoker_system_reflection_runtimeconstructorinfo},
    {"System.Reflection.RuntimeConstructorInfo::InternalInvoke(System.Object,System.Object[],System.Exception&)",
     (InternalCallFunction)&SystemReflectionRuntimeConstructorInfo::internal_invoke, runtimeconstructioninfo_internal_invoke_invoker},
};

utils::Span<InternalCallEntry> SystemReflectionRuntimeConstructorInfo::get_internal_call_entries() noexcept
{
    constexpr size_t entry_count =
        sizeof(s_internal_call_entries_system_reflection_runtimeconstructorinfo) / sizeof(s_internal_call_entries_system_reflection_runtimeconstructorinfo[0]);
    return utils::Span<InternalCallEntry>(s_internal_call_entries_system_reflection_runtimeconstructorinfo, entry_count);
}

} // namespace icalls
} // namespace leanclr

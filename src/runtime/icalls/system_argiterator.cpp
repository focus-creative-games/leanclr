#include "system_argiterator.h"

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

/// @icall: System.ArgIterator::Setup(System.IntPtr,System.IntPtr)
RtResultVoid SystemArgIterator::setup(intptr_t sig, intptr_t first_arg) noexcept
{
    (void)sig;
    (void)first_arg;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

static RtResultVoid setup_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                  interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto sig = EvalStackOp::get_param<intptr_t>(params, 0);
    auto first_arg = EvalStackOp::get_param<intptr_t>(params, 1);
    RET_ERR_ON_FAIL(SystemArgIterator::setup(sig, first_arg));
    RET_VOID_OK();
}

/// @icall: System.ArgIterator::IntGetNextArg(System.Void*)
RtResultVoid SystemArgIterator::int_get_next_arg(void* value) noexcept
{
    (void)value;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

static RtResultVoid int_get_next_arg_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                             const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto value = EvalStackOp::get_param<void*>(params, 0);
    RET_ERR_ON_FAIL(SystemArgIterator::int_get_next_arg(value));
    RET_VOID_OK();
}

/// @icall: System.ArgIterator::IntGetNextArgWithType(System.Void*,System.IntPtr)
RtResultVoid SystemArgIterator::int_get_next_arg_with_type(void* value, intptr_t rth_handle) noexcept
{
    (void)value;
    (void)rth_handle;
    RETURN_NOT_IMPLEMENTED_ERROR();
}

static RtResultVoid int_get_next_arg_with_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                       const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    (void)ret;
    auto value = EvalStackOp::get_param<void*>(params, 0);
    auto rth_handle = EvalStackOp::get_param<intptr_t>(params, 1);
    RET_ERR_ON_FAIL(SystemArgIterator::int_get_next_arg_with_type(value, rth_handle));
    RET_VOID_OK();
}

/// @icall: System.ArgIterator::IntGetNextArgType()
RtResult<intptr_t> SystemArgIterator::int_get_next_arg_type() noexcept
{
    RETURN_NOT_IMPLEMENTED_ERROR();
}

static RtResultVoid int_get_next_arg_type_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                  const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, result, SystemArgIterator::int_get_next_arg_type());
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemArgIterator::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.ArgIterator::Setup(System.IntPtr,System.IntPtr)", (vm::InternalCallFunction)&SystemArgIterator::setup, setup_invoker},
        {"System.ArgIterator::IntGetNextArg(System.Void*)", (vm::InternalCallFunction)&SystemArgIterator::int_get_next_arg, int_get_next_arg_invoker},
        {"System.ArgIterator::IntGetNextArgWithType(System.Void*,System.IntPtr)", (vm::InternalCallFunction)&SystemArgIterator::int_get_next_arg_with_type,
         int_get_next_arg_with_type_invoker},
        {"System.ArgIterator::IntGetNextArgType()", (vm::InternalCallFunction)&SystemArgIterator::int_get_next_arg_type, int_get_next_arg_type_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

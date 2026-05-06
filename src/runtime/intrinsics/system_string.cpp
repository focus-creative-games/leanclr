#include "intrinsics/system_string.h"

#include "interp/eval_stack_op.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace intrinsics
{

RtResult<uint16_t> SystemString::get_chars(vm::RtString* s, int32_t index) noexcept
{
    return vm::String::get_chars(s, index);
}

RtResult<int32_t> SystemString::get_length(vm::RtString* s) noexcept
{
    if (s == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    RET_OK(vm::String::get_length(s));
}

RtResult<int32_t> SystemString::get_hash_code(vm::RtString* str) noexcept
{
    int32_t hash = str ? vm::String::get_hash_code(str) : 0;
    RET_OK(hash);
}

/// @intrinsic: System.String::get_Chars
RtResultVoid get_chars_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                               interp::RtStackObject* ret) noexcept
{
    vm::RtString* s = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    int32_t index = interp::EvalStackOp::get_param<int32_t>(params, 1);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint16_t, ch, SystemString::get_chars(s, index));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(ch));
    RET_VOID_OK();
}

/// @intrinsic: System.String::get_Length
RtResultVoid get_length_invoker_intrinsics_system_string(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method,
                                                         const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    vm::RtString* s = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, length, SystemString::get_length(s));
    interp::EvalStackOp::set_return(ret, length);
    RET_VOID_OK();
}

/// @intrinsic: System.String::GetHashCode()
static RtResultVoid get_hash_code_invoker(metadata::RtManagedMethodPointer methodPtr, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                          interp::RtStackObject* ret) noexcept
{
    auto str = interp::EvalStackOp::get_param<vm::RtString*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, hash, SystemString::get_hash_code(str));
    interp::EvalStackOp::set_return(ret, hash);
    RET_VOID_OK();
}

// Intrinsic registry
static vm::IntrinsicEntry s_intrinsic_entries_system_string[] = {
    {"System.String::get_Chars", (vm::IntrinsicFunction)&SystemString::get_chars, get_chars_invoker},
    {"System.String::get_Length", (vm::IntrinsicFunction)&SystemString::get_length, get_length_invoker_intrinsics_system_string},
    {"System.String::GetHashCode", (vm::IntrinsicFunction)&SystemString::get_hash_code, get_hash_code_invoker},
    // redirected to intrinsic
    {"System.String::GetLegacyNonRandomizedHashCode", (vm::IntrinsicFunction)&SystemString::get_hash_code, get_hash_code_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemString::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_string, sizeof(s_intrinsic_entries_system_string) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr

#include "mono_safestringmarshal.h"
#include "icall_base.h"
#include "vm/rt_string.h"
#include "alloc/general_allocation.h"
#include "utils/string_util.h"
#include "utils/string_builder.h"

namespace leanclr
{
namespace icalls
{
using namespace leanclr::interp;
using namespace leanclr::vm;
using namespace leanclr::metadata;

// ========== Implementation Functions ==========

RtResult<intptr_t> MonoSafeStringMarshal::string_to_utf8_bytes(RtString** ptrs) noexcept
{
    RtString* s = *ptrs;
    if (s == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    utils::StringBuilder sb;
    utils::StringUtil::utf16_to_utf8(String::get_chars_ptr(s), static_cast<size_t>(String::get_length(s)), sb);
    RET_OK((intptr_t)sb.dup_to_zero_end_cstr());
}

RtResultVoid MonoSafeStringMarshal::gfree(intptr_t ptr) noexcept
{
    alloc::GeneralAllocation::free((void*)ptr);
    RET_VOID_OK();
}

// ========== Invoker Functions ==========

/// @icall: Mono.SafeStringMarshal::StringToUtf8_icall(System.String&)
static RtResultVoid string_to_utf8_bytes_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    RtString** s_ptr = EvalStackOp::get_param<RtString**>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, result, MonoSafeStringMarshal::string_to_utf8_bytes(s_ptr));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Mono.SafeStringMarshal::GFree(System.IntPtr)
static RtResultVoid gfree_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret) noexcept
{
    intptr_t ptr = EvalStackOp::get_param<intptr_t>(params, 0);
    return MonoSafeStringMarshal::gfree(ptr);
}

// ========== Registration ==========

static InternalCallEntry s_internal_call_entries_mono_safestringmarshal[] = {
    {"Mono.SafeStringMarshal::StringToUtf8_icall(System.String&)", (InternalCallFunction)&MonoSafeStringMarshal::string_to_utf8_bytes,
     string_to_utf8_bytes_invoker},
    {"Mono.SafeStringMarshal::GFree(System.IntPtr)", (InternalCallFunction)&MonoSafeStringMarshal::gfree, gfree_invoker},
};

utils::Span<InternalCallEntry> MonoSafeStringMarshal::get_internal_call_entries() noexcept
{
    constexpr size_t entry_count = sizeof(s_internal_call_entries_mono_safestringmarshal) / sizeof(s_internal_call_entries_mono_safestringmarshal[0]);
    return utils::Span<InternalCallEntry>(s_internal_call_entries_mono_safestringmarshal, entry_count);
}

} // namespace icalls
} // namespace leanclr

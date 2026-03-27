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

RtResult<const char*> MonoSafeStringMarshal::string_to_utf8_bytes(RtString* s)
{
    if (s == nullptr)
    {
        RET_ERR(RtErr::NullReference);
    }
    utils::StringBuilder sb;
    utils::StringUtil::utf16_to_utf8(String::get_chars_ptr(s), String::get_length(s), sb);
    RET_OK(sb.dup_to_zero_end_cstr());
}

RtResultVoid MonoSafeStringMarshal::gfree(void* ptr)
{
    alloc::GeneralAllocation::free(ptr);
    RET_VOID_OK();
}

// ========== Invoker Functions ==========

/// @icall: Mono.SafeStringMarshal::StringToUtf8_icall(System.String&)
static RtResultVoid string_to_utf8_bytes_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret)
{
    RtString** s_ptr = EvalStackOp::get_param<RtString**>(params, 0);
    RtString* s = *s_ptr;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, result, MonoSafeStringMarshal::string_to_utf8_bytes(s));
    EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

/// @icall: Mono.SafeStringMarshal::GFree(System.IntPtr)
static RtResultVoid gfree_invoker(RtManagedMethodPointer, const RtMethodInfo*, const RtStackObject* params, RtStackObject* ret)
{
    void* ptr = EvalStackOp::get_param<void*>(params, 0);
    return MonoSafeStringMarshal::gfree(ptr);
}

// ========== Registration ==========

static InternalCallEntry s_internal_call_entries_mono_safestringmarshal[] = {
    {"Mono.SafeStringMarshal::StringToUtf8_icall(System.String&)", (InternalCallFunction)&MonoSafeStringMarshal::string_to_utf8_bytes,
     string_to_utf8_bytes_invoker},
    {"Mono.SafeStringMarshal::GFree(System.IntPtr)", (InternalCallFunction)&MonoSafeStringMarshal::gfree, gfree_invoker},
};

utils::Span<InternalCallEntry> MonoSafeStringMarshal::get_internal_call_entries()
{
    constexpr size_t entry_count = sizeof(s_internal_call_entries_mono_safestringmarshal) / sizeof(s_internal_call_entries_mono_safestringmarshal[0]);
    return utils::Span<InternalCallEntry>(s_internal_call_entries_mono_safestringmarshal, entry_count);
}

} // namespace icalls
} // namespace leanclr

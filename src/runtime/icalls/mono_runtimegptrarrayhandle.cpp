#include "mono_runtimegptrarrayhandle.h"
#include "icall_base.h"
#include "utils/safegptrarray.h"

namespace leanclr
{
namespace icalls
{

/// @icall: Mono.RuntimeGPtrArrayHandle::GPtrArrayFree
RtResultVoid MonoRuntimeGPtrArrayHandle::gptr_array_free(void* arr) noexcept
{
    // The arr parameter is a pointer to SafeGPtrArray<void>
    // We need to cast it properly and call destroy
    auto* gptr_array = static_cast<utils::SafeGPtrArray<void>*>(arr);
    utils::SafeGPtrArray<void>::destroy(gptr_array);
    RET_VOID_OK();
}

static RtResultVoid gptr_array_free_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                            interp::RtStackObject*) noexcept
{
    auto gptr_array_handle = EvalStackOp::get_param<void*>(params, 0);
    RET_ERR_ON_FAIL(MonoRuntimeGPtrArrayHandle::gptr_array_free(gptr_array_handle));
    RET_VOID_OK();
}

// Internal call registry
static vm::InternalCallEntry s_internal_call_entries_mono_runtimegptrarrayhandle[] = {
    {"Mono.RuntimeGPtrArrayHandle::GPtrArrayFree", (vm::InternalCallFunction)&MonoRuntimeGPtrArrayHandle::gptr_array_free, gptr_array_free_invoker},
};

utils::Span<vm::InternalCallEntry> MonoRuntimeGPtrArrayHandle::get_internal_call_entries() noexcept
{
    return utils::Span<vm::InternalCallEntry>(s_internal_call_entries_mono_runtimegptrarrayhandle,
                                              sizeof(s_internal_call_entries_mono_runtimegptrarrayhandle) / sizeof(vm::InternalCallEntry));
}

} // namespace icalls
} // namespace leanclr

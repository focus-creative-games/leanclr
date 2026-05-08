#include "system_threading_nativeeventcalls.h"

#include "platform/rt_event.h"

namespace leanclr
{
namespace icalls
{

RtResult<intptr_t> SystemThreadingNativeEventCalls::create_event_icall(bool manual, bool initial, const Utf16Char* name, int32_t name_length,
                                                                       int32_t* error_code) noexcept
{
    if (error_code == nullptr)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    *error_code = 0;
    // Named events are not supported in this single-threaded runtime profile.
    if (name != nullptr && name_length > 0)
    {
        RET_ERR(RtErr::NotSupported);
    }

    auto* ev = new platform::Event(manual, initial);
    auto* handle = new platform::EventHandle(ev);
    RET_OK(reinterpret_cast<intptr_t>(handle));
}

RtResult<bool> SystemThreadingNativeEventCalls::set_event_internal(intptr_t handle) noexcept
{
    if (handle == 0)
    {
        RET_OK(false);
    }
    auto* h = reinterpret_cast<platform::EventHandle*>(handle);
    RET_OK(h->get().set());
}

RtResult<bool> SystemThreadingNativeEventCalls::reset_event_internal(intptr_t handle) noexcept
{
    if (handle == 0)
    {
        RET_OK(false);
    }
    auto* h = reinterpret_cast<platform::EventHandle*>(handle);
    RET_OK(h->get().reset());
}

RtResultVoid SystemThreadingNativeEventCalls::close_event_internal(intptr_t handle) noexcept
{
    if (handle == 0)
    {
        RET_VOID_OK();
    }
    delete reinterpret_cast<platform::EventHandle*>(handle);
    RET_VOID_OK();
}

static RtResultVoid create_event_icall_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    const bool manual = EvalStackOp::get_param<bool>(params, 0);
    const bool initial = EvalStackOp::get_param<bool>(params, 1);
    const auto* name = EvalStackOp::get_param<const Utf16Char*>(params, 2);
    const int32_t name_length = EvalStackOp::get_param<int32_t>(params, 3);
    int32_t* error_code = EvalStackOp::get_param<int32_t*>(params, 4);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, ip, SystemThreadingNativeEventCalls::create_event_icall(manual, initial, name, name_length, error_code));
    EvalStackOp::set_return(ret, ip);
    RET_VOID_OK();
}

static RtResultVoid set_event_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                               interp::RtStackObject* ret) noexcept
{
    const intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, ok, SystemThreadingNativeEventCalls::set_event_internal(handle));
    EvalStackOp::set_return(ret, static_cast<int32_t>(ok));
    RET_VOID_OK();
}

static RtResultVoid reset_event_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    const intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, ok, SystemThreadingNativeEventCalls::reset_event_internal(handle));
    EvalStackOp::set_return(ret, static_cast<int32_t>(ok));
    RET_VOID_OK();
}

static RtResultVoid close_event_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* /*ret*/) noexcept
{
    const intptr_t handle = EvalStackOp::get_param<intptr_t>(params, 0);
    RET_ERR_ON_FAIL(SystemThreadingNativeEventCalls::close_event_internal(handle));
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemThreadingNativeEventCalls::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Threading.NativeEventCalls::CreateEvent_icall(System.Boolean,System.Boolean,System.Char*,System.Int32,System.Int32&)",
         (vm::InternalCallFunction)&SystemThreadingNativeEventCalls::create_event_icall, create_event_icall_invoker},
        {"System.Threading.NativeEventCalls::SetEvent_internal(System.IntPtr)", (vm::InternalCallFunction)&SystemThreadingNativeEventCalls::set_event_internal,
         set_event_internal_invoker},
        {"System.Threading.NativeEventCalls::ResetEvent_internal(System.IntPtr)",
         (vm::InternalCallFunction)&SystemThreadingNativeEventCalls::reset_event_internal, reset_event_internal_invoker},
        {"System.Threading.NativeEventCalls::CloseEvent_internal(System.IntPtr)",
         (vm::InternalCallFunction)&SystemThreadingNativeEventCalls::close_event_internal, close_event_internal_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

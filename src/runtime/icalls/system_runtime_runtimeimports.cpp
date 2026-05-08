#include "system_runtime_runtimeimports.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace leanclr
{
namespace icalls
{

using namespace interp;
using namespace metadata;

// @icall: System.Runtime.RuntimeImports::ZeroMemory
RtResultVoid SystemRuntimeRuntimeImports::zero_memory(uint8_t* ptr, uintptr_t size) noexcept
{
    std::memset(ptr, 0, size);
    RET_VOID_OK();
}

static RtResultVoid zero_memory_invoker(RtManagedMethodPointer method_pointer, const RtMethodInfo* method, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    uint8_t* ptr = EvalStackOp::get_param<uint8_t*>(params, 0);
    uintptr_t size = EvalStackOp::get_param<uintptr_t>(params, 1);
    return SystemRuntimeRuntimeImports::zero_memory(ptr, size);
}

// @icall: System.Runtime.RuntimeImports::Memmove
RtResultVoid SystemRuntimeRuntimeImports::memmove(uint8_t* dest, const uint8_t* src, uintptr_t size) noexcept
{
    std::memmove(dest, src, size);
    RET_VOID_OK();
}

static RtResultVoid memmove_invoker(RtManagedMethodPointer method_pointer, const RtMethodInfo* method, const interp::RtStackObject* params,
                                    interp::RtStackObject* ret) noexcept
{
    uint8_t* dest = EvalStackOp::get_param<uint8_t*>(params, 0);
    const uint8_t* src = EvalStackOp::get_param<const uint8_t*>(params, 1);
    uintptr_t size = EvalStackOp::get_param<uintptr_t>(params, 2);
    return SystemRuntimeRuntimeImports::memmove(dest, src, size);
}

// @icall: System.Runtime.RuntimeImports::Memmove_wbarrier
RtResultVoid SystemRuntimeRuntimeImports::memmove_wbarrier(uint8_t* dest, const uint8_t* src, uintptr_t size) noexcept
{
    // FIXME: implement write barrier
    std::memmove(dest, src, size);
    RET_VOID_OK();
}

static RtResultVoid memmove_wbarrier_invoker(RtManagedMethodPointer method_pointer, const RtMethodInfo* method, const interp::RtStackObject* params,
                                             interp::RtStackObject* ret) noexcept
{
    uint8_t* dest = EvalStackOp::get_param<uint8_t*>(params, 0);
    const uint8_t* src = EvalStackOp::get_param<const uint8_t*>(params, 1);
    uintptr_t size = EvalStackOp::get_param<uintptr_t>(params, 2);
    return SystemRuntimeRuntimeImports::memmove_wbarrier(dest, src, size);
}

// @icall: System.Runtime.RuntimeImports::_ecvt_s
RtResultVoid SystemRuntimeRuntimeImports::ecvt_s(uint8_t* buffer, int32_t size, double value, int32_t digits, int32_t* decpt, int32_t* sign) noexcept
{
#ifdef LEANCLR_PLATFORM_WIN
    // Windows/MSVC: _ecvt_s
    int err = _ecvt_s(reinterpret_cast<char*>(buffer), size, value, digits, decpt, sign);
    if (err != 0)
    {
        RET_ERR(RtErr::Argument);
    }
#elif defined(LEANCLR_PLATFORM_POSIX) && !defined(__ANDROID__)
    // POSIX: ecvt (not provided by Android Bionic)
    char* str = ecvt(value, digits, decpt, sign);
    if (!str)
    {
        RET_ERR(RtErr::Argument);
    }
    std::strncpy(reinterpret_cast<char*>(buffer), str, static_cast<size_t>(size - 1));
    buffer[size - 1] = 0;
#else
    int written = snprintf(reinterpret_cast<char*>(buffer), static_cast<size_t>(size), "%.*e", digits, value);
    if (written < 0 || written >= size)
    {
        RET_ERR(RtErr::Argument);
    }
    // 解析小数点和符号
    const char* buf = reinterpret_cast<const char*>(buffer);
    *sign = (buf[0] == '-') ? 1 : 0;
    const char* e_ptr = strchr(buf, 'e');
    if (!e_ptr)
        e_ptr = strchr(buf, 'E');
    if (!e_ptr)
    {
        *decpt = 0;
    }
    else
    {
        int exp = atoi(e_ptr + 1);
        *decpt = exp + 1;
    }
#endif
    RET_VOID_OK();
}

static RtResultVoid ecvt_s_invoker(RtManagedMethodPointer method_pointer, const RtMethodInfo* method, const interp::RtStackObject* params,
                                   interp::RtStackObject* ret) noexcept
{
    uint8_t* buffer = EvalStackOp::get_param<uint8_t*>(params, 0);
    int32_t size = EvalStackOp::get_param<int32_t>(params, 1);
    double value = EvalStackOp::get_param<double>(params, 2);
    int32_t digits = EvalStackOp::get_param<int32_t>(params, 3);
    int32_t* decpt = EvalStackOp::get_param<int32_t*>(params, 4);
    int32_t* sign = EvalStackOp::get_param<int32_t*>(params, 5);
    return SystemRuntimeRuntimeImports::ecvt_s(buffer, size, value, digits, decpt, sign);
}

utils::Span<vm::InternalCallEntry> SystemRuntimeRuntimeImports::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Runtime.RuntimeImports::ZeroMemory", (vm::InternalCallFunction)&SystemRuntimeRuntimeImports::zero_memory, zero_memory_invoker},
        {"System.Runtime.RuntimeImports::Memmove", (vm::InternalCallFunction)&SystemRuntimeRuntimeImports::memmove, memmove_invoker},
        {"System.Runtime.RuntimeImports::Memmove_wbarrier", (vm::InternalCallFunction)&SystemRuntimeRuntimeImports::memmove_wbarrier, memmove_wbarrier_invoker},
        {"System.Runtime.RuntimeImports::_ecvt_s", (vm::InternalCallFunction)&SystemRuntimeRuntimeImports::ecvt_s, ecvt_s_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr

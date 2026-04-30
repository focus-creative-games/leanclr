#include <cstdio>
#include "machine_state.h"

#include "alloc/general_allocation.h"
#include "vm/settings.h"
#include "interpreter.h"

namespace leanclr
{
namespace interp
{
void MachineState::initialize()
{
    MachineState& ms = get_global_machine_state();
    if (ms._eval_stack_base == nullptr)
    {
        size_t default_size = vm::Settings::get_default_eval_stack_object_count();
        ms._eval_stack_base = alloc::GeneralAllocation::calloc_any<RtStackObject>(default_size);
        assert(ms._eval_stack_base != nullptr);
        ms._eval_stack_size = static_cast<uint32_t>(default_size);
    }

    if (ms._frame_stack_base == nullptr)
    {
        size_t default_frame_size = vm::Settings::get_default_frame_stack_size();
        ms._frame_stack_base = static_cast<InterpFrame*>(alloc::GeneralAllocation::malloc_zeroed(sizeof(InterpFrame) * default_frame_size));
        assert(ms._frame_stack_base != nullptr);
        ms._frame_stack_size = static_cast<uint32_t>(default_frame_size);
    }

    ms._eval_stack_top = 0;
    ms._frame_stack_top = 0;
}

RtResult<RtStackObject*> MachineState::alloc_eval_stack(uint32_t size)
{
    if (_eval_stack_top + size > _eval_stack_size)
    {
        RET_ERR(RtErr::StackOverflow);
    }
    RtStackObject* ptr = _eval_stack_base + _eval_stack_top;
    _eval_stack_top += size;
    RET_OK(ptr);
}

RtResult<InterpFrame*> MachineState::alloc_frame_stack()
{
    if (_frame_stack_top + 1 > _frame_stack_size)
    {
        RET_ERR(RtErr::StackOverflow);
    }
    InterpFrame* ptr = _frame_stack_base + _frame_stack_top;
    _frame_stack_top += 1;
    RET_OK(ptr);
}

void MachineState::free_frame_stack(uint32_t old_eval_stack_top)
{
    assert(_frame_stack_top > 0);
    _frame_stack_top -= 1;
    assert((_frame_stack_base + _frame_stack_top)->old_eval_stack_top == old_eval_stack_top);
    _eval_stack_top = old_eval_stack_top;
}

RtResult<InterpFrame*> MachineState::enter_frame_from_native(const metadata::RtMethodInfo* method, const RtStackObject* args)
{
#if LEANCLR_ENABLE_FRAME_TRACE
    std::printf("enter_frame_from_native: token:%u method:%s.%s::%s\n", method->token, method->parent->namespaze, method->parent->name, method->name);
#endif
    const RtInterpMethodInfo* imi = method->interp_data;
    if (!imi)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(imi, Interpreter::init_interpreter_method(method));
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(InterpFrame*, frame, alloc_frame_stack());
    frame->method = method;

    const uint32_t method_max_stack = imi->max_stack_object_size;
    frame->old_eval_stack_top = get_eval_stack_top();
    UNWRAP_OR_RET_ERR_ON_FAIL(frame->eval_stack_base, alloc_eval_stack(method_max_stack));
#if LEANCLR_DEBUG
    std::memset(frame->eval_stack_base, 0, static_cast<size_t>(method_max_stack) * sizeof(RtStackObject));
#endif

    if (method->total_arg_stack_object_size > 0)
    {
        std::memcpy(frame->eval_stack_base, args, method->total_arg_stack_object_size * sizeof(RtStackObject));
    }
    frame->eval_stack_size = method_max_stack;
    frame->ip = imi->codes;
    RET_OK(frame);
}

RtResult<InterpFrame*> MachineState::enter_frame_from_interp(const metadata::RtMethodInfo* method, RtStackObject* frame_base)
{
#if LEANCLR_ENABLE_FRAME_TRACE
    std::printf("enter_frame_from_interp: token:0x%0x method:%s.%s::%s\n", method->token, method->parent->namespaze, method->parent->name, method->name);
#endif
    const RtInterpMethodInfo* imi = method->interp_data;
    if (!imi)
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(imi, Interpreter::init_interpreter_method(method));
    }
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(InterpFrame*, frame, alloc_frame_stack());
    frame->method = method;

    const uint32_t method_max_stack = imi->max_stack_object_size;
    frame->old_eval_stack_top = get_eval_stack_top();
    const uint32_t frame_base_idx = static_cast<uint32_t>(frame_base - _eval_stack_base);
    const uint32_t new_eval_stack_top = frame_base_idx + method_max_stack;
    if (new_eval_stack_top > _eval_stack_size)
    {
        RET_ERR(RtErr::StackOverflow);
    }
    _eval_stack_top = new_eval_stack_top;
    frame->eval_stack_base = frame_base;
    frame->eval_stack_size = method_max_stack;
#if LEANCLR_DEBUG
    const size_t arg_size = method->total_arg_stack_object_size;
    std::memset(frame->eval_stack_base + arg_size, 0, (static_cast<size_t>(method_max_stack) - arg_size) * sizeof(RtStackObject));
#endif
    frame->ip = imi->codes;
    RET_OK(frame);
}

InterpFrame* MachineState::leave_frame(const MachineStateSavePoint& sp, InterpFrame* frame)
{
#if LEANCLR_ENABLE_FRAME_TRACE
    std::printf("exit_frame: token:0x%0x method:%s.%s::%s\n", frame->method->token, frame->method->parent->namespaze, frame->method->parent->name,
                frame->method->name);
#endif
    const uint32_t index = static_cast<uint32_t>(frame - _frame_stack_base);
    assert(_frame_stack_top == index + 1);
    if (index <= sp._old_frame_stack_top)
    {
        return nullptr;
    }
    _frame_stack_top = index;
    _eval_stack_top = frame->old_eval_stack_top;
    return frame - 1;
}

uint32_t MachineState::enter_frame_from_icall_or_intrinsic(const metadata::RtMethodInfo* method)
{
#if LEANCLR_ENABLE_FRAME_TRACE
    std::printf("enter_frame_from_icall_or_intrinsic: token:0x%0x method:%s.%s::%s\n", method->token, method->parent->namespaze, method->parent->name,
                method->name);
#endif
    const uint32_t old_frame_top = _frame_stack_top;
    if (_frame_stack_top < _frame_stack_size)
    {
        InterpFrame* frame = _frame_stack_base + _frame_stack_top;
        _frame_stack_top += 1;
        frame->method = method;
#if LEANCLR_DEBUG
        frame->eval_stack_base = nullptr;
        frame->eval_stack_size = 0;
        frame->ip = nullptr;
#endif
    }
    return old_frame_top;
}

void MachineState::leave_frame_from_icall_or_intrinsic(uint32_t old_frame_top)
{
#if LEANCLR_ENABLE_FRAME_TRACE
    if (old_frame_top < _frame_stack_top)
    {
        InterpFrame* frame = _frame_stack_base + old_frame_top;
        std::printf("exit_frame_from_icall_or_intrinsic: token:0x%0x\n", frame->method->token);
    }
#endif
    _frame_stack_top = old_frame_top;
}

} // namespace interp
} // namespace leanclr

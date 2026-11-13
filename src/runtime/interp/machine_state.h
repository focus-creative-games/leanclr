#pragma once

#include <cstddef>
#include <cstdint>

#include "interp_defs.h"
#include "gc/gc_roots.h"
#include "utils/rt_span.h"

namespace leanclr
{
namespace interp
{

enum class InterpFrameRootScanMode : uint8_t
{
    InterpreterFrame,
    ExplicitObjectRoots,
    MethodArguments,
    None,
};

struct InterpFrame
{
    const metadata::RtMethodInfo* method;
    RtStackObject* eval_stack_base;
    uint32_t eval_stack_size;
    uint32_t old_eval_stack_top;
#if LEANCLR_BCL_CORECLR_NET10
    int32_t vararg_count;
    InterpFrameRootScanMode root_scan_mode;
    uint32_t gc_live_stack_size;
#endif
    const uint8_t* ip;

    void save(const uint8_t* next_ip)
    {
        ip = next_ip;
    }
};

struct MachineStateSavePoint;

// Stack-linked native roots do not allocate and cannot exhaust the interpreter
// frame reserve. Their lifetime covers callbacks and nested managed invocations.
struct MethodArgumentRoots
{
    const metadata::RtMethodInfo* method;
    RtStackObject* args;
    MethodArgumentRoots* previous;
    static MethodArgumentRoots* current;

    MethodArgumentRoots(const metadata::RtMethodInfo* method, const RtStackObject* args)
        : method(method), args(const_cast<RtStackObject*>(args)), previous(current)
    {
        current = this;
    }

    ~MethodArgumentRoots()
    {
        current = previous;
    }

    MethodArgumentRoots(const MethodArgumentRoots&) = delete;
    MethodArgumentRoots& operator=(const MethodArgumentRoots&) = delete;
};

// Native reflection arguments are pointers to typed storage, including unboxed
// structs. Keep all inputs alive while earlier arguments are being boxed.
struct IndirectArgumentRoots
{
    const metadata::RtMethodInfo* method;
    void** args;
    IndirectArgumentRoots* previous;
    static IndirectArgumentRoots* current;

    IndirectArgumentRoots(const metadata::RtMethodInfo* method, void** args) : method(method), args(args), previous(current)
    {
        current = this;
    }

    ~IndirectArgumentRoots()
    {
        current = previous;
    }

    IndirectArgumentRoots(const IndirectArgumentRoots&) = delete;
    IndirectArgumentRoots& operator=(const IndirectArgumentRoots&) = delete;
};

class MachineState
{
  public:
    static void initialize();

    static MachineState& get_global_machine_state()
    {
        static MachineState machine_state;
        return machine_state;
    }

    void reset()
    {
        _eval_stack_top = 0;
        _frame_stack_top = 0;
    }

    uint32_t get_eval_stack_top() const
    {
        return _eval_stack_top;
    }

    void set_eval_stack_top(uint32_t new_top)
    {
        _eval_stack_top = new_top;
    }

    uint32_t get_frame_stack_top() const
    {
        return _frame_stack_top;
    }

    void set_frame_stack_top(uint32_t new_top)
    {
        _frame_stack_top = new_top;
    }

    void visit_roots(gc::GcVisitObjectRoot visit, void* userdata) const;

    InterpFrame* get_executing_frame_stack()
    {
        if (_frame_stack_top == 0)
        {
            return nullptr;
        }
        return _frame_stack_base + (_frame_stack_top - 1);
    }

    InterpFrame* get_calling_frame_stack()
    {
        if (_frame_stack_top <= 1)
        {
            return nullptr;
        }
        return _frame_stack_base + (_frame_stack_top - 2);
    }

    utils::Span<const InterpFrame> get_active_frames() const
    {
        return utils::Span<const InterpFrame>(_frame_stack_base, static_cast<size_t>(_frame_stack_top));
    }

    RtResult<RtStackObject*> alloc_eval_stack(uint32_t size);
    RtResult<InterpFrame*> alloc_frame_stack();
    void free_frame_stack(uint32_t old_eval_stack_top);

    RtResult<InterpFrame*> enter_frame_from_native(const metadata::RtMethodInfo* method, const RtStackObject* args);
    RtResult<InterpFrame*> enter_frame_from_interp(const metadata::RtMethodInfo* method, RtStackObject* frame_base);
    InterpFrame* leave_frame(const MachineStateSavePoint& sp, InterpFrame* frame);

    uint32_t enter_frame_from_icall_or_intrinsic(const metadata::RtMethodInfo* method, RtStackObject* eval_stack_base = nullptr, uint32_t eval_stack_size = 0,
                                                 InterpFrameRootScanMode root_scan_mode = InterpFrameRootScanMode::None);
    void leave_frame_from_icall_or_intrinsic(uint32_t old_frame_top);

  private:
    MachineState() = default;

    RtStackObject* _eval_stack_base = nullptr;
    uint32_t _eval_stack_size = 0;
    uint32_t _eval_stack_top = 0;
    InterpFrame* _frame_stack_base = nullptr;
    uint32_t _frame_stack_size = 0;
    uint32_t _frame_stack_top = 0;
};

struct MachineStateSavePoint
{
    ~MachineStateSavePoint()
    {
        _machine_state->set_eval_stack_top(_old_eval_stack_top);
        _machine_state->set_frame_stack_top(_old_frame_stack_top);
    }

    explicit MachineStateSavePoint(MachineState& ms)
        : _machine_state(&ms), _old_eval_stack_top(ms.get_eval_stack_top()), _old_frame_stack_top(ms.get_frame_stack_top())
    {
    }

    MachineState* _machine_state;
    uint32_t _old_eval_stack_top;
    uint32_t _old_frame_stack_top;
};

} // namespace interp
} // namespace leanclr

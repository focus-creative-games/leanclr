#include <cstdio>
#include "machine_state.h"

#include "alloc/general_allocation.h"
#include "profile/profile.h"
#include "vm/settings.h"
#include "interpreter.h"
#include "codegen/leanclr_common.h"
#include "gc/garbage_collector.h"
#include "vm/class.h"
#include "vm/method.h"

namespace leanclr
{
namespace interp
{
MethodArgumentRoots* MethodArgumentRoots::current = nullptr;
IndirectArgumentRoots* IndirectArgumentRoots::current = nullptr;

static void visit_machine_state_roots(gc::GcVisitObjectRoot visit, void* userdata)
{
    MachineState::get_global_machine_state().visit_roots(visit, userdata);
}

static void visit_object_if_valid(vm::RtObject* obj, gc::GcVisitObjectRoot visit, void* userdata)
{
    obj = gc::GarbageCollector::find_object_containing(obj);
    if (obj != nullptr)
    {
        visit(obj, userdata);
    }
}

static size_t get_stack_object_size_for_type_sig(const metadata::RtTypeSig* type_sig)
{
    auto reduce_result = InterpDefs::get_reduce_type_and_size_by_typesig(type_sig);
    if (reduce_result.is_err())
    {
        return 1;
    }
    return InterpDefs::get_stack_object_size_by_byte_size(reduce_result.unwrap().byte_size);
}

static void visit_value_type_roots(const metadata::RtClass* klass, uint8_t* value_data, gc::GcVisitObjectRoot visit, void* userdata)
{
    if (klass == nullptr || klass->gc_bitmap_word_count == 0 || value_data == nullptr)
    {
        return;
    }

    const size_t bits_per_word = vm::Class::kBitsPerWord;
    for (size_t word_index = 0; word_index < klass->gc_bitmap_word_count; ++word_index)
    {
        size_t word = klass->gc_bitmap[word_index];
        for (size_t bit = 0; word != 0 && bit < bits_per_word; ++bit)
        {
            if ((word & (static_cast<size_t>(1) << bit)) != 0)
            {
                size_t bit_index = word_index * bits_per_word + bit;
                const size_t object_offset = bit_index * sizeof(void*);
                if (object_offset < vm::RT_OBJECT_HEADER_SIZE)
                    continue;
                auto slot = reinterpret_cast<vm::RtObject**>(value_data + (object_offset - vm::RT_OBJECT_HEADER_SIZE));
                visit_object_if_valid(*slot, visit, userdata);
                word &= ~(static_cast<size_t>(1) << bit);
            }
        }
    }
}

static void visit_typed_argument_roots(const metadata::RtTypeSig* type_sig, RtStackObject* slot, gc::GcVisitObjectRoot visit, void* userdata)
{
    if (type_sig == nullptr || slot == nullptr)
    {
        return;
    }

    metadata::RtTypeSig byval_type_sig{};
    const metadata::RtTypeSig* value_type_sig = type_sig;
    if (type_sig->by_ref)
    {
        // A scalar or reference field byref must retain its owning heap object,
        // in addition to any references stored in the field itself.
        visit_object_if_valid(reinterpret_cast<vm::RtObject*>(slot->ptr), visit, userdata);
        byval_type_sig = type_sig->to_canonized_without_byref();
        value_type_sig = &byval_type_sig;
    }

    auto klass_result = vm::Class::get_class_from_typesig(value_type_sig);
    if (klass_result.is_err())
    {
        return;
    }
    const metadata::RtClass* klass = klass_result.unwrap();

    if (vm::Class::is_reference_type(klass))
    {
        vm::RtObject* obj = nullptr;
        if (type_sig->by_ref)
        {
            obj = slot->ptr != nullptr ? *reinterpret_cast<vm::RtObject**>(slot->ptr) : nullptr;
        }
        else
        {
            obj = slot->obj;
        }
        visit_object_if_valid(obj, visit, userdata);
        return;
    }

    if (!vm::Class::get_has_references(klass))
    {
        return;
    }

    uint8_t* value_data = type_sig->by_ref ? reinterpret_cast<uint8_t*>(slot->ptr) : reinterpret_cast<uint8_t*>(slot);
    visit_value_type_roots(klass, value_data, visit, userdata);
}

static void visit_method_argument_roots(const metadata::RtMethodInfo* method, RtStackObject* args, uint32_t arg_stack_size, gc::GcVisitObjectRoot visit,
                                        void* userdata)
{
    if (method == nullptr || args == nullptr || arg_stack_size == 0)
    {
        return;
    }

    size_t offset = 0;
    if (vm::Method::is_instance(method))
    {
        const metadata::RtTypeSig* this_type_sig = vm::Class::is_value_type(method->parent) ? method->parent->by_ref : method->parent->by_val;
        if (offset < arg_stack_size)
        {
            visit_typed_argument_roots(this_type_sig, args + offset, visit, userdata);
        }
        offset += get_stack_object_size_for_type_sig(this_type_sig);
    }

    for (uint16_t i = 0; i < method->parameter_count && offset < arg_stack_size; ++i)
    {
        const metadata::RtTypeSig* parameter_type_sig = method->parameters[i];
        visit_typed_argument_roots(parameter_type_sig, args + offset, visit, userdata);
        offset += get_stack_object_size_for_type_sig(parameter_type_sig);
    }
}

static void visit_stack_object_roots(RtStackObject* scan_begin, RtStackObject* scan_end, gc::GcVisitObjectRoot visit, void* userdata)
{
    for (RtStackObject* slot = scan_begin; slot < scan_end; ++slot)
    {
        visit_object_if_valid(slot->obj, visit, userdata);
    }
}

void MachineState::visit_roots(gc::GcVisitObjectRoot visit, void* userdata) const
{
    for (auto* roots = IndirectArgumentRoots::current; roots; roots = roots->previous)
    {
        if (!roots->args)
            continue;
        for (uint16_t i = 0; i < roots->method->parameter_count; ++i)
        {
            auto type = roots->method->parameters[i]->to_canonized();
            type.by_ref = 1;
            RtStackObject slot{};
            slot.ptr = roots->args[i];
            visit_typed_argument_roots(&type, &slot, visit, userdata);
        }
    }
    for (auto* roots = MethodArgumentRoots::current; roots != nullptr; roots = roots->previous)
        visit_method_argument_roots(roots->method, roots->args, roots->method->total_arg_stack_object_size, visit, userdata);
#if LEANCLR_BCL_CORECLR_NET10
    for (uint32_t frame_idx = 0; frame_idx < _frame_stack_top; ++frame_idx)
    {
        const InterpFrame& frame = _frame_stack_base[frame_idx];
        if (frame.eval_stack_base == nullptr || frame.eval_stack_size == 0)
        {
            continue;
        }

        if (frame.root_scan_mode == InterpFrameRootScanMode::MethodArguments)
        {
            visit_method_argument_roots(frame.method, frame.eval_stack_base, frame.eval_stack_size, visit, userdata);
            continue;
        }

        if (frame.root_scan_mode == InterpFrameRootScanMode::None)
        {
            continue;
        }

        const RtInterpMethodInfo* imi = frame.method != nullptr ? frame.method->interp_data : nullptr;
        const uint32_t scan_size =
            frame.root_scan_mode == InterpFrameRootScanMode::ExplicitObjectRoots || imi == nullptr ? frame.eval_stack_size : frame.gc_live_stack_size;

        RtStackObject* const scan_begin = frame.eval_stack_base;
        RtStackObject* scan_end = scan_begin + scan_size;
        if (frame.root_scan_mode == InterpFrameRootScanMode::InterpreterFrame)
        {
            const uintptr_t global_begin = reinterpret_cast<uintptr_t>(_eval_stack_base);
            const uintptr_t global_end = reinterpret_cast<uintptr_t>(_eval_stack_base + _eval_stack_top);
            const uintptr_t frame_begin = reinterpret_cast<uintptr_t>(scan_begin);
            if (frame_begin >= global_begin && frame_begin < global_end)
            {
                RtStackObject* const eval_end = _eval_stack_base + _eval_stack_top;
                if (scan_end > eval_end)
                {
                    scan_end = eval_end;
                }
            }
        }

        visit_stack_object_roots(scan_begin, scan_end, visit, userdata);
    }
#else
    for (uint32_t i = 0; i < _frame_stack_top; ++i)
    {
        const auto& frame = _frame_stack_base[i];
        if (!frame.eval_stack_base || !frame.eval_stack_size)
            continue;
        auto* begin = frame.eval_stack_base;
        auto* end = begin + frame.eval_stack_size;
        const auto address = reinterpret_cast<uintptr_t>(begin);
        const auto stack_begin = reinterpret_cast<uintptr_t>(_eval_stack_base);
        const auto stack_end = reinterpret_cast<uintptr_t>(_eval_stack_base + _eval_stack_top);
        if (address >= stack_begin && address < stack_end && end > _eval_stack_base + _eval_stack_top)
            end = _eval_stack_base + _eval_stack_top;
        visit_stack_object_roots(begin, end, visit, userdata);
    }
#endif
}

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
    static bool roots_registered = false;
    if (!roots_registered)
    {
        gc::GcRoots::register_visit_object_roots(visit_machine_state_roots);
#if LEANCLR_BCL_CORECLR_NET10
        gc::GcRoots::register_visit_object_roots(Interpreter::visit_exception_roots);
#endif
        gc::GcRoots::register_visit_object_roots(codegen::AotRootScope::visit_roots);
        roots_registered = true;
    }
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
#if LEANCLR_BCL_CORECLR_NET10
    frame->vararg_count = 0;
    frame->root_scan_mode = InterpFrameRootScanMode::InterpreterFrame;
    frame->gc_live_stack_size = imi->total_arg_and_local_stack_object_size;
#endif
    frame->ip = imi->codes;
#if LEANCLR_PGO_PROFILE
    profile::Profile::inc_call_count(method);
#endif
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
#if LEANCLR_BCL_CORECLR_NET10
    frame->vararg_count = 0;
    frame->root_scan_mode = InterpFrameRootScanMode::InterpreterFrame;
    frame->gc_live_stack_size = imi->total_arg_and_local_stack_object_size;
#endif
#if LEANCLR_DEBUG
    const size_t arg_size = method->total_arg_stack_object_size;
    std::memset(frame->eval_stack_base + arg_size, 0, (static_cast<size_t>(method_max_stack) - arg_size) * sizeof(RtStackObject));
#endif
    frame->ip = imi->codes;
#if LEANCLR_PGO_PROFILE
    profile::Profile::inc_call_count(method);
#endif
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

uint32_t MachineState::enter_frame_from_icall_or_intrinsic(const metadata::RtMethodInfo* method, RtStackObject* eval_stack_base, uint32_t eval_stack_size,
                                                           InterpFrameRootScanMode root_scan_mode)
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
        frame->eval_stack_base = eval_stack_base;
        frame->eval_stack_size = eval_stack_size;

#if LEANCLR_BCL_CORECLR_NET10
        frame->root_scan_mode = root_scan_mode;
        frame->gc_live_stack_size = eval_stack_size;
#endif
#if LEANCLR_BCL_CORECLR_NET10 || LEANCLR_DEBUG
        frame->ip = nullptr;
#endif
#if LEANCLR_BCL_CORECLR_NET10
        frame->old_eval_stack_top = get_eval_stack_top();
        frame->vararg_count = 0;
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

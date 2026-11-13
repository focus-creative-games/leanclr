#include <cmath>
#include <limits>
#include "utils/integer_math.h"

#include "interpreter.h"
#include "vm/class.h"
#include "metadata/module_def.h"
#include "metadata/aot_module.h"
#include "transformer.h"
#include "machine_state.h"
#include "execution_helper.h"
#include "profile/profile.h"
#include "vm/object.h"
#include "vm/rt_array.h"
#include "vm/method.h"
#include "vm/calli.h"
#include "vm/runtime.h"
#include "vm/internal_calls.h"
#include "vm/intrinsics.h"
#include "vm/rt_exception.h"
#include "vm/enum.h"

namespace leanclr
{
namespace interp
{

static RtResult<const RtInterpMethodInfo*> transform(const metadata::RtMethodInfo* method)
{
    const metadata::RtClass* klass = method->parent;
    metadata::RtModuleDef* mod = !vm::Class::is_array_or_szarray(klass) ? klass->image : klass->parent->image;
    auto retMethodBody = mod->read_method_body(method->token);
    RET_ERR_ON_FAIL(retMethodBody);
    auto& optMethodBody = retMethodBody.unwrap();
    if (!optMethodBody)
    {
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }

    metadata::RtMethodBody& methodBody = optMethodBody.value();
    size_t guessSize = methodBody.code_size * 32;
    size_t pageSize = 1024;
    alloc::MemPool pool(guessSize, pageSize, utils::MemOp::align_up(guessSize, pageSize));
    Transformer transformer(mod, method, methodBody, pool);
    RET_ERR_ON_FAIL(transformer.transform());
    return transformer.build_interp_method_info();
}

RtResult<const RtInterpMethodInfo*> Interpreter::init_interpreter_method(const metadata::RtMethodInfo* method)
{
    assert(!method->interp_data);
    RET_ERR_ON_FAIL(vm::Class::initialize_all(const_cast<metadata::RtClass*>(method->parent)));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtInterpMethodInfo*, interp_method, transform(method));
    const_cast<metadata::RtMethodInfo*>(method)->interp_data = interp_method;
    RET_OK(interp_method);
}

#define MAKE_INTERP_CALL_SITE() ::leanclr::gc::GcAllocSite::make_interp(frame->method, static_cast<uint32_t>(ip - imi->codes))

#define LEANCLR_NEWOBJ_INTERP(klass) LEANCLR_NEWOBJ(klass, MAKE_INTERP_CALL_SITE())

#define LEANCLR_BOX_OBJECT_INTERP(klass, value) LEANCLR_BOX_OBJECT(klass, value, MAKE_INTERP_CALL_SITE())

#define LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS_INTERP(arr_klass) LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS(arr_klass, MAKE_INTERP_CALL_SITE())
#define LEANCLR_NEW_SZARRAY_FROM_ARRAY_KLASS_INTERP(arr_klass, length) LEANCLR_NEW_SZARRAY_FROM_ARRAY_KLASS(arr_klass, length, MAKE_INTERP_CALL_SITE())
#define LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERP(ele_class, length) LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS(ele_class, length, MAKE_INTERP_CALL_SITE())
#define LEANCLR_NEW_MDARRAY_FROM_ARRAY_KLASS_INTERP(arr_klass, lengths, lower_bounds) \
    LEANCLR_NEW_MDARRAY_FROM_ARRAY_KLASS(arr_klass, lengths, lower_bounds, MAKE_INTERP_CALL_SITE())
#define LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS_INTERP(ele_klass, rank, lengths, lower_bounds) \
    LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(ele_klass, rank, lengths, lower_bounds, MAKE_INTERP_CALL_SITE())

template <typename T>
inline T get_stack_value_at(RtStackObject* base, size_t index)
{
    RtStackObject* obj = base + index;
    return utils::MemOp::load_native(reinterpret_cast<T*>(obj));
}

template <typename T>
inline void set_stack_value_at(RtStackObject* base, size_t index, T value)
{
    RtStackObject* obj = base + index;
    utils::MemOp::store_native(reinterpret_cast<T*>(obj), value);
}

template <typename T>
inline T* get_ptr_stack_value_at(RtStackObject* base, size_t index)
{
    RtStackObject* obj = base + index;
    return reinterpret_cast<T*>(obj);
}

template <typename T>
inline T get_ind_stack_value_at(RtStackObject* base, size_t index)
{
    RtStackObject* obj = base + index;
    return utils::MemOp::load_native(reinterpret_cast<T*>(obj->ptr));
}

template <typename T>
inline void set_ind_stack_value_at(RtStackObject* base, size_t index, T value)
{
    RtStackObject* obj = base + index;
    utils::MemOp::store_native(reinterpret_cast<T*>(obj->ptr), value);
}

template <typename T>
inline T* get_resolved_data(const RtInterpMethodInfo* imi, size_t index)
{
    return (T*)imi->resolved_datas[index];
}

struct ThrowFlow
{
    vm::RtException* ex;
    InterpFrame* frame;
    const void* ip;
    size_t next_search_clause_idx;
    const RtInterpExceptionClause* cur_clause;
    bool handled;
};

struct LeaveFlow
{
    InterpFrame* frame;
    const void* src_ip;
    const void* target_ip;
    const RtInterpExceptionClause* cur_finally_clause;
    size_t next_search_clause_idx;
    size_t remain_finally_clause_count;
};

struct ExceptionFlow
{
    bool throw_flow;

    union
    {
        ThrowFlow throw_data;
        LeaveFlow leave_data;
    };
};

static utils::Vector<ExceptionFlow> s_exception_flows;

ExceptionFlow* peek_top_exception_flow()
{
    assert(!s_exception_flows.empty());
    return &s_exception_flows.back();
}

void push_throw_flow(vm::RtException* ex, InterpFrame* frame, const void* ip)
{
    ExceptionFlow flow;
    flow.throw_flow = true;
    auto& data = flow.throw_data;
    data.ex = ex;
    data.frame = frame;
    data.ip = ip;
    data.next_search_clause_idx = 0;
    data.cur_clause = nullptr;
    data.handled = false;
    s_exception_flows.push_back(flow);
}

void pop_throw_flow(vm::RtException* ex, InterpFrame* frame)
{
    assert(!s_exception_flows.empty());
    ExceptionFlow& flow = s_exception_flows.back();
    assert(flow.throw_flow);
    auto& data = flow.throw_data;
    assert(data.ex == ex);
    assert(data.frame == frame);
    s_exception_flows.pop_back();
}

void pop_leave_flow(InterpFrame* frame)
{
    assert(!s_exception_flows.empty());
    ExceptionFlow& flow = s_exception_flows.back();
    assert(!flow.throw_flow);
    auto& data = flow.leave_data;
    assert(data.frame == frame);
    s_exception_flows.pop_back();
}

void pop_outscope_flows(const RtInterpMethodInfo* imi, InterpFrame* frame, const void* ip)
{
    uint32_t ip_offset = static_cast<uint32_t>((const uint8_t*)ip - imi->codes);
    while (!s_exception_flows.empty())
    {
        ExceptionFlow& flow = s_exception_flows.back();
        if (flow.throw_flow)
        {
            auto& data = flow.throw_data;
            assert(data.cur_clause);
            if (data.frame != frame)
            {
                break;
            }
            if (data.cur_clause->is_in_handler_block(ip_offset))
            {
                break;
            }
        }
        else
        {
            auto& data = flow.leave_data;
            assert(data.cur_finally_clause);
            if (data.frame != frame)
            {
                break;
            }
            if (data.cur_finally_clause->is_in_handler_block(ip_offset))
            {
                break;
            }
        }
        s_exception_flows.pop_back();
    }
}

void pop_all_flow_of_cur_frame_exclude_last(InterpFrame* frame)
{
    while (!s_exception_flows.empty())
    {
        ExceptionFlow& flow = s_exception_flows.back();
        if (flow.throw_flow)
        {
            auto& data = flow.throw_data;
            if (data.frame != frame)
            {
                break;
            }
        }
        else
        {
            auto& data = flow.leave_data;
            assert(data.cur_finally_clause);
            if (data.frame != frame)
            {
                break;
            }
        }
        s_exception_flows.pop_back();
    }
}

void setup_filter_checker(const RtInterpExceptionClause* clause)
{
    assert(!s_exception_flows.empty());
    ExceptionFlow& flow = s_exception_flows.back();
    assert(flow.throw_flow);
    auto& data = flow.throw_data;
    assert(!data.handled);
    data.cur_clause = clause;
}

void setup_filter_handler(const RtInterpMethodInfo* imi, InterpFrame* frame, const void* handler_start_ip)
{
    assert(!s_exception_flows.empty());
    ExceptionFlow& flow = s_exception_flows.back();
    assert(flow.throw_flow);
    auto& data = flow.throw_data;
    assert(!data.handled);
    assert(data.cur_clause);
    assert(imi->codes + data.cur_clause->handler_begin_offset == handler_start_ip);

    ExceptionFlow new_flow = {};
    new_flow.throw_flow = true;
    auto& new_data = new_flow.throw_data;
    new_data.ex = data.ex;
    new_data.frame = data.frame;
    new_data.ip = data.ip;
    new_data.next_search_clause_idx = data.next_search_clause_idx;
    new_data.cur_clause = nullptr;
    new_data.handled = true;
    pop_outscope_flows(imi, frame, handler_start_ip);
    s_exception_flows.push_back(new_flow);
}

void setup_catch_handler(const RtInterpMethodInfo* imi, InterpFrame* frame, const RtInterpExceptionClause* clause, const void* handler_start_ip)
{
    assert(!s_exception_flows.empty());
    ExceptionFlow flow = s_exception_flows.back();
    s_exception_flows.pop_back();
    assert(flow.throw_flow);
    auto& data = flow.throw_data;
    assert(!data.handled);
    assert(imi->codes + clause->handler_begin_offset == handler_start_ip);

    ExceptionFlow new_flow = {};
    new_flow.throw_flow = true;
    auto& new_data = new_flow.throw_data;
    new_data.ex = data.ex;
    new_data.frame = data.frame;
    new_data.ip = data.ip;
    new_data.next_search_clause_idx = data.next_search_clause_idx;
    new_data.cur_clause = clause;
    new_data.handled = true;
    pop_outscope_flows(imi, frame, handler_start_ip);
    s_exception_flows.push_back(new_flow);
}

void setup_finally_or_fault_handler(const RtInterpMethodInfo* imi, const RtInterpExceptionClause* clause, const void* handler_start_ip)
{
    assert(!s_exception_flows.empty());
    ExceptionFlow& flow = s_exception_flows.back();
    assert(flow.throw_flow);
    auto& data = flow.throw_data;
    assert(!data.handled);
    assert(imi->codes + clause->handler_begin_offset == handler_start_ip);
    data.cur_clause = clause;
}

void push_leave_flow(InterpFrame* frame, const void* src_ip, const void* target_ip, const RtInterpExceptionClause* clause, size_t next_search_clause_idx,
                     size_t finally_clause_count)
{
    ExceptionFlow flow;
    flow.throw_flow = false;
    auto& data = flow.leave_data;
    data.frame = frame;
    data.src_ip = src_ip;
    data.target_ip = target_ip;
    data.cur_finally_clause = clause;
    data.next_search_clause_idx = next_search_clause_idx;
    data.remain_finally_clause_count = finally_clause_count;
    s_exception_flows.push_back(flow);
}

bool is_in_filter_check_flow(InterpFrame* frame)
{
    if (s_exception_flows.empty())
    {
        return false;
    }
    ExceptionFlow& flow = s_exception_flows.back();
    if (!flow.throw_flow)
    {
        return false;
    }
    auto& data = flow.throw_data;
    return data.frame == frame && !data.handled && data.cur_clause && data.cur_clause->flags == metadata::RtILExceptionClauseType::Filter;
}

vm::RtException* find_exception_in_enclosing_throw_flow(InterpFrame* frame, uint32_t ip_offset)
{
    for (size_t i = s_exception_flows.size(); i > 0; --i)
    {
        ExceptionFlow& flow = s_exception_flows[i - 1];
        if (flow.throw_flow)
        {
            auto& data = flow.throw_data;
            if (data.frame != frame)
            {
                return nullptr;
            }
            if (data.cur_clause && data.cur_clause->is_in_handler_block(ip_offset))
            {
                return data.ex;
            }
        }
    }
    return nullptr;
}

vm::RtException* get_exception_in_last_throw_flow(InterpFrame* frame, uint32_t ip_offset)
{
    assert(!s_exception_flows.empty());
    ExceptionFlow& flow = s_exception_flows.back();
    assert(flow.throw_flow);
    auto& data = flow.throw_data;
    assert(data.frame == frame);
    assert(data.cur_clause && (data.cur_clause->is_in_handler_block(ip_offset) || data.cur_clause->is_in_filter_block(ip_offset)));
    return data.ex;
}

#define RAISE_RUNTIME_ERROR(err)                                                       \
    if (is_in_filter_check_flow(frame))                                                \
    {                                                                                  \
        goto unwind_exception_handler;                                                 \
    }                                                                                  \
    {                                                                                  \
        vm::RtException* ex = vm::Exception::raise_error_as_exception(err, frame, ip); \
        push_throw_flow(ex, frame, ip);                                                \
        frame->save(ip);                                                               \
        goto unwind_exception_handler;                                                 \
    }

#define RAISE_RUNTIME_EXCEPTION(ex)                                            \
    if (is_in_filter_check_flow(frame))                                        \
    {                                                                          \
        goto unwind_exception_handler;                                         \
    }                                                                          \
    {                                                                          \
        vm::RtException* __ex = vm::Exception::raise_exception(ex, frame, ip); \
        push_throw_flow(__ex, frame, ip);                                      \
        frame->save(ip);                                                       \
        goto unwind_exception_handler;                                         \
    }

#define HANDLE_RAISE_RUNTIME_ERROR(type, ret, expr) \
    auto&& __##ret = (expr);                        \
    if (__##ret.is_err())                           \
    {                                               \
        RAISE_RUNTIME_ERROR(__##ret.unwrap_err());  \
    }                                               \
    type ret = __##ret.unwrap();

#define HANDLE_RAISE_RUNTIME_ERROR2(ret, expr)        \
    {                                                 \
        auto&& __temp = (expr);                       \
        if (__temp.is_err())                          \
        {                                             \
            RAISE_RUNTIME_ERROR(__temp.unwrap_err()); \
        }                                             \
        ret = __temp.unwrap();                        \
    }

#define HANDLE_RAISE_RUNTIME_ERROR_VOID(expr)         \
    {                                                 \
        auto&& __temp = (expr);                       \
        if (__temp.is_err())                          \
        {                                             \
            RAISE_RUNTIME_ERROR(__temp.unwrap_err()); \
        }                                             \
    }

#define TRY_RUN_CLASS_STATIC_CCTOR(klass)                                                      \
    {                                                                                          \
        if (vm::Class::is_cctor_not_finished(klass))                                           \
        {                                                                                      \
            HANDLE_RAISE_RUNTIME_ERROR_VOID(vm::Runtime::run_class_static_constructor(klass)); \
        }                                                                                      \
    }

#define ENTER_INTERP_FRAME(_method, _frame_base_idx, _next_ip)                                                    \
    frame->save(_next_ip);                                                                                        \
    HANDLE_RAISE_RUNTIME_ERROR2(frame, ms.enter_frame_from_interp(_method, eval_stack_base + (_frame_base_idx))); \
    ip = frame->ip;                                                                                               \
    goto method_start;

#define LEAVE_FRAME()                  \
    frame = ms.leave_frame(sp, frame); \
    if (!frame)                        \
    {                                  \
        goto end_loop;                 \
    }                                  \
    ip = frame->ip;                    \
    goto method_start;

template <typename T>
T* get_static_field_address(const metadata::RtFieldInfo* field)
{
    const metadata::RtClass* klass = field->parent;
    return reinterpret_cast<T*>(klass->static_fields_data + field->offset);
}

// Instructions and switch payloads are four-byte aligned; opcodes are always 16 bits.
#define LEANCLR_READ_OPCODE() (*reinterpret_cast<const uint16_t*>(ip))
#if LEANCLR_USE_COMPUTED_GOTO_DISPATCHER
#define LEANCLR_SWITCH() goto* instruction_labels[LEANCLR_READ_OPCODE()];
#define LEANCLR_CASE_BEGIN(code) \
    LABEL_##code:                \
    {                            \
        const auto ir = reinterpret_cast<const ll::code*>(ip);
#define LEANCLR_CASE_BEGIN_LITE(code) \
    LABEL_##code:                     \
    {
#define LEANCLR_CASE_END()                           \
    ip = reinterpret_cast<const uint8_t*>(ir + 1);   \
    goto* instruction_labels[LEANCLR_READ_OPCODE()]; \
    }
#define LEANCLR_CASE_END_LITE()                      \
    goto* instruction_labels[LEANCLR_READ_OPCODE()]; \
    }
#else
#define LEANCLR_SWITCH() switch (static_cast<ll::OpCodeEnum>(LEANCLR_READ_OPCODE()))
#define LEANCLR_CASE_BEGIN(code) \
    case ll::OpCodeEnum::code:   \
    {                            \
        const auto ir = reinterpret_cast<const ll::code*>(ip);
#define LEANCLR_CASE_BEGIN_LITE(code) \
    case ll::OpCodeEnum::code:        \
    {
#define LEANCLR_CASE_END()                         \
    ip = reinterpret_cast<const uint8_t*>(ir + 1); \
    continue;                                      \
    }
#define LEANCLR_CASE_END_LITE() \
    continue;                   \
    }
#endif

RtResult<const RtStackObject*> Interpreter::execute(const metadata::RtMethodInfo* method, const interp::RtStackObject* params)
{
    return execute_impl(method, params, 0, 0, RtErr::None, false);
}

RtResult<const RtStackObject*> Interpreter::resume_exception(const metadata::RtMethodInfo* method, const RtStackObject* snapshot, size_t slots,
                                                             uint32_t il_offset, RtErr error, bool explicit_throw)
{
    if (error == RtErr::None || !snapshot || !method || slots < method->total_arg_stack_object_size)
        RET_ERR(RtErr::InvalidOperation);
    return execute_impl(method, snapshot, slots, il_offset, error, explicit_throw);
}

RtResult<const RtStackObject*> Interpreter::execute_impl(const metadata::RtMethodInfo* method, const RtStackObject* params, size_t snapshot_slots,
                                                         uint32_t il_offset, RtErr error, bool explicit_throw)
{
    MachineState& ms = MachineState::get_global_machine_state();
    MachineStateSavePoint sp(ms);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(InterpFrame*, frame, ms.enter_frame_from_native(method, params));

#if LEANCLR_USE_COMPUTED_GOTO_DISPATCHER
#include "generated/interpreter_dispatch.inc"
#endif

    const uint8_t* ip = frame->ip;
    const RtStackObject* ret = frame->eval_stack_base;

    if (error != RtErr::None)
    {
        const auto* info = frame->method->interp_data;
        if (snapshot_slots != info->total_arg_and_local_stack_object_size)
            RET_ERR(RtErr::InvalidOperation);
        const RtInterpResumeEntry* entry = nullptr;
        for (uint32_t i = 0; i < info->exception_resume_entry_count; ++i)
            if (info->exception_resume_entries[i].il_offset == il_offset)
            {
                entry = &info->exception_resume_entries[i];
                break;
            }
        if (!entry || entry->code_offset >= info->code_size)
            RET_ERR(RtErr::InvalidOperation);
        std::memcpy(frame->eval_stack_base, params, snapshot_slots * sizeof(RtStackObject));
        ip = info->codes + entry->code_offset;
        frame->save(ip);
        vm::RtException* exception = vm::Exception::raise_error_as_exception(error, frame, ip);
        if (!exception)
            RET_ERR(RtErr::OutOfMemory);
        if (explicit_throw && error == RtErr::ManagedException)
        {
            if (!vm::Class::is_exception_sub_class(reinterpret_cast<vm::RtObject*>(exception)->klass))
                RET_ERR(RtErr::InvalidCast);
            exception = vm::Exception::raise_exception(exception, frame, ip);
        }
        push_throw_flow(exception, frame, ip);
        goto unwind_exception_handler;
    }

method_start:
{
    RtStackObject* const eval_stack_base = frame->eval_stack_base;
    const RtInterpMethodInfo* const imi = frame->method->interp_data;
    // loop_start:
    while (true)
    {
        LEANCLR_SWITCH()
        {
#include "interpreter_handlers.inc"
#if !LEANCLR_USE_COMPUTED_GOTO_DISPATCHER
        default:
            RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
#endif
        }
    }
}

unwind_exception_handler:

    while (true)
    {
        RtStackObject* const eval_stack_base = frame->eval_stack_base;
        const RtInterpMethodInfo* const imi = frame->method->interp_data;
        ExceptionFlow* cur_flow = peek_top_exception_flow();
        assert(cur_flow);
        const RtInterpExceptionClause* clauses = imi->exception_clauses;
        size_t clause_count = imi->exception_clause_count;
        if (cur_flow->throw_flow)
        {
            auto& data = cur_flow->throw_data;
            vm::RtException* ex = data.ex;
            uint32_t throw_ip_offset = static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(data.ip) - imi->codes);
            bool handled = false;
            for (size_t i = data.next_search_clause_idx; i < clause_count; ++i)
            {
                data.next_search_clause_idx = static_cast<size_t>(i + 1);
                const RtInterpExceptionClause* clause = &clauses[i];
                if (!clause->is_in_try_block(throw_ip_offset))
                {
                    continue;
                }

                switch (clause->flags)
                {
                case metadata::RtILExceptionClauseType::Exception:
                {
                    if (!clause->ex_klass || vm::Class::is_assignable_from(reinterpret_cast<vm::RtObject*>(ex)->klass, clause->ex_klass))
                    {
                        ip = imi->codes + clause->handler_begin_offset;
                        setup_catch_handler(imi, frame, clause, ip);
                        set_stack_value_at<vm::RtException*>(eval_stack_base, imi->total_arg_and_local_stack_object_size, ex);
                        handled = true;
                    }
                    break;
                }
                case metadata::RtILExceptionClauseType::Filter:
                {
                    ip = imi->codes + clause->handler_begin_offset;
                    setup_filter_checker(clause);
                    set_stack_value_at<vm::RtException*>(eval_stack_base, imi->total_arg_and_local_stack_object_size, ex);
                    handled = true;
                    break;
                }
                case metadata::RtILExceptionClauseType::Finally:
                case metadata::RtILExceptionClauseType::Fault:
                {
                    ip = imi->codes + clause->handler_begin_offset;
                    setup_finally_or_fault_handler(imi, clause, ip);
                    set_stack_value_at<vm::RtException*>(eval_stack_base, imi->total_arg_and_local_stack_object_size, ex);
                    handled = true;
                    break;
                }
                }
                if (handled)
                {
                    break;
                }
            }
            if (!handled)
            {
                pop_all_flow_of_cur_frame_exclude_last(frame);
                frame = ms.leave_frame(sp, frame);
                if (!frame)
                {
                    vm::Exception::set_current_exception(ex);
                    RET_ERR(RtErr::ManagedException);
                }
                push_throw_flow(ex, frame, frame->ip);
                // unwind to previous frame to continue searching
                continue;
            }
        }
        else
        {
            auto& data = cur_flow->leave_data;
            if (data.remain_finally_clause_count == 0)
            {
                ip = reinterpret_cast<const uint8_t*>(data.target_ip);
                pop_leave_flow(frame);
            }
            else
            {
                uint32_t src_ip_offset = static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(data.src_ip) - imi->codes);
                uint32_t target_ip_offset = static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(data.target_ip) - imi->codes);
                const RtInterpExceptionClause* next_finally_clause = nullptr;
                for (size_t i = data.next_search_clause_idx; i < clause_count; ++i)
                {
                    const RtInterpExceptionClause* clause = &clauses[i];
                    if (clause->try_begin_offset <= src_ip_offset && src_ip_offset < clause->try_end_offset)
                    {
                        data.next_search_clause_idx = static_cast<uint32_t>(i + 1);
                        if (clause->flags == metadata::RtILExceptionClauseType::Finally && clause->is_in_try_block(src_ip_offset) &&
                            !clause->is_in_try_block(target_ip_offset))
                        {
                            next_finally_clause = clause;
                            break;
                        }
                    }
                }
                assert(next_finally_clause);
                data.remain_finally_clause_count--;
                ip = imi->codes + next_finally_clause->handler_begin_offset;
            }
        }
        goto method_start;
    }
end_loop:
    RET_OK(ret);
}
} // namespace interp
} // namespace leanclr

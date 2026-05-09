#include <cmath>
#include <limits>

#include "interpreter.h"
#include "vm/class.h"
#include "metadata/module_def.h"
#include "metadata/aot_module.h"
#include "hl_transformer.h"
#include "ll_transformer.h"
#include "machine_state.h"
#include "execution_helper.h"
#include "vm/object.h"
#include "vm/rt_array.h"
#include "vm/method.h"
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
    hl::Transformer hl_transformer(mod, method, methodBody, pool);
    RET_ERR_ON_FAIL(hl_transformer.transform());
    ll::Transformer ll_transformer(hl_transformer, pool);
    RET_ERR_ON_FAIL(ll_transformer.transform());
    return ll_transformer.build_interp_method_info();
}

RtResult<const RtInterpMethodInfo*> Interpreter::init_interpreter_method(const metadata::RtMethodInfo* method)
{
    assert(!method->interp_data);
    RET_ERR_ON_FAIL(vm::Class::initialize_all(const_cast<metadata::RtClass*>(method->parent)));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const RtInterpMethodInfo*, interp_method, transform(method));
    const_cast<metadata::RtMethodInfo*>(method)->interp_data = interp_method;
    RET_OK(interp_method);
}

template <typename T>
inline T get_stack_value_at(RtStackObject* base, size_t index)
{
    RtStackObject* obj = base + index;
    return *reinterpret_cast<T*>(obj);
}

template <typename T>
inline void set_stack_value_at(RtStackObject* base, size_t index, T value)
{
    RtStackObject* obj = base + index;
    *reinterpret_cast<T*>(obj) = value;
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
    return *(reinterpret_cast<T*>(obj->ptr));
}

template <typename T>
inline void set_ind_stack_value_at(RtStackObject* base, size_t index, T value)
{
    RtStackObject* obj = base + index;
    *(reinterpret_cast<T*>(obj->ptr)) = value;
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

#if LEANCLR_USE_COMPUTED_GOTO_DISPATCHER
#define LEANCLR_SWITCH_N(n, op_offset) goto* in_labels##n[ip[op_offset]];
#define LEANCLR_CONTINUE_N(n, op_offset) goto* in_labels##n[ip[op_offset]];
#define LEANCLR_CASE_BEGIN_N(n, code) \
    LABEL##n##_##code:                \
    {                                 \
        const auto ir = (ll::code*)ip;
#define LEANCLR_CASE_BEGIN_LITE_N(n, code) \
    LABEL##n##_##code:                     \
    {
#define LEANCLR_CASE_END_N(n)                      \
    ip = reinterpret_cast<const uint8_t*>(ir + 1); \
    goto* in_labels0[*ip];                         \
    }
#define LEANCLR_CASE_END_LITE_N(n) \
    goto* in_labels0[*ip];         \
    }

#else

#define LEANCLR_SWITCH_N(n, op_offset) switch (*(ll::OpCodeValue##n*)(ip + op_offset))
#define LEANCLR_CONTINUE_N(n, op_offset) continue

#define LEANCLR_CASE_BEGIN_N(n, code) \
    case ll::OpCodeValue##n::code:    \
    {                                 \
        const auto ir = (ll::code*)ip;

#define LEANCLR_CASE_BEGIN_LITE_N(n, code) \
    case ll::OpCodeValue##n::code:         \
    {
#define LEANCLR_CASE_END_N(n)                      \
    ip = reinterpret_cast<const uint8_t*>(ir + 1); \
    continue;                                      \
    }
#define LEANCLR_CASE_END_LITE_N(n) \
    continue;                      \
    }
#endif

#define LEANCLR_SWITCH0() LEANCLR_SWITCH_N(0, 0)
#define LEANCLR_CONTINUE0() LEANCLR_CONTINUE_N(0, 0)
#define LEANCLR_CASE_BEGIN0(code) LEANCLR_CASE_BEGIN_N(0, code)
#define LEANCLR_CASE_BEGIN_LITE0(code) LEANCLR_CASE_BEGIN_LITE_N(0, code)
#define LEANCLR_CASE_END0() LEANCLR_CASE_END_N(0)
#define LEANCLR_CASE_END_LITE0() LEANCLR_CASE_END_LITE_N(0)

#define LEANCLR_SWITCH1() LEANCLR_SWITCH_N(1, 1)
#define LEANCLR_CONTINUE1() LEANCLR_CONTINUE_N(1, 1)
#define LEANCLR_CASE_BEGIN1(code) LEANCLR_CASE_BEGIN_N(1, code)
#define LEANCLR_CASE_BEGIN_LITE1(code) LEANCLR_CASE_BEGIN_LITE_N(1, code)
#define LEANCLR_CASE_END1() LEANCLR_CASE_END_N(1)
#define LEANCLR_CASE_END_LITE1() LEANCLR_CASE_END_LITE_N(1)

#define LEANCLR_SWITCH2() LEANCLR_SWITCH_N(2, 1)
#define LEANCLR_CONTINUE2() LEANCLR_CONTINUE_N(2, 1)
#define LEANCLR_CASE_BEGIN2(code) LEANCLR_CASE_BEGIN_N(2, code)
#define LEANCLR_CASE_BEGIN_LITE2(code) LEANCLR_CASE_BEGIN_LITE_N(2, code)
#define LEANCLR_CASE_END2() LEANCLR_CASE_END_N(2)
#define LEANCLR_CASE_END_LITE2() LEANCLR_CASE_END_LITE_N(2)

#define LEANCLR_SWITCH3() LEANCLR_SWITCH_N(3, 1)
#define LEANCLR_CONTINUE3() LEANCLR_CONTINUE_N(3, 1)
#define LEANCLR_CASE_BEGIN3(code) LEANCLR_CASE_BEGIN_N(3, code)
#define LEANCLR_CASE_BEGIN_LITE3(code) LEANCLR_CASE_BEGIN_LITE_N(3, code)
#define LEANCLR_CASE_END3() LEANCLR_CASE_END_N(3)
#define LEANCLR_CASE_END_LITE3() LEANCLR_CASE_END_LITE_N(3)

#define LEANCLR_SWITCH4() LEANCLR_SWITCH_N(4, 1)
#define LEANCLR_CONTINUE4() LEANCLR_CONTINUE_N(4, 1)
#define LEANCLR_CASE_BEGIN4(code) LEANCLR_CASE_BEGIN_N(4, code)
#define LEANCLR_CASE_BEGIN_LITE4(code) LEANCLR_CASE_BEGIN_LITE_N(4, code)
#define LEANCLR_CASE_END4() LEANCLR_CASE_END_N(4)
#define LEANCLR_CASE_END_LITE4() LEANCLR_CASE_END_LITE_N(4)

#define LEANCLR_SWITCH5() LEANCLR_SWITCH_N(5, 1)
#define LEANCLR_CONTINUE5() LEANCLR_CONTINUE_N(5, 1)
#define LEANCLR_CASE_BEGIN5(code) LEANCLR_CASE_BEGIN_N(5, code)
#define LEANCLR_CASE_BEGIN_LITE5(code) LEANCLR_CASE_BEGIN_LITE_N(5, code)
#define LEANCLR_CASE_END5() LEANCLR_CASE_END_N(5)
#define LEANCLR_CASE_END_LITE5() LEANCLR_CASE_END_LITE_N(5)

RtResult<const RtStackObject*> Interpreter::execute(const metadata::RtMethodInfo* method, const interp::RtStackObject* params)
{
    MachineState& ms = MachineState::get_global_machine_state();
    MachineStateSavePoint sp(ms);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(InterpFrame*, frame, ms.enter_frame_from_native(method, params));

#pragma region goto_lable
#if LEANCLR_USE_COMPUTED_GOTO_DISPATCHER
    ///{{COMPUTED_GOTO_LABELS
    static void* const in_labels0[] = {
        &&LABEL0_InitLocals1Short,
        &&LABEL0_InitLocals2Short,
        &&LABEL0_InitLocals3Short,
        &&LABEL0_InitLocals4Short,
        &&LABEL0_InitLocalsShort,
        &&LABEL0_LdLocI1Short,
        &&LABEL0_LdLocU1Short,
        &&LABEL0_LdLocI2Short,
        &&LABEL0_LdLocU2Short,
        &&LABEL0_LdLocI4Short,
        &&LABEL0_LdLocI8Short,
        &&LABEL0_LdLocAnyShort,
        &&LABEL0_LdLocaShort,
        &&LABEL0_StLocI1Short,
        &&LABEL0_StLocI2Short,
        &&LABEL0_StLocI4Short,
        &&LABEL0_StLocI8Short,
        &&LABEL0_StLocAnyShort,
        &&LABEL0_LdNullShort,
        &&LABEL0_LdcI4I2Short,
        &&LABEL0_LdcI4I4Short,
        &&LABEL0_LdcI8I2Short,
        &&LABEL0_LdcI8I4Short,
        &&LABEL0_LdcI8I8Short,
        &&LABEL0_LdStrShort,
        &&LABEL0_BrShort,
        &&LABEL0_BrTrueI4Short,
        &&LABEL0_BrTrueI8Short,
        &&LABEL0_BrFalseI4Short,
        &&LABEL0_BrFalseI8Short,
        &&LABEL0_BeqI4Short,
        &&LABEL0_BeqI8Short,
        &&LABEL0_BgeI4Short,
        &&LABEL0_BgeI8Short,
        &&LABEL0_BgtI4Short,
        &&LABEL0_BgtI8Short,
        &&LABEL0_BleI4Short,
        &&LABEL0_BleI8Short,
        &&LABEL0_BltI4Short,
        &&LABEL0_BltI8Short,
        &&LABEL0_BneUnI4Short,
        &&LABEL0_BneUnI8Short,
        &&LABEL0_BgeUnI4Short,
        &&LABEL0_BgeUnI8Short,
        &&LABEL0_BgtUnI4Short,
        &&LABEL0_BgtUnI8Short,
        &&LABEL0_BleUnI4Short,
        &&LABEL0_BleUnI8Short,
        &&LABEL0_BltUnI4Short,
        &&LABEL0_BltUnI8Short,
        &&LABEL0_AddI4Short,
        &&LABEL0_AddI8Short,
        &&LABEL0_AddR4Short,
        &&LABEL0_AddR8Short,
        &&LABEL0_SubI4Short,
        &&LABEL0_SubI8Short,
        &&LABEL0_SubR4Short,
        &&LABEL0_SubR8Short,
        &&LABEL0_MulI4Short,
        &&LABEL0_MulI8Short,
        &&LABEL0_MulR4Short,
        &&LABEL0_MulR8Short,
        &&LABEL0_DivI4Short,
        &&LABEL0_DivI8Short,
        &&LABEL0_DivR4Short,
        &&LABEL0_DivR8Short,
        &&LABEL0_DivUnI4Short,
        &&LABEL0_DivUnI8Short,
        &&LABEL0_RemI4Short,
        &&LABEL0_RemI8Short,
        &&LABEL0_RemR4Short,
        &&LABEL0_RemR8Short,
        &&LABEL0_RemUnI4Short,
        &&LABEL0_RemUnI8Short,
        &&LABEL0_AndI4Short,
        &&LABEL0_AndI8Short,
        &&LABEL0_OrI4Short,
        &&LABEL0_OrI8Short,
        &&LABEL0_XorI4Short,
        &&LABEL0_XorI8Short,
        &&LABEL0_ShlI4Short,
        &&LABEL0_ShrI4Short,
        &&LABEL0_ShrUnI4Short,
        &&LABEL0_NegI4Short,
        &&LABEL0_NegI8Short,
        &&LABEL0_NegR4Short,
        &&LABEL0_NegR8Short,
        &&LABEL0_NotI4Short,
        &&LABEL0_NotI8Short,
        &&LABEL0_ConvI1I4Short,
        &&LABEL0_ConvI1I8Short,
        &&LABEL0_ConvI1R4Short,
        &&LABEL0_ConvI1R8Short,
        &&LABEL0_ConvU1I4Short,
        &&LABEL0_ConvU1I8Short,
        &&LABEL0_ConvU1R4Short,
        &&LABEL0_ConvU1R8Short,
        &&LABEL0_ConvI2I4Short,
        &&LABEL0_ConvI2I8Short,
        &&LABEL0_ConvI2R4Short,
        &&LABEL0_ConvI2R8Short,
        &&LABEL0_ConvU2I4Short,
        &&LABEL0_ConvU2I8Short,
        &&LABEL0_ConvU2R4Short,
        &&LABEL0_ConvU2R8Short,
        &&LABEL0_ConvI4I8Short,
        &&LABEL0_ConvI4R4Short,
        &&LABEL0_ConvI4R8Short,
        &&LABEL0_ConvU4I8Short,
        &&LABEL0_ConvU4R4Short,
        &&LABEL0_ConvU4R8Short,
        &&LABEL0_ConvI8I4Short,
        &&LABEL0_ConvI8R4Short,
        &&LABEL0_ConvI8R8Short,
        &&LABEL0_ConvR4I4Short,
        &&LABEL0_ConvR4I8Short,
        &&LABEL0_ConvR4R8Short,
        &&LABEL0_ConvR8I4Short,
        &&LABEL0_ConvR8I8Short,
        &&LABEL0_ConvR8R4Short,
        &&LABEL0_CeqI4Short,
        &&LABEL0_CeqI8Short,
        &&LABEL0_CeqR4Short,
        &&LABEL0_CeqR8Short,
        &&LABEL0_CgtI4Short,
        &&LABEL0_CgtI8Short,
        &&LABEL0_CgtUnI4Short,
        &&LABEL0_CgtUnI8Short,
        &&LABEL0_CltI4Short,
        &&LABEL0_CltI8Short,
        &&LABEL0_CltUnI4Short,
        &&LABEL0_CltUnI8Short,
        &&LABEL0_InitObjI1Short,
        &&LABEL0_InitObjI2Short,
        &&LABEL0_InitObjI4Short,
        &&LABEL0_InitObjI8Short,
        &&LABEL0_InitObjAnyShort,
        &&LABEL0_CpObjI1Short,
        &&LABEL0_CpObjI2Short,
        &&LABEL0_CpObjI4Short,
        &&LABEL0_CpObjI8Short,
        &&LABEL0_CpObjAnyShort,
        &&LABEL0_LdObjAnyShort,
        &&LABEL0_StObjAnyShort,
        &&LABEL0_CastClassShort,
        &&LABEL0_IsInstShort,
        &&LABEL0_BoxShort,
        &&LABEL0_UnboxShort,
        &&LABEL0_UnboxAnyShort,
        &&LABEL0_NewArrShort,
        &&LABEL0_LdLenShort,
        &&LABEL0_LdelemaShort,
        &&LABEL0_LdelemI1Short,
        &&LABEL0_LdelemU1Short,
        &&LABEL0_LdelemI2Short,
        &&LABEL0_LdelemU2Short,
        &&LABEL0_LdelemI4Short,
        &&LABEL0_LdelemI8Short,
        &&LABEL0_LdelemIShort,
        &&LABEL0_LdelemR4Short,
        &&LABEL0_LdelemR8Short,
        &&LABEL0_LdelemRefShort,
        &&LABEL0_LdelemAnyRefShort,
        &&LABEL0_LdelemAnyValShort,
        &&LABEL0_StelemI1Short,
        &&LABEL0_StelemI2Short,
        &&LABEL0_StelemI4Short,
        &&LABEL0_StelemI8Short,
        &&LABEL0_StelemIShort,
        &&LABEL0_StelemR4Short,
        &&LABEL0_StelemR8Short,
        &&LABEL0_StelemRefShort,
        &&LABEL0_StelemAnyRefShort,
        &&LABEL0_StelemAnyValShort,
        &&LABEL0_LdftnShort,
        &&LABEL0_LdvirtftnShort,
        &&LABEL0_LdfldI1Short,
        &&LABEL0_LdfldU1Short,
        &&LABEL0_LdfldI2Short,
        &&LABEL0_LdfldU2Short,
        &&LABEL0_LdfldI4Short,
        &&LABEL0_LdfldI8Short,
        &&LABEL0_LdfldAnyShort,
        &&LABEL0_LdvfldI1Short,
        &&LABEL0_LdvfldU1Short,
        &&LABEL0_LdvfldI2Short,
        &&LABEL0_LdvfldU2Short,
        &&LABEL0_LdvfldI4Short,
        &&LABEL0_LdvfldI8Short,
        &&LABEL0_LdvfldAnyShort,
        &&LABEL0_LdfldaShort,
        &&LABEL0_StfldI1Short,
        &&LABEL0_StfldI2Short,
        &&LABEL0_StfldI4Short,
        &&LABEL0_StfldI8Short,
        &&LABEL0_StfldAnyShort,
        &&LABEL0_LdsfldI1Short,
        &&LABEL0_LdsfldU1Short,
        &&LABEL0_LdsfldI2Short,
        &&LABEL0_LdsfldU2Short,
        &&LABEL0_LdsfldI4Short,
        &&LABEL0_LdsfldI8Short,
        &&LABEL0_LdsfldAnyShort,
        &&LABEL0_LdsfldaShort,
        &&LABEL0_LdsfldRvaDataShort,
        &&LABEL0_StsfldI1Short,
        &&LABEL0_StsfldI2Short,
        &&LABEL0_StsfldI4Short,
        &&LABEL0_StsfldI8Short,
        &&LABEL0_StsfldAnyShort,
        &&LABEL0_RetVoidShort,
        &&LABEL0_RetI4Short,
        &&LABEL0_RetI8Short,
        &&LABEL0_RetAnyShort,
        &&LABEL0_RetNopShort,
        &&LABEL0_CallInterpShort,
        &&LABEL0_CallVirtInterpShort,
        &&LABEL0_CallInternalCallShort,
        &&LABEL0_CallIntrinsicShort,
        &&LABEL0_CallPInvokeShort,
        &&LABEL0_CallAotShort,
        &&LABEL0_CallRuntimeImplementedShort,
        &&LABEL0_CalliInterpShort,
        &&LABEL0_BoxRefInplaceShort,
        &&LABEL0_NewObjInterpShort,
        &&LABEL0_NewValueTypeInterpShort,
        &&LABEL0_NewObjInternalCallShort,
        &&LABEL0_NewObjIntrinsicShort,
        &&LABEL0_NewObjAotShort,
        &&LABEL0_NewValueTypeAotShort,
        &&LABEL0_ThrowShort,
        &&LABEL0_RethrowShort,
        &&LABEL0_LeaveTryWithFinallyShort,
        &&LABEL0_LeaveCatchWithFinallyShort,
        &&LABEL0_LeaveCatchWithoutFinallyShort,
        &&LABEL0_EndFilterShort,
        &&LABEL0_EndFinallyShort,
        &&LABEL0_EndFaultShort,
        &&LABEL0___UnusedF9,
        &&LABEL0___UnusedF9,
        &&LABEL0___UnusedF9,
        &&LABEL0___UnusedF9,
        &&LABEL0___UnusedF9,
        &&LABEL0___UnusedF9,
        &&LABEL0___UnusedF9,
        &&LABEL0___UnusedF9,
        &&LABEL0___UnusedF9,
        &&LABEL0___UnusedF9,
        &&LABEL0___UnusedF9,
        &&LABEL0___UnusedF9,
        &&LABEL0___UnusedF9,
        &&LABEL0_Prefix1,
        &&LABEL0_Prefix2,
        &&LABEL0_Prefix3,
        &&LABEL0_Prefix4,
        &&LABEL0_Prefix5,
    };
    static void* const in_labels1[] = {
        &&LABEL1_InitLocals,
        &&LABEL1_LdLocI1,
        &&LABEL1_LdLocU1,
        &&LABEL1_LdLocI2,
        &&LABEL1_LdLocU2,
        &&LABEL1_LdLocI4,
        &&LABEL1_LdLocI8,
        &&LABEL1_LdLocAny,
        &&LABEL1_LdLoca,
        &&LABEL1_StLocI1,
        &&LABEL1_StLocI2,
        &&LABEL1_StLocI4,
        &&LABEL1_StLocI8,
        &&LABEL1_StLocAny,
        &&LABEL1_LdNull,
        &&LABEL1_LdcI4I2,
        &&LABEL1_LdcI4I4,
        &&LABEL1_LdcI8I2,
        &&LABEL1_LdcI8I4,
        &&LABEL1_LdcI8I8,
        &&LABEL1_LdStr,
        &&LABEL1_Br,
        &&LABEL1_BrTrueI4,
        &&LABEL1_BrTrueI8,
        &&LABEL1_BrFalseI4,
        &&LABEL1_BrFalseI8,
        &&LABEL1_BeqI4,
        &&LABEL1_BeqI8,
        &&LABEL1_BeqR4,
        &&LABEL1_BeqR8,
        &&LABEL1_BgeI4,
        &&LABEL1_BgeI8,
        &&LABEL1_BgeR4,
        &&LABEL1_BgeR8,
        &&LABEL1_BgtI4,
        &&LABEL1_BgtI8,
        &&LABEL1_BgtR4,
        &&LABEL1_BgtR8,
        &&LABEL1_BleI4,
        &&LABEL1_BleI8,
        &&LABEL1_BleR4,
        &&LABEL1_BleR8,
        &&LABEL1_BltI4,
        &&LABEL1_BltI8,
        &&LABEL1_BltR4,
        &&LABEL1_BltR8,
        &&LABEL1_BneUnI4,
        &&LABEL1_BneUnI8,
        &&LABEL1_BneUnR4,
        &&LABEL1_BneUnR8,
        &&LABEL1_BgeUnI4,
        &&LABEL1_BgeUnI8,
        &&LABEL1_BgeUnR4,
        &&LABEL1_BgeUnR8,
        &&LABEL1_BgtUnI4,
        &&LABEL1_BgtUnI8,
        &&LABEL1_BgtUnR4,
        &&LABEL1_BgtUnR8,
        &&LABEL1_BleUnI4,
        &&LABEL1_BleUnI8,
        &&LABEL1_BleUnR4,
        &&LABEL1_BleUnR8,
        &&LABEL1_BltUnI4,
        &&LABEL1_BltUnI8,
        &&LABEL1_BltUnR4,
        &&LABEL1_BltUnR8,
        &&LABEL1_Switch,
        &&LABEL1_LdIndI1Short,
        &&LABEL1_LdIndU1Short,
        &&LABEL1_LdIndI2Short,
        &&LABEL1_LdIndU2Short,
        &&LABEL1_LdIndI4Short,
        &&LABEL1_LdIndI8Short,
        &&LABEL1_StIndI1Short,
        &&LABEL1_StIndI2Short,
        &&LABEL1_StIndI4Short,
        &&LABEL1_StIndI8Short,
        &&LABEL1_StIndI8I4Short,
        &&LABEL1_StIndI8U4Short,
        &&LABEL1_AddI4,
        &&LABEL1_AddI8,
        &&LABEL1_AddR4,
        &&LABEL1_AddR8,
        &&LABEL1_SubI4,
        &&LABEL1_SubI8,
        &&LABEL1_SubR4,
        &&LABEL1_SubR8,
        &&LABEL1_MulI4,
        &&LABEL1_MulI8,
        &&LABEL1_MulR4,
        &&LABEL1_MulR8,
        &&LABEL1_DivI4,
        &&LABEL1_DivI8,
        &&LABEL1_DivR4,
        &&LABEL1_DivR8,
        &&LABEL1_DivUnI4,
        &&LABEL1_DivUnI8,
        &&LABEL1_RemI4,
        &&LABEL1_RemI8,
        &&LABEL1_RemR4,
        &&LABEL1_RemR8,
        &&LABEL1_RemUnI4,
        &&LABEL1_RemUnI8,
        &&LABEL1_AndI4,
        &&LABEL1_AndI8,
        &&LABEL1_OrI4,
        &&LABEL1_OrI8,
        &&LABEL1_XorI4,
        &&LABEL1_XorI8,
        &&LABEL1_ShlI4,
        &&LABEL1_ShlI8,
        &&LABEL1_ShrI4,
        &&LABEL1_ShrI8,
        &&LABEL1_ShrUnI4,
        &&LABEL1_ShrUnI8,
        &&LABEL1_NegI4,
        &&LABEL1_NegI8,
        &&LABEL1_NegR4,
        &&LABEL1_NegR8,
        &&LABEL1_NotI4,
        &&LABEL1_NotI8,
        &&LABEL1_CeqI4,
        &&LABEL1_CeqI8,
        &&LABEL1_CeqR4,
        &&LABEL1_CeqR8,
        &&LABEL1_CgtI4,
        &&LABEL1_CgtI8,
        &&LABEL1_CgtR4,
        &&LABEL1_CgtR8,
        &&LABEL1_CgtUnI4,
        &&LABEL1_CgtUnI8,
        &&LABEL1_CgtUnR4,
        &&LABEL1_CgtUnR8,
        &&LABEL1_CltI4,
        &&LABEL1_CltI8,
        &&LABEL1_CltR4,
        &&LABEL1_CltR8,
        &&LABEL1_CltUnI4,
        &&LABEL1_CltUnI8,
        &&LABEL1_CltUnR4,
        &&LABEL1_CltUnR8,
        &&LABEL1_InitObjI1,
        &&LABEL1_InitObjI2,
        &&LABEL1_InitObjI4,
        &&LABEL1_InitObjI8,
        &&LABEL1_InitObjAny,
        &&LABEL1_CpObjI1,
        &&LABEL1_CpObjI2,
        &&LABEL1_CpObjI4,
        &&LABEL1_CpObjI8,
        &&LABEL1_CpObjAny,
        &&LABEL1_LdObjAny,
        &&LABEL1_StObjAny,
        &&LABEL1_CastClass,
        &&LABEL1_IsInst,
        &&LABEL1_Box,
        &&LABEL1_Unbox,
        &&LABEL1_UnboxAny,
        &&LABEL1_NewArr,
        &&LABEL1_LdLen,
        &&LABEL1_Ldelema,
        &&LABEL1_LdelemI1,
        &&LABEL1_LdelemU1,
        &&LABEL1_LdelemI2,
        &&LABEL1_LdelemU2,
        &&LABEL1_LdelemI4,
        &&LABEL1_LdelemI8,
        &&LABEL1_LdelemI,
        &&LABEL1_LdelemR4,
        &&LABEL1_LdelemR8,
        &&LABEL1_LdelemRef,
        &&LABEL1_LdelemAnyRef,
        &&LABEL1_LdelemAnyVal,
        &&LABEL1_StelemI1,
        &&LABEL1_StelemI2,
        &&LABEL1_StelemI4,
        &&LABEL1_StelemI8,
        &&LABEL1_StelemI,
        &&LABEL1_StelemR4,
        &&LABEL1_StelemR8,
        &&LABEL1_StelemRef,
        &&LABEL1_StelemAnyRef,
        &&LABEL1_StelemAnyVal,
        &&LABEL1_MkRefAny,
        &&LABEL1_RefAnyVal,
        &&LABEL1_RefAnyType,
        &&LABEL1_LdToken,
        &&LABEL1_CkfiniteR4,
        &&LABEL1_CkfiniteR8,
        &&LABEL1_LocAlloc,
        &&LABEL1_Ldftn,
        &&LABEL1_Ldvirtftn,
        &&LABEL1_LdfldI1,
        &&LABEL1_LdfldU1,
        &&LABEL1_LdfldI2,
        &&LABEL1_LdfldU2,
        &&LABEL1_LdfldI4,
        &&LABEL1_LdfldI8,
        &&LABEL1_LdfldAny,
        &&LABEL1_LdvfldI1,
        &&LABEL1_LdvfldU1,
        &&LABEL1_LdvfldI2,
        &&LABEL1_LdvfldU2,
        &&LABEL1_LdvfldI4,
        &&LABEL1_LdvfldI8,
        &&LABEL1_LdvfldAny,
        &&LABEL1_Ldflda,
        &&LABEL1_StfldI1,
        &&LABEL1_StfldI2,
        &&LABEL1_StfldI4,
        &&LABEL1_StfldI8,
        &&LABEL1_StfldAny,
        &&LABEL1_LdsfldI1,
        &&LABEL1_LdsfldU1,
        &&LABEL1_LdsfldI2,
        &&LABEL1_LdsfldU2,
        &&LABEL1_LdsfldI4,
        &&LABEL1_LdsfldI8,
        &&LABEL1_LdsfldAny,
        &&LABEL1_Ldsflda,
        &&LABEL1_LdsfldRvaData,
        &&LABEL1_StsfldI1,
        &&LABEL1_StsfldI2,
        &&LABEL1_StsfldI4,
        &&LABEL1_StsfldI8,
        &&LABEL1_StsfldAny,
        &&LABEL1_RetVoid,
        &&LABEL1_RetI4,
        &&LABEL1_RetI8,
        &&LABEL1_RetAny,
        &&LABEL1_CallInterp,
        &&LABEL1_CallVirtInterp,
        &&LABEL1_CallInternalCall,
        &&LABEL1_CallIntrinsic,
        &&LABEL1_CallPInvoke,
        &&LABEL1_CallAot,
        &&LABEL1_CallRuntimeImplemented,
        &&LABEL1_CalliInterp,
        &&LABEL1_BoxRefInplace,
        &&LABEL1_NewObjInterp,
        &&LABEL1_NewValueTypeInterp,
        &&LABEL1_NewObjInternalCall,
        &&LABEL1_NewObjIntrinsic,
        &&LABEL1_NewObjAot,
        &&LABEL1_NewValueTypeAot,
        &&LABEL1_Throw,
        &&LABEL1_Rethrow,
        &&LABEL1_LeaveTryWithFinally,
        &&LABEL1_LeaveCatchWithFinally,
        &&LABEL1_LeaveCatchWithoutFinally,
        &&LABEL1_EndFilter,
        &&LABEL1_EndFinally,
        &&LABEL1_EndFault,
    };
    static void* const in_labels2[] = {
        &&LABEL2_LdIndI1,  &&LABEL2_LdIndU1,   &&LABEL2_LdIndI2,
        &&LABEL2_LdIndU2,  &&LABEL2_LdIndI4,   &&LABEL2_LdIndI8,
        &&LABEL2_StIndI1,  &&LABEL2_StIndI2,   &&LABEL2_StIndI4,
        &&LABEL2_StIndI8,  &&LABEL2_StIndI8I4, &&LABEL2_StIndI8U4,
        &&LABEL2_ConvI1I4, &&LABEL2_ConvI1I8,  &&LABEL2_ConvI1R4,
        &&LABEL2_ConvI1R8, &&LABEL2_ConvU1I4,  &&LABEL2_ConvU1I8,
        &&LABEL2_ConvU1R4, &&LABEL2_ConvU1R8,  &&LABEL2_ConvI2I4,
        &&LABEL2_ConvI2I8, &&LABEL2_ConvI2R4,  &&LABEL2_ConvI2R8,
        &&LABEL2_ConvU2I4, &&LABEL2_ConvU2I8,  &&LABEL2_ConvU2R4,
        &&LABEL2_ConvU2R8, &&LABEL2_ConvI4I8,  &&LABEL2_ConvI4R4,
        &&LABEL2_ConvI4R8, &&LABEL2_ConvU4I8,  &&LABEL2_ConvU4R4,
        &&LABEL2_ConvU4R8, &&LABEL2_ConvI8I4,  &&LABEL2_ConvI8U4,
        &&LABEL2_ConvI8R4, &&LABEL2_ConvI8R8,  &&LABEL2_ConvU8I4,
        &&LABEL2_ConvU8R4, &&LABEL2_ConvU8R8,  &&LABEL2_ConvR4I4,
        &&LABEL2_ConvR4I8, &&LABEL2_ConvR4R8,  &&LABEL2_ConvR8I4,
        &&LABEL2_ConvR8I8, &&LABEL2_ConvR8R4,  &&LABEL2_LdelemaReadOnly,
        &&LABEL2_InitBlk,  &&LABEL2_CpBlk,     &&LABEL2_GetEnumLongHashCode,
    };
    static void* const in_labels3[] = {
        &&LABEL3_LdIndI2Unaligned,   &&LABEL3_LdIndU2Unaligned,  &&LABEL3_LdIndI4Unaligned,   &&LABEL3_LdIndI8Unaligned,   &&LABEL3_StIndI2Unaligned,
        &&LABEL3_StIndI4Unaligned,   &&LABEL3_StIndI8Unaligned,  &&LABEL3_StIndI8I4Unaligned, &&LABEL3_StIndI8U4Unaligned, &&LABEL3_AddOvfI4,
        &&LABEL3_AddOvfI8,           &&LABEL3_AddOvfUnI4,        &&LABEL3_AddOvfUnI8,         &&LABEL3_MulOvfI4,           &&LABEL3_MulOvfI8,
        &&LABEL3_MulOvfUnI4,         &&LABEL3_MulOvfUnI8,        &&LABEL3_SubOvfI4,           &&LABEL3_SubOvfI8,           &&LABEL3_SubOvfUnI4,
        &&LABEL3_SubOvfUnI8,         &&LABEL3_ConvOvfI1I4,       &&LABEL3_ConvOvfI1I8,        &&LABEL3_ConvOvfI1R4,        &&LABEL3_ConvOvfI1R8,
        &&LABEL3_ConvOvfU1I4,        &&LABEL3_ConvOvfU1I8,       &&LABEL3_ConvOvfU1R4,        &&LABEL3_ConvOvfU1R8,        &&LABEL3_ConvOvfI2I4,
        &&LABEL3_ConvOvfI2I8,        &&LABEL3_ConvOvfI2R4,       &&LABEL3_ConvOvfI2R8,        &&LABEL3_ConvOvfU2I4,        &&LABEL3_ConvOvfU2I8,
        &&LABEL3_ConvOvfU2R4,        &&LABEL3_ConvOvfU2R8,       &&LABEL3_ConvOvfI4I8,        &&LABEL3_ConvOvfI4R4,        &&LABEL3_ConvOvfI4R8,
        &&LABEL3_ConvOvfU4I4,        &&LABEL3_ConvOvfU4I8,       &&LABEL3_ConvOvfU4R4,        &&LABEL3_ConvOvfU4R8,        &&LABEL3_ConvOvfI8R4,
        &&LABEL3_ConvOvfI8R8,        &&LABEL3_ConvOvfU8I4,       &&LABEL3_ConvOvfU8I8,        &&LABEL3_ConvOvfU8R4,        &&LABEL3_ConvOvfU8R8,
        &&LABEL3_ConvOvfI1UnI4,      &&LABEL3_ConvOvfI1UnI8,     &&LABEL3_ConvOvfI1UnR4,      &&LABEL3_ConvOvfI1UnR8,      &&LABEL3_ConvOvfU1UnI4,
        &&LABEL3_ConvOvfU1UnI8,      &&LABEL3_ConvOvfU1UnR4,     &&LABEL3_ConvOvfU1UnR8,      &&LABEL3_ConvOvfI2UnI4,      &&LABEL3_ConvOvfI2UnI8,
        &&LABEL3_ConvOvfI2UnR4,      &&LABEL3_ConvOvfI2UnR8,     &&LABEL3_ConvOvfU2UnI4,      &&LABEL3_ConvOvfU2UnI8,      &&LABEL3_ConvOvfU2UnR4,
        &&LABEL3_ConvOvfU2UnR8,      &&LABEL3_ConvOvfI4UnI4,     &&LABEL3_ConvOvfI4UnI8,      &&LABEL3_ConvOvfI4UnR4,      &&LABEL3_ConvOvfI4UnR8,
        &&LABEL3_ConvOvfU4UnI8,      &&LABEL3_ConvOvfU4UnR4,     &&LABEL3_ConvOvfU4UnR8,      &&LABEL3_ConvOvfI8UnI8,      &&LABEL3_ConvOvfI8UnR4,
        &&LABEL3_ConvOvfI8UnR8,      &&LABEL3_ConvOvfU8UnR4,     &&LABEL3_ConvOvfU8UnR8,      &&LABEL3_InitObjI2Unaligned, &&LABEL3_InitObjI4Unaligned,
        &&LABEL3_InitObjI8Unaligned, &&LABEL3_LdfldI1Large,      &&LABEL3_LdfldU1Large,       &&LABEL3_LdfldI2Large,       &&LABEL3_LdfldI2Unaligned,
        &&LABEL3_LdfldU2Large,       &&LABEL3_LdfldU2Unaligned,  &&LABEL3_LdfldI4Large,       &&LABEL3_LdfldI4Unaligned,   &&LABEL3_LdfldI8Large,
        &&LABEL3_LdfldI8Unaligned,   &&LABEL3_LdfldAnyLarge,     &&LABEL3_LdvfldI1Large,      &&LABEL3_LdvfldU1Large,      &&LABEL3_LdvfldI2Large,
        &&LABEL3_LdvfldI2Unaligned,  &&LABEL3_LdvfldU2Large,     &&LABEL3_LdvfldU2Unaligned,  &&LABEL3_LdvfldI4Large,      &&LABEL3_LdvfldI4Unaligned,
        &&LABEL3_LdvfldI8Large,      &&LABEL3_LdvfldI8Unaligned, &&LABEL3_LdvfldAnyLarge,     &&LABEL3_LdfldaLarge,        &&LABEL3_StfldI1Large,
        &&LABEL3_StfldI2Large,       &&LABEL3_StfldI2Unaligned,  &&LABEL3_StfldI4Large,       &&LABEL3_StfldI4Unaligned,   &&LABEL3_StfldI8Large,
        &&LABEL3_StfldI8Unaligned,   &&LABEL3_StfldAnyLarge,
    };
    static void* const in_labels4[] = {
        &&LABEL4_Illegal,
        &&LABEL4_Nop,
        &&LABEL4_Arglist,
    };
    static void* const in_labels5[] = {};

///}}COMPUTED_GOTO_LABELS
#endif
#pragma endregion

    const uint8_t* ip = frame->ip;
    const RtStackObject* ret = frame->eval_stack_base;

method_start:
{
    RtStackObject* const eval_stack_base = frame->eval_stack_base;
    const RtInterpMethodInfo* const imi = frame->method->interp_data;
    // loop_start:
    while (true)
    {
        LEANCLR_SWITCH0()
        {
            ///{{SHORT_INSTRUCTION_CASES
            LEANCLR_CASE_BEGIN0(InitLocals1Short)
            {
                RtStackObject* locals = eval_stack_base + ir->offset;
                locals->value = 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(InitLocals2Short)
            {
                RtStackObject* locals = eval_stack_base + ir->offset;
                locals[0].value = 0;
                locals[1].value = 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(InitLocals3Short)
            {
                RtStackObject* locals = eval_stack_base + ir->offset;
                locals[0].value = 0;
                locals[1].value = 0;
                locals[2].value = 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(InitLocals4Short)
            {
                RtStackObject* locals = eval_stack_base + ir->offset;
                locals[0].value = 0;
                locals[1].value = 0;
                locals[2].value = 0;
                locals[3].value = 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(InitLocalsShort)
            {
                std::memset(eval_stack_base + ir->offset, 0, ir->size * sizeof(RtStackObject));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdLocI1Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i32 = src->i8;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdLocU1Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i32 = src->u8;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdLocI2Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i32 = src->i16;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdLocU2Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i32 = src->u16;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdLocI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i32 = src->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdLocI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i64 = src->i64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdLocAnyShort)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                std::memcpy(dst, src, ir->size * sizeof(RtStackObject));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdLocaShort)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->ptr = src;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StLocI1Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i8 = src->i8;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StLocI2Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i16 = src->i16;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StLocI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i32 = src->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StLocI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i64 = src->i64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StLocAnyShort)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                std::memcpy(dst, src, ir->size * sizeof(RtStackObject));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdNullShort)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                dst->ptr = nullptr;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdcI4I2Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                dst->i32 = static_cast<int32_t>(ir->value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdcI4I4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                dst->i32 = ir->value;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdcI8I2Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                dst->i64 = static_cast<int64_t>(ir->value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdcI8I4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                dst->i64 = static_cast<int64_t>(ir->value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdcI8I8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
                dst->i64 = *(int64_t*)&ir->value_low;
#else
                dst->i64 = (static_cast<int64_t>(ir->value_low)) | ((static_cast<int64_t>(ir->value_high)) << 32);
#endif
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdStrShort)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                vm::RtString* str = get_resolved_data<vm::RtString>(imi, ir->str_idx);
                dst->str = str;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN_LITE0(BrShort)
            {
                const auto* ir = (ll::Br*)ip;
                ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BrTrueI4Short)
            {
                const auto* ir = (ll::BrTrueI4*)ip;
                RtStackObject* cond = eval_stack_base + ir->condition;
                if (cond->i32 != 0)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BrTrueI8Short)
            {
                const auto* ir = (ll::BrTrueI8*)ip;
                RtStackObject* cond = eval_stack_base + ir->condition;
                if (cond->i64 != 0)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BrFalseI4Short)
            {
                const auto* ir = (ll::BrFalseI4*)ip;
                RtStackObject* cond = eval_stack_base + ir->condition;
                if (cond->i32 == 0)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BrFalseI8Short)
            {
                const auto* ir = (ll::BrFalseI8*)ip;
                RtStackObject* cond = eval_stack_base + ir->condition;
                if (cond->i64 == 0)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BeqI4Short)
            {
                const auto* ir = (ll::BeqI4*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->i32 == op2->i32)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BeqI8Short)
            {
                const auto* ir = (ll::BeqI8*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->i64 == op2->i64)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BgeI4Short)
            {
                const auto* ir = (ll::BgeI4*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->i32 >= op2->i32)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BgeI8Short)
            {
                const auto* ir = (ll::BgeI8*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->i64 >= op2->i64)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BgtI4Short)
            {
                const auto* ir = (ll::BgtI4*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->i32 > op2->i32)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BgtI8Short)
            {
                const auto* ir = (ll::BgtI8*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->i64 > op2->i64)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BleI4Short)
            {
                const auto* ir = (ll::BleI4*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->i32 <= op2->i32)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BleI8Short)
            {
                const auto* ir = (ll::BleI8*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->i64 <= op2->i64)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BltI4Short)
            {
                const auto* ir = (ll::BltI4*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->i32 < op2->i32)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BltI8Short)
            {
                const auto* ir = (ll::BltI8*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->i64 < op2->i64)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BneUnI4Short)
            {
                const auto* ir = (ll::BneUnI4*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->u32 != op2->u32)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BneUnI8Short)
            {
                const auto* ir = (ll::BneUnI8*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->u64 != op2->u64)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BgeUnI4Short)
            {
                const auto* ir = (ll::BgeUnI4*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->u32 >= op2->u32)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BgeUnI8Short)
            {
                const auto* ir = (ll::BgeUnI8*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->u64 >= op2->u64)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BgtUnI4Short)
            {
                const auto* ir = (ll::BgtUnI4*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->u32 > op2->u32)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BgtUnI8Short)
            {
                const auto* ir = (ll::BgtUnI8*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->u64 > op2->u64)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BleUnI4Short)
            {
                const auto* ir = (ll::BleUnI4*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->u32 <= op2->u32)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BleUnI8Short)
            {
                const auto* ir = (ll::BleUnI8*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->u64 <= op2->u64)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BltUnI4Short)
            {
                const auto* ir = (ll::BltUnI4*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->u32 < op2->u32)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(BltUnI8Short)
            {
                const auto* ir = (ll::BltUnI8*)ip;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                if (op1->u64 < op2->u64)
                {
                    ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN0(AddI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i32 = src1->i32 + src2->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(AddI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i64 = src1->i64 + src2->i64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(AddR4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->f32 = src1->f32 + src2->f32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(AddR8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->f64 = src1->f64 + src2->f64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(SubI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i32 = src1->i32 - src2->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(SubI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i64 = src1->i64 - src2->i64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(SubR4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->f32 = src1->f32 - src2->f32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(SubR8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->f64 = src1->f64 - src2->f64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(MulI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i32 = src1->i32 * src2->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(MulI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i64 = src1->i64 * src2->i64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(MulR4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->f32 = src1->f32 * src2->f32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(MulR8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->f64 = src1->f64 * src2->f64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(DivI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                int32_t dividend = src1->i32;
                int32_t divisor = src2->i32;
                if (divisor == 0)
                {
                    RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                }
                if (divisor == -1 && dividend == INT32_MIN)
                {
                    RAISE_RUNTIME_ERROR(RtErr::Arithmetic);
                }
                dst->i32 = dividend / divisor;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(DivI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                int64_t dividend = src1->i64;
                int64_t divisor = src2->i64;
                if (divisor == 0)
                {
                    RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                }
                if (divisor == -1 && dividend == INT64_MIN)
                {
                    RAISE_RUNTIME_ERROR(RtErr::Arithmetic);
                }
                dst->i64 = dividend / divisor;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(DivR4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->f32 = src1->f32 / src2->f32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(DivR8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->f64 = src1->f64 / src2->f64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(DivUnI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                uint32_t dividend = src1->u32;
                uint32_t divisor = src2->u32;
                if (divisor == 0)
                {
                    RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                }
                dst->u32 = dividend / divisor;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(DivUnI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                uint64_t dividend = src1->u64;
                uint64_t divisor = src2->u64;
                if (divisor == 0)
                {
                    RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                }
                dst->u64 = dividend / divisor;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(RemI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                int32_t dividend = src1->i32;
                int32_t divisor = src2->i32;
                if (divisor == 0)
                {
                    RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                }
                if (divisor == -1 && dividend == INT32_MIN)
                {
                    RAISE_RUNTIME_ERROR(RtErr::Arithmetic);
                }
                else
                {
                    dst->i32 = dividend % divisor;
                }
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(RemI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                int64_t dividend = src1->i64;
                int64_t divisor = src2->i64;
                if (divisor == 0)
                {
                    RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                }
                if (divisor == -1 && dividend == INT64_MIN)
                {
                    RAISE_RUNTIME_ERROR(RtErr::Arithmetic);
                }
                else
                {
                    dst->i64 = dividend % divisor;
                }
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(RemR4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->f32 = std::fmod(src1->f32, src2->f32);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(RemR8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->f64 = std::fmod(src1->f64, src2->f64);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(RemUnI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                uint32_t dividend = src1->u32;
                uint32_t divisor = src2->u32;
                if (divisor == 0)
                {
                    RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                }
                dst->u32 = dividend % divisor;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(RemUnI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                uint64_t dividend = src1->u64;
                uint64_t divisor = src2->u64;
                if (divisor == 0)
                {
                    RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                }
                dst->u64 = dividend % divisor;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(AndI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i32 = src1->i32 & src2->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(AndI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i64 = src1->i64 & src2->i64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(OrI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i32 = src1->i32 | src2->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(OrI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i64 = src1->i64 | src2->i64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(XorI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i32 = src1->i32 ^ src2->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(XorI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i64 = src1->i64 ^ src2->i64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ShlI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i32 = src1->i32 << src2->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ShrI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->i32 = src1->i32 >> src2->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ShrUnI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src1 = eval_stack_base + ir->arg1;
                RtStackObject* src2 = eval_stack_base + ir->arg2;
                dst->u32 = src1->u32 >> src2->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(NegI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i32 = -src->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(NegI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i64 = -src->i64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(NegR4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->f32 = -src->f32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(NegR8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->f64 = -src->f64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(NotI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i32 = ~src->i32;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(NotI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* src = eval_stack_base + ir->src;
                dst->i64 = ~src->i64;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI1I4Short)
            {
                int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                int8_t value = static_cast<int8_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI1I8Short)
            {
                int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                int8_t value = static_cast<int8_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI1R4Short)
            {
                float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                int32_t value = cast_float_to_small_int<float, int8_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI1R8Short)
            {
                double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                int32_t value = cast_float_to_small_int<double, int8_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvU1I4Short)
            {
                int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                uint8_t value = static_cast<uint8_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvU1I8Short)
            {
                int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                uint8_t value = static_cast<uint8_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvU1R4Short)
            {
                float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                int32_t value = cast_float_to_small_int<float, uint8_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvU1R8Short)
            {
                double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                int32_t value = cast_float_to_small_int<double, uint8_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI2I4Short)
            {
                int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                int16_t value = static_cast<int16_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI2I8Short)
            {
                int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                int16_t value = static_cast<int16_t>(src);
                set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI2R4Short)
            {
                float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                int32_t value = cast_float_to_small_int<float, int16_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI2R8Short)
            {
                double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                int32_t value = cast_float_to_small_int<double, int16_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvU2I4Short)
            {
                int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                uint16_t value = static_cast<uint16_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvU2I8Short)
            {
                int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                uint16_t value = static_cast<uint16_t>(src);
                set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvU2R4Short)
            {
                float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                int32_t value = cast_float_to_small_int<float, uint16_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvU2R8Short)
            {
                double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                int32_t value = cast_float_to_small_int<double, uint16_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI4I8Short)
            {
                int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, src);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI4R4Short)
            {
                float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                int32_t value = cast_float_to_i32<float, int32_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI4R8Short)
            {
                double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                int32_t value = cast_float_to_i32<double, int32_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvU4I8Short)
            {
                uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
                set_stack_value_at<uint32_t>(eval_stack_base, ir->dst, src);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvU4R4Short)
            {
                float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                int32_t value = cast_float_to_i32<float, uint32_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvU4R8Short)
            {
                double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                int32_t value = cast_float_to_i32<double, uint32_t>(src);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI8I4Short)
            {
                int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(src));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI8R4Short)
            {
                float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                int64_t value = cast_float_to_i64<float, int64_t>(src);
                set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvI8R8Short)
            {
                double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                int64_t value = cast_float_to_i64<double, int64_t>(src);
                set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvR4I4Short)
            {
                int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                set_stack_value_at<float>(eval_stack_base, ir->dst, static_cast<float>(src));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvR4I8Short)
            {
                int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                set_stack_value_at<float>(eval_stack_base, ir->dst, static_cast<float>(src));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvR4R8Short)
            {
                double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                set_stack_value_at<float>(eval_stack_base, ir->dst, static_cast<float>(src));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvR8I4Short)
            {
                int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                set_stack_value_at<double>(eval_stack_base, ir->dst, static_cast<double>(src));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvR8I8Short)
            {
                int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                set_stack_value_at<double>(eval_stack_base, ir->dst, static_cast<double>(src));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ConvR8R4Short)
            {
                float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                set_stack_value_at<double>(eval_stack_base, ir->dst, static_cast<double>(src));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CeqI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                dst->i32 = (op1->i32 == op2->i32) ? 1 : 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CeqI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                dst->i32 = (op1->i64 == op2->i64) ? 1 : 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CeqR4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                float left = op1->f32;
                float right = op2->f32;
                dst->i32 = (left == right && !std::isunordered(left, right)) ? 1 : 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CeqR8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                double left = op1->f64;
                double right = op2->f64;
                dst->i32 = (left == right && !std::isunordered(left, right)) ? 1 : 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CgtI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                dst->i32 = (op1->i32 > op2->i32) ? 1 : 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CgtI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                dst->i32 = (op1->i64 > op2->i64) ? 1 : 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CgtUnI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                dst->i32 = (op1->u32 > op2->u32) ? 1 : 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CgtUnI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                dst->i32 = (op1->u64 > op2->u64) ? 1 : 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CltI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                dst->i32 = (op1->i32 < op2->i32) ? 1 : 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CltI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                dst->i32 = (op1->i64 < op2->i64) ? 1 : 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CltUnI4Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                dst->i32 = (op1->u32 < op2->u32) ? 1 : 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CltUnI8Short)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* op1 = eval_stack_base + ir->arg1;
                RtStackObject* op2 = eval_stack_base + ir->arg2;
                dst->i32 = (op1->u64 < op2->u64) ? 1 : 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(InitObjI1Short)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->addr;
                int8_t* addr = reinterpret_cast<int8_t*>(addr_obj->ptr);
                *addr = 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(InitObjI2Short)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->addr;
                int16_t* addr = reinterpret_cast<int16_t*>(addr_obj->ptr);
                *addr = 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(InitObjI4Short)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->addr;
                int32_t* addr = reinterpret_cast<int32_t*>(addr_obj->ptr);
                *addr = 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(InitObjI8Short)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->addr;
                int64_t* addr = reinterpret_cast<int64_t*>(addr_obj->ptr);
                *addr = 0;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(InitObjAnyShort)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->addr;
                uint8_t* addr = reinterpret_cast<uint8_t*>(addr_obj->ptr);
                std::memset(addr, 0, ir->size);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CpObjI1Short)
            {
                RtStackObject* dst_obj = eval_stack_base + ir->dst;
                RtStackObject* src_obj = eval_stack_base + ir->src;
                int8_t* dst_addr = reinterpret_cast<int8_t*>(dst_obj->ptr);
                const int8_t* src_addr = reinterpret_cast<const int8_t*>(src_obj->ptr);
                *dst_addr = *src_addr;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CpObjI2Short)
            {
                RtStackObject* dst_obj = eval_stack_base + ir->dst;
                RtStackObject* src_obj = eval_stack_base + ir->src;
                int16_t* dst_addr = reinterpret_cast<int16_t*>(dst_obj->ptr);
                const int16_t* src_addr = reinterpret_cast<const int16_t*>(src_obj->ptr);
                *dst_addr = *src_addr;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CpObjI4Short)
            {
                RtStackObject* dst_obj = eval_stack_base + ir->dst;
                RtStackObject* src_obj = eval_stack_base + ir->src;
                int32_t* dst_addr = reinterpret_cast<int32_t*>(dst_obj->ptr);
                const int32_t* src_addr = reinterpret_cast<const int32_t*>(src_obj->ptr);
                *dst_addr = *src_addr;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CpObjI8Short)
            {
                RtStackObject* dst_obj = eval_stack_base + ir->dst;
                RtStackObject* src_obj = eval_stack_base + ir->src;
                int64_t* dst_addr = reinterpret_cast<int64_t*>(dst_obj->ptr);
                const int64_t* src_addr = reinterpret_cast<const int64_t*>(src_obj->ptr);
                *dst_addr = *src_addr;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CpObjAnyShort)
            {
                RtStackObject* dst_obj = eval_stack_base + ir->dst;
                RtStackObject* src_obj = eval_stack_base + ir->src;
                std::memcpy(dst_obj->ptr, src_obj->cptr, ir->size);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdObjAnyShort)
            {
                RtStackObject* dst = eval_stack_base + ir->dst;
                RtStackObject* addr_obj = eval_stack_base + ir->addr;
                std::memcpy(dst, addr_obj->cptr, ir->size);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StObjAnyShort)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->addr;
                RtStackObject* src = eval_stack_base + ir->src;
                std::memcpy(addr_obj->ptr, src, ir->size);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(CastClassShort)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (obj)
                {
                    metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                    if (!vm::Class::is_assignable_from(obj->klass, to_class))
                    {
                        RAISE_RUNTIME_ERROR(RtErr::InvalidCast);
                    }
                }
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(IsInstShort)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                vm::RtObject* result;
                if (obj)
                {
                    metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                    result = vm::Class::is_assignable_from(obj->klass, to_class) ? obj : nullptr;
                }
                else
                {
                    result = nullptr;
                }
                set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, result);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(BoxShort)
            {
                RtStackObject* src = eval_stack_base + ir->src;
                metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                HANDLE_RAISE_RUNTIME_ERROR(vm::RtObject*, boxed_obj, vm::Object::box_object(to_class, src));
                set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, boxed_obj);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(UnboxShort)
            {
                vm::RtObject* boxed_obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!boxed_obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                HANDLE_RAISE_RUNTIME_ERROR(const void*, unbox_addr, vm::Object::unbox_ex(boxed_obj, to_class));
                set_stack_value_at(eval_stack_base, ir->dst, unbox_addr);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(UnboxAnyShort)
            {
                vm::RtObject* boxed_obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                RtStackObject* dst = eval_stack_base + ir->dst;
                HANDLE_RAISE_RUNTIME_ERROR_VOID(vm::Object::unbox_any(boxed_obj, to_class, dst, true));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(NewArrShort)
            {
                int32_t length = get_stack_value_at<int32_t>(eval_stack_base, ir->length);
                metadata::RtClass* element_class = get_resolved_data<metadata::RtClass>(imi, ir->arr_klass_idx);
                HANDLE_RAISE_RUNTIME_ERROR(vm::RtArray*, new_array, vm::Array::new_szarray_from_array_klass(element_class, length));
                set_stack_value_at<vm::RtArray*>(eval_stack_base, ir->dst, new_array);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdLenShort)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                set_stack_value_at(eval_stack_base, ir->dst, vm::Array::get_array_length(array));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemaShort)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                const metadata::RtClass* arr_klass = array->klass;
                const metadata::RtClass* element_klass = vm::Class::get_array_element_class(arr_klass);
                const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->ele_klass_idx);
                if (!vm::Class::is_pointer_element_compatible_with(element_klass, check_klass))
                {
                    RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
                }
                const void* element_addr = vm::Array::get_array_element_address_as_ptr_void(array, index);
                set_stack_value_at(eval_stack_base, ir->dst, element_addr);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemI1Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 1);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                int8_t value = vm::Array::get_array_data_at<int8_t>(array, index);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemU1Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 1);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                uint8_t value = vm::Array::get_array_data_at<uint8_t>(array, index);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemI2Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 2);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                int16_t value = vm::Array::get_array_data_at<int16_t>(array, index);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemU2Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 2);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                uint16_t value = vm::Array::get_array_data_at<uint16_t>(array, index);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemI4Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 4);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                int32_t value = vm::Array::get_array_data_at<int32_t>(array, index);
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemI8Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 8);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                int64_t value = vm::Array::get_array_data_at<int64_t>(array, index);
                set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemIShort)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == PTR_SIZE);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                intptr_t value = vm::Array::get_array_data_at<intptr_t>(array, index);
                set_stack_value_at<intptr_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemR4Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 4);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                float value = vm::Array::get_array_data_at<float>(array, index);
                set_stack_value_at<float>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemR8Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 8);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                double value = vm::Array::get_array_data_at<double>(array, index);
                set_stack_value_at<double>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemRefShort)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == PTR_SIZE);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                vm::RtObject* value = vm::Array::get_array_data_at<vm::RtObject*>(array, index);
                set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemAnyRefShort)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                vm::RtObject* value = vm::Array::get_array_data_at<vm::RtObject*>(array, index);
                const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->ele_klass_idx);
                if (value && !vm::Class::is_assignable_from(value->klass, check_klass))
                {
                    RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
                }
                set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdelemAnyValShort)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                // const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(array);
                // const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->ele_klass_idx);
                // if (!vm::Class::is_pointer_element_compatible_with(ele_klass, check_klass))
                // {
                //     RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
                // }
                assert(ir->ele_size == vm::Array::get_array_element_size(array));
                const void* src_addr = vm::Array::get_array_element_address_with_size_as_ptr_void(array, index, ir->ele_size);
                RtStackObject* dst = eval_stack_base + ir->dst;
                std::memcpy(dst, src_addr, ir->ele_size);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StelemI1Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 1);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                int8_t value = get_stack_value_at<int8_t>(eval_stack_base, ir->value);
                vm::Array::set_array_data_at<int8_t>(array, index, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StelemI2Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 2);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->value);
                vm::Array::set_array_data_at<int16_t>(array, index, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StelemI4Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 4);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
                vm::Array::set_array_data_at<int32_t>(array, index, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StelemI8Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 8);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
                vm::Array::set_array_data_at<int64_t>(array, index, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StelemIShort)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == PTR_SIZE);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                intptr_t value = get_stack_value_at<intptr_t>(eval_stack_base, ir->value);
                vm::Array::set_array_data_at<intptr_t>(array, index, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StelemR4Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 4);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                float value = get_stack_value_at<float>(eval_stack_base, ir->value);
                vm::Array::set_array_data_at<float>(array, index, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StelemR8Short)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == 8);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                double value = get_stack_value_at<double>(eval_stack_base, ir->value);
                vm::Array::set_array_data_at<double>(array, index, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StelemRefShort)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                assert(vm::Array::get_array_element_size(array) == PTR_SIZE);
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                vm::RtObject* value = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->value);
                const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(array);
                if (value && !vm::Class::is_assignable_from(value->klass, ele_klass))
                {
                    RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
                }
                vm::Array::set_array_data_at<vm::RtObject*>(array, index, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StelemAnyRefShort)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                vm::RtObject* value = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->value);
                const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(array);
                if (value && !vm::Class::is_assignable_from(value->klass, ele_klass))
                {
                    RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
                }
                vm::Array::set_array_data_at<vm::RtObject*>(array, index, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StelemAnyValShort)
            {
                vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                if (!array)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                if (vm::Array::is_out_of_range(array, index))
                {
                    RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                }
                const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(array);
                const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->ele_klass_idx);
                if (!vm::Class::is_pointer_element_compatible_with(ele_klass, check_klass))
                {
                    RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
                }
                assert(ir->ele_size == vm::Array::get_array_element_size(array));
                RtStackObject* src = eval_stack_base + ir->value;
                void* dst_addr = const_cast<void*>(vm::Array::get_array_element_address_with_size_as_ptr_void(array, index, ir->ele_size));
                std::memcpy(dst_addr, src, ir->ele_size);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdftnShort)
            {
                metadata::RtMethodInfo* method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                set_stack_value_at(eval_stack_base, ir->dst, reinterpret_cast<void*>(method));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdvirtftnShort)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                const metadata::RtMethodInfo* method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                assert(method->slot < method->parent->vtable_count);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, actual_method, vm::Method::get_virtual_method_impl(obj, method));
                set_stack_value_at(eval_stack_base, ir->dst, actual_method);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdfldI1Short)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                const int8_t* field_addr = reinterpret_cast<const int8_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                int8_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdfldU1Short)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                uint8_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdfldI2Short)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                const int16_t* field_addr = reinterpret_cast<const int16_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                int16_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdfldU2Short)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                const uint16_t* field_addr = reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                uint16_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdfldI4Short)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                const int32_t* field_addr = reinterpret_cast<const int32_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                int32_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdfldI8Short)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                const int64_t* field_addr = reinterpret_cast<const int64_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                int64_t value = *field_addr;
                set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdfldAnyShort)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                RtStackObject* dst = eval_stack_base + ir->dst;
                std::memcpy(dst, field_addr, ir->size);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdvfldI1Short)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->obj;
                const int8_t* field_addr = reinterpret_cast<const int8_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
                int8_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdvfldU1Short)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->obj;
                const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset;
                uint8_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdvfldI2Short)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->obj;
                const int16_t* field_addr = reinterpret_cast<const int16_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
                int16_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdvfldU2Short)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->obj;
                const uint16_t* field_addr = reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
                uint16_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdvfldI4Short)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->obj;
                const int32_t* field_addr = reinterpret_cast<const int32_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
                int32_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdvfldI8Short)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->obj;
                const int64_t* field_addr = reinterpret_cast<const int64_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
                int64_t value = *field_addr;
                set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdvfldAnyShort)
            {
                RtStackObject* addr_obj = eval_stack_base + ir->obj;
                const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset;
                RtStackObject* dst = eval_stack_base + ir->dst;
                std::memmove(dst, field_addr, ir->size);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdfldaShort)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                set_stack_value_at(eval_stack_base, ir->dst, field_addr);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StfldI1Short)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                int8_t value = get_stack_value_at<int8_t>(eval_stack_base, ir->value);
                int8_t* field_addr = reinterpret_cast<int8_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
                *field_addr = value;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StfldI2Short)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->value);
                int16_t* field_addr = reinterpret_cast<int16_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
                *field_addr = value;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StfldI4Short)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
                int32_t* field_addr = reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
                *field_addr = value;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StfldI8Short)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
                int64_t* field_addr = reinterpret_cast<int64_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
                *field_addr = value;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StfldAnyShort)
            {
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                RtStackObject* src = eval_stack_base + ir->value;
                uint8_t* field_addr = reinterpret_cast<uint8_t*>(obj) + ir->offset;
                std::memcpy(field_addr, src, ir->size);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdsfldI1Short)
            {
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                const int8_t* field_addr = get_static_field_address<int8_t>(field);
                int8_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdsfldU1Short)
            {
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                const uint8_t* field_addr = get_static_field_address<uint8_t>(field);
                uint8_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdsfldI2Short)
            {
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                const int16_t* field_addr = get_static_field_address<int16_t>(field);
                int16_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdsfldU2Short)
            {
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                const uint16_t* field_addr = get_static_field_address<uint16_t>(field);
                uint16_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdsfldI4Short)
            {
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                const int32_t* field_addr = get_static_field_address<int32_t>(field);
                int32_t value = *field_addr;
                set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdsfldI8Short)
            {
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                const int64_t* field_addr = get_static_field_address<int64_t>(field);
                int64_t value = *field_addr;
                set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdsfldAnyShort)
            {
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                const uint8_t* field_addr = get_static_field_address<uint8_t>(field);
                RtStackObject* dst = eval_stack_base + ir->dst;
                std::memcpy(dst, field_addr, ir->size);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdsfldaShort)
            {
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                metadata::RtClass* klass = field->parent;
                TRY_RUN_CLASS_STATIC_CCTOR(klass);
                const void* field_addr = get_static_field_address<void>(field);
                set_stack_value_at(eval_stack_base, ir->dst, field_addr);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(LdsfldRvaDataShort)
            {
                const void* rva_data = get_resolved_data<void>(imi, ir->data);
                set_stack_value_at(eval_stack_base, ir->dst, rva_data);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StsfldI1Short)
            {
                int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                metadata::RtClass* klass = field->parent;
                TRY_RUN_CLASS_STATIC_CCTOR(klass);
                int8_t* field_addr = get_static_field_address<int8_t>(field);
                *field_addr = static_cast<int8_t>(value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StsfldI2Short)
            {
                int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                metadata::RtClass* klass = field->parent;
                TRY_RUN_CLASS_STATIC_CCTOR(klass);
                int16_t* field_addr = get_static_field_address<int16_t>(field);
                *field_addr = static_cast<int16_t>(value);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StsfldI4Short)
            {
                int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                metadata::RtClass* klass = field->parent;
                TRY_RUN_CLASS_STATIC_CCTOR(klass);
                int32_t* field_addr = get_static_field_address<int32_t>(field);
                *field_addr = value;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StsfldI8Short)
            {
                int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                metadata::RtClass* klass = field->parent;
                TRY_RUN_CLASS_STATIC_CCTOR(klass);
                int64_t* field_addr = get_static_field_address<int64_t>(field);
                *field_addr = value;
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(StsfldAnyShort)
            {
                RtStackObject* src = eval_stack_base + ir->value;
                const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                metadata::RtClass* klass = field->parent;
                TRY_RUN_CLASS_STATIC_CCTOR(klass);
                uint8_t* field_addr = get_static_field_address<uint8_t>(field);
                std::memcpy(field_addr, src, ir->size);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(RetVoidShort)
            {
                LEAVE_FRAME();
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(RetI4Short)
            {
                eval_stack_base->i32 = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                LEAVE_FRAME();
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(RetI8Short)
            {
                eval_stack_base->i64 = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                LEAVE_FRAME();
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(RetAnyShort)
            {
                std::memcpy(eval_stack_base, eval_stack_base + ir->src, ir->size * sizeof(RtStackObject));
                LEAVE_FRAME();
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN_LITE0(RetNopShort)
            {
                LEAVE_FRAME();
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(CallInterpShort)
            {
                const auto* ir = reinterpret_cast<const ll::CallInterpShort*>(ip);
                const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                if (vm::Method::is_static(target_method))
                {
                    TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                }
                const uint8_t* next_ip = reinterpret_cast<const uint8_t*>(ir + 1);
                ENTER_INTERP_FRAME(target_method, ir->frame_base, next_ip);
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(CallVirtInterpShort)
            {
                const auto* ir = reinterpret_cast<const ll::CallVirtInterpShort*>(ip);
                vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->frame_base);
                if (!obj)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                const metadata::RtMethodInfo* original_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, actual_method,
                                                        vm::Method::get_virtual_method_impl(obj, original_method));
                if (actual_method->invoker_type == metadata::RtInvokerType::Interpreter)
                {
                    if (vm::Class::is_value_type(actual_method->parent))
                    {
                        set_stack_value_at(eval_stack_base, ir->frame_base, obj + 1);
                    }
                    ENTER_INTERP_FRAME(actual_method, ir->frame_base, reinterpret_cast<const uint8_t*>(ir + 1));
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                    RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                    HANDLE_RAISE_RUNTIME_ERROR_VOID(CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(actual_method->invoke_method_ptr)(
                        actual_method->virtual_method_ptr, actual_method, frame_base, frame_base));
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(CallInternalCallShort)
            {
                const auto* ir = reinterpret_cast<const ll::CallInternalCallShort*>(ip);
                const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                if (vm::Method::is_static(target_method))
                {
                    TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                }
                ip = reinterpret_cast<const uint8_t*>(ir + 1);
                RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(CallIntrinsicShort)
            {
                const auto* ir = reinterpret_cast<const ll::CallIntrinsicShort*>(ip);
                const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                if (vm::Method::is_static(target_method))
                {
                    TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                }
                ip = reinterpret_cast<const uint8_t*>(ir + 1);
                RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(CallPInvokeShort)
            {
                const auto* ir = reinterpret_cast<const ll::CallPInvokeShort*>(ip);
                const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                if (vm::Method::is_static(target_method))
                {
                    TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                }
                ip = reinterpret_cast<const uint8_t*>(ir + 1);
                RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(CallAotShort)
            {
                const auto* ir = reinterpret_cast<const ll::CallAotShort*>(ip);
                const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                if (vm::Method::is_static(target_method))
                {
                    TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                }
                ip = reinterpret_cast<const uint8_t*>(ir + 1);
                RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(CallRuntimeImplementedShort)
            {
                const auto* ir = reinterpret_cast<const ll::CallRuntimeImplementedShort*>(ip);
                const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                if (vm::Method::is_static(target_method))
                {
                    TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                }
                ip = reinterpret_cast<const uint8_t*>(ir + 1);
                RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(CalliInterpShort)
            {
                const auto* ir = reinterpret_cast<const ll::CalliInterpShort*>(ip);
                const metadata::RtMethodSig* method_sig = get_resolved_data<const metadata::RtMethodSig>(imi, ir->method_sig_idx);
                const uint8_t* next_ip = reinterpret_cast<const uint8_t*>(ir + 1);
                const metadata::RtMethodInfo* target_method = get_stack_value_at<const metadata::RtMethodInfo*>(eval_stack_base, ir->method_idx);
                if (target_method->parameter_count != method_sig->params.size())
                {
                    RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
                }
                if (target_method->invoker_type == metadata::RtInvokerType::Interpreter)
                {
                    ENTER_INTERP_FRAME(target_method, ir->frame_base, reinterpret_cast<const uint8_t*>(ir + 1));
                }
                else
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                    RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                    HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN0(BoxRefInplaceShort)
            {
                const void* src_ptr = get_stack_value_at<const void*>(eval_stack_base, ir->src);
                metadata::RtClass* to_klass = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                HANDLE_RAISE_RUNTIME_ERROR(vm::RtObject*, boxed_obj, vm::Object::box_object(to_klass, src_ptr));
                set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, boxed_obj);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN_LITE0(NewObjInterpShort)
            {
                const auto* ir = reinterpret_cast<const ll::NewObjInterpShort*>(ip);
                const metadata::RtMethodInfo* ctor = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                const metadata::RtClass* klass = ctor->parent;
                TRY_RUN_CLASS_STATIC_CCTOR(klass);
                HANDLE_RAISE_RUNTIME_ERROR(vm::RtObject*, obj, vm::Object::new_object(klass));
                RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                std::memmove(frame_base + 1, frame_base, static_cast<size_t>(ir->total_params_stack_object_size) * sizeof(RtStackObject));
                frame_base->obj = obj;
                ENTER_INTERP_FRAME(ctor, ir->frame_base, reinterpret_cast<const uint8_t*>(ir + 1));
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(NewValueTypeInterpShort)
            {
                const auto* ir = reinterpret_cast<const ll::NewValueTypeInterpShort*>(ip);
                const metadata::RtMethodInfo* ctor = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                const metadata::RtClass* klass = ctor->parent;
                RtStackObject* original_frame_base = eval_stack_base + ir->frame_base;
                const size_t value_stack_objects = InterpDefs::get_stack_object_size_by_byte_size(klass->instance_size_without_header);
                RtStackObject* final_frame_base = original_frame_base + value_stack_objects;
                std::memmove(final_frame_base + 1, original_frame_base, static_cast<size_t>(ir->total_params_stack_object_size) * sizeof(RtStackObject));
                final_frame_base->ptr = original_frame_base;
                std::memset(original_frame_base, 0, value_stack_objects * sizeof(RtStackObject));
                ENTER_INTERP_FRAME(ctor, ir->frame_base + value_stack_objects, reinterpret_cast<const uint8_t*>(ir + 1));
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN0(NewObjInternalCallShort)
            {
                const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                vm::InternalCallInvoker invoker = vm::InternalCalls::get_internal_call_invoker_by_id_unchecked(ir->invoker_idx);
                RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                HANDLE_RAISE_RUNTIME_ERROR_VOID(invoker(target_method->method_ptr, target_method, frame_base, frame_base));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(NewObjIntrinsicShort)
            {
                const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                vm::InternalCallInvoker invoker = vm::Intrinsics::get_intrinsic_invoker_by_id_unchecked(ir->invoker_idx);
                RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                HANDLE_RAISE_RUNTIME_ERROR_VOID(invoker(target_method->method_ptr, target_method, frame_base, frame_base));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(NewObjAotShort)
            {
                const metadata::RtMethodInfo* ctor = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                const metadata::RtClass* klass = ctor->parent;
                TRY_RUN_CLASS_STATIC_CCTOR(klass);
                HANDLE_RAISE_RUNTIME_ERROR(vm::RtObject*, obj, vm::Object::new_object(klass));
                RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                std::memmove(frame_base + 1, frame_base, static_cast<size_t>(ir->total_params_stack_object_size) * sizeof(RtStackObject));
                frame_base->obj = obj;
                auto invoker = CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(ctor->invoke_method_ptr);
                HANDLE_RAISE_RUNTIME_ERROR_VOID(invoker(ctor->method_ptr, ctor, frame_base, frame_base));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(NewValueTypeAotShort)
            {
                const metadata::RtMethodInfo* ctor = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                const metadata::RtClass* klass = ctor->parent;
                RtStackObject* original_frame_base = eval_stack_base + ir->frame_base;
                const size_t value_stack_objects = InterpDefs::get_stack_object_size_by_byte_size(klass->instance_size_without_header);
                RtStackObject* final_frame_base = original_frame_base + value_stack_objects;
                std::memmove(final_frame_base + 1, original_frame_base, static_cast<size_t>(ir->total_params_stack_object_size) * sizeof(RtStackObject));
                final_frame_base->ptr = original_frame_base;
                std::memset(original_frame_base, 0, value_stack_objects * sizeof(RtStackObject));
                auto invoker = CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(ctor->invoke_method_ptr);
                HANDLE_RAISE_RUNTIME_ERROR_VOID(invoker(ctor->method_ptr, ctor, final_frame_base, final_frame_base));
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(ThrowShort)
            {
                vm::RtException* ex = get_stack_value_at<vm::RtException*>(eval_stack_base, ir->ex);
                if (!ex)
                {
                    RAISE_RUNTIME_ERROR(RtErr::NullReference);
                }
                if (!vm::Class::is_exception_sub_class(ex->klass))
                {
                    RAISE_RUNTIME_ERROR(RtErr::InvalidCast);
                }
                RAISE_RUNTIME_EXCEPTION(ex);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN0(RethrowShort)
            {
                vm::RtException* ex = find_exception_in_enclosing_throw_flow(frame, static_cast<uint32_t>(ip - imi->codes));
                if (!ex)
                {
                    RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
                }
                RAISE_RUNTIME_EXCEPTION(ex);
            }
            LEANCLR_CASE_END0()
            LEANCLR_CASE_BEGIN_LITE0(LeaveTryWithFinallyShort)
            {
                const auto* ir = reinterpret_cast<const ll::LeaveTryWithFinallyShort*>(ip);
                assert(ir->finally_clauses_count > 0);
                assert(ir->first_finally_clause_index < imi->exception_clause_count);
                const uint8_t* target_ip = ip + ir->target_offset;
                const RtInterpExceptionClause* finally_clause = &imi->exception_clauses[ir->first_finally_clause_index];
                push_leave_flow(frame, ip, target_ip, finally_clause, ir->first_finally_clause_index + 1, ir->finally_clauses_count - 1);
                ip = imi->codes + finally_clause->handler_begin_offset;
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(LeaveCatchWithFinallyShort)
            {
                const auto* ir = reinterpret_cast<const ll::LeaveCatchWithFinallyShort*>(ip);
                assert(ir->finally_clauses_count > 0);
                assert(ir->first_finally_clause_index < imi->exception_clause_count);
                vm::RtException* ex = get_exception_in_last_throw_flow(frame, static_cast<uint32_t>(ip - imi->codes));
                pop_throw_flow(ex, frame);
                const uint8_t* target_ip = ip + ir->target_offset;
                const RtInterpExceptionClause* finally_clause = &imi->exception_clauses[ir->first_finally_clause_index];
                push_leave_flow(frame, ip, target_ip, finally_clause, ir->first_finally_clause_index + 1, ir->finally_clauses_count - 1);
                ip = imi->codes + finally_clause->handler_begin_offset;
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(LeaveCatchWithoutFinallyShort)
            {
                const auto* ir = reinterpret_cast<const ll::LeaveCatchWithoutFinallyShort*>(ip);
                vm::RtException* ex = get_exception_in_last_throw_flow(frame, static_cast<uint32_t>(ip - imi->codes));
                pop_throw_flow(ex, frame);
                ip += ir->target_offset;
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(EndFilterShort)
            {
                const auto* ir = reinterpret_cast<const ll::EndFilterShort*>(ip);
                int32_t cond = get_stack_value_at<int32_t>(eval_stack_base, ir->cond);
                if (cond)
                {
                    ip = reinterpret_cast<const uint8_t*>(ir + 1);
                    setup_filter_handler(imi, frame, ip);
                    vm::RtException* ex = get_exception_in_last_throw_flow(frame, static_cast<uint32_t>(ip - imi->codes));
                    set_stack_value_at<vm::RtObject*>(eval_stack_base, imi->total_arg_and_local_stack_object_size, ex);
                }
                else
                {
                    goto unwind_exception_handler;
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(EndFinallyShort)
            {
                goto unwind_exception_handler;
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(EndFaultShort)
            {
                goto unwind_exception_handler;
            }
            LEANCLR_CASE_END_LITE0()

            ///}}SHORT_INSTRUCTION_CASES
            LEANCLR_CASE_BEGIN_LITE0(__UnusedF9)
            {
                assert(false && "Unused opcode");
                RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(Prefix1)
            {
                LEANCLR_SWITCH1()
                {
                    LEANCLR_CASE_BEGIN1(InitLocals)
                    {
                        std::memset(eval_stack_base + ir->offset, 0, ir->size * sizeof(RtStackObject));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdLocI1)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i32 = src->i8;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdLocU1)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i32 = src->u8;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdLocI2)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i32 = src->i16;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdLocU2)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i32 = src->u16;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdLocI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i32 = src->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdLocI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i64 = src->i64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdLocAny)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        std::memcpy(dst, src, ir->size * sizeof(RtStackObject));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdLoca)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->ptr = src;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StLocI1)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i8 = src->i8;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StLocI2)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i16 = src->i16;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StLocI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i32 = src->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StLocI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i64 = src->i64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StLocAny)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        std::memcpy(dst, src, ir->size * sizeof(RtStackObject));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdNull)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        dst->ptr = nullptr;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdcI4I2)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        dst->i32 = static_cast<int32_t>(ir->value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdcI4I4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        dst->i32 = ir->value;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdcI8I2)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        dst->i64 = static_cast<int64_t>(ir->value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdcI8I4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        dst->i64 = static_cast<int64_t>(ir->value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdcI8I8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
#if LEANCLR_SUPPORT_UNALIGNED_ACCESS
                        dst->i64 = *(int64_t*)&ir->value_low;
#else
                        dst->i64 = (static_cast<int64_t>(ir->value_low)) | ((static_cast<int64_t>(ir->value_high)) << 32);
#endif
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdStr)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        vm::RtString* str = get_resolved_data<vm::RtString>(imi, ir->str_idx);
                        dst->str = str;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN_LITE1(Br)
                    {
                        const auto* ir = (ll::Br*)ip;
                        ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BrTrueI4)
                    {
                        const auto* ir = (ll::BrTrueI4*)ip;
                        RtStackObject* cond = eval_stack_base + ir->condition;
                        if (cond->i32 != 0)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BrTrueI8)
                    {
                        const auto* ir = (ll::BrTrueI8*)ip;
                        RtStackObject* cond = eval_stack_base + ir->condition;
                        if (cond->i64 != 0)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BrFalseI4)
                    {
                        const auto* ir = (ll::BrFalseI4*)ip;
                        RtStackObject* cond = eval_stack_base + ir->condition;
                        if (cond->i32 == 0)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BrFalseI8)
                    {
                        const auto* ir = (ll::BrFalseI8*)ip;
                        RtStackObject* cond = eval_stack_base + ir->condition;
                        if (cond->i64 == 0)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BeqI4)
                    {
                        const auto* ir = (ll::BeqI4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->i32 == op2->i32)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BeqI8)
                    {
                        const auto* ir = (ll::BeqI8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->i64 == op2->i64)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BeqR4)
                    {
                        const auto* ir = (ll::BeqR4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        if (left == right && !std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BeqR8)
                    {
                        const auto* ir = (ll::BeqR8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        if (left == right && !std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgeI4)
                    {
                        const auto* ir = (ll::BgeI4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->i32 >= op2->i32)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgeI8)
                    {
                        const auto* ir = (ll::BgeI8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->i64 >= op2->i64)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgeR4)
                    {
                        const auto* ir = (ll::BgeR4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        if (left >= right && !std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgeR8)
                    {
                        const auto* ir = (ll::BgeR8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        if (left >= right && !std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgtI4)
                    {
                        const auto* ir = (ll::BgtI4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->i32 > op2->i32)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgtI8)
                    {
                        const auto* ir = (ll::BgtI8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->i64 > op2->i64)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgtR4)
                    {
                        const auto* ir = (ll::BgtR4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        if (left > right && !std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgtR8)
                    {
                        const auto* ir = (ll::BgtR8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        if (left > right && !std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BleI4)
                    {
                        const auto* ir = (ll::BleI4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->i32 <= op2->i32)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BleI8)
                    {
                        const auto* ir = (ll::BleI8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->i64 <= op2->i64)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BleR4)
                    {
                        const auto* ir = (ll::BleR4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        if (left <= right && !std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BleR8)
                    {
                        const auto* ir = (ll::BleR8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        if (left <= right && !std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BltI4)
                    {
                        const auto* ir = (ll::BltI4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->i32 < op2->i32)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BltI8)
                    {
                        const auto* ir = (ll::BltI8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->i64 < op2->i64)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BltR4)
                    {
                        const auto* ir = (ll::BltR4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        if (left < right && !std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BltR8)
                    {
                        const auto* ir = (ll::BltR8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        if (left < right && !std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BneUnI4)
                    {
                        const auto* ir = (ll::BneUnI4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->u32 != op2->u32)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BneUnI8)
                    {
                        const auto* ir = (ll::BneUnI8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->u64 != op2->u64)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BneUnR4)
                    {
                        const auto* ir = (ll::BneUnR4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        if (left != right || std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BneUnR8)
                    {
                        const auto* ir = (ll::BneUnR8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        if (left != right || std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgeUnI4)
                    {
                        const auto* ir = (ll::BgeUnI4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->u32 >= op2->u32)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgeUnI8)
                    {
                        const auto* ir = (ll::BgeUnI8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->u64 >= op2->u64)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgeUnR4)
                    {
                        const auto* ir = (ll::BgeUnR4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        if (left >= right || std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgeUnR8)
                    {
                        const auto* ir = (ll::BgeUnR8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        if (left >= right || std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgtUnI4)
                    {
                        const auto* ir = (ll::BgtUnI4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->u32 > op2->u32)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgtUnI8)
                    {
                        const auto* ir = (ll::BgtUnI8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->u64 > op2->u64)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgtUnR4)
                    {
                        const auto* ir = (ll::BgtUnR4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        if (left > right || std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BgtUnR8)
                    {
                        const auto* ir = (ll::BgtUnR8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        if (left > right || std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BleUnI4)
                    {
                        const auto* ir = (ll::BleUnI4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->u32 <= op2->u32)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BleUnI8)
                    {
                        const auto* ir = (ll::BleUnI8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->u64 <= op2->u64)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BleUnR4)
                    {
                        const auto* ir = (ll::BleUnR4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        if (left <= right || std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BleUnR8)
                    {
                        const auto* ir = (ll::BleUnR8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        if (left <= right || std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BltUnI4)
                    {
                        const auto* ir = (ll::BltUnI4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->u32 < op2->u32)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BltUnI8)
                    {
                        const auto* ir = (ll::BltUnI8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        if (op1->u64 < op2->u64)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BltUnR4)
                    {
                        const auto* ir = (ll::BltUnR4*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        if (left < right || std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(BltUnR8)
                    {
                        const auto* ir = (ll::BltUnR8*)ip;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        if (left < right || std::isunordered(left, right))
                        {
                            ip = reinterpret_cast<const uint8_t*>(ip + ir->target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(Switch)
                    {
                        const auto* ir = (ll::Switch*)ip;
                        RtStackObject* index_obj = eval_stack_base + ir->index;
                        uint32_t idx = index_obj->u32;
                        if (idx < ir->num_targets)
                        {
                            const int32_t* target_offsets = reinterpret_cast<const int32_t*>(ir + 1);
                            int32_t target_offset = target_offsets[idx];
                            ip = reinterpret_cast<const uint8_t*>(ip + target_offset);
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(reinterpret_cast<const uint8_t*>(ir + 1) + ir->num_targets * sizeof(int32_t));
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN1(LdIndI1Short)
                    {
                        int8_t value = get_ind_stack_value_at<int8_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdIndU1Short)
                    {
                        uint8_t value = get_ind_stack_value_at<uint8_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdIndI2Short)
                    {
                        int16_t value = get_ind_stack_value_at<int16_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdIndU2Short)
                    {
                        uint16_t value = get_ind_stack_value_at<uint16_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdIndI4Short)
                    {
                        int32_t value = get_ind_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdIndI8Short)
                    {
                        int64_t value = get_ind_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StIndI1Short)
                    {
                        int8_t value = get_stack_value_at<int8_t>(eval_stack_base, ir->src);
                        set_ind_stack_value_at<int8_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StIndI2Short)
                    {
                        int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->src);
                        set_ind_stack_value_at<int16_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StIndI4Short)
                    {
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        set_ind_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StIndI8Short)
                    {
                        int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        set_ind_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StIndI8I4Short)
                    {
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        set_ind_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StIndI8U4Short)
                    {
                        uint32_t value = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
                        set_ind_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(AddI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i32 = src1->i32 + src2->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(AddI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i64 = src1->i64 + src2->i64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(AddR4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->f32 = src1->f32 + src2->f32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(AddR8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->f64 = src1->f64 + src2->f64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(SubI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i32 = src1->i32 - src2->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(SubI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i64 = src1->i64 - src2->i64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(SubR4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->f32 = src1->f32 - src2->f32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(SubR8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->f64 = src1->f64 - src2->f64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(MulI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i32 = src1->i32 * src2->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(MulI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i64 = src1->i64 * src2->i64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(MulR4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->f32 = src1->f32 * src2->f32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(MulR8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->f64 = src1->f64 * src2->f64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(DivI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        int32_t dividend = src1->i32;
                        int32_t divisor = src2->i32;
                        if (divisor == 0)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                        }
                        if (divisor == -1 && dividend == INT32_MIN)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Arithmetic);
                        }
                        dst->i32 = dividend / divisor;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(DivI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        int64_t dividend = src1->i64;
                        int64_t divisor = src2->i64;
                        if (divisor == 0)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                        }
                        if (divisor == -1 && dividend == INT64_MIN)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Arithmetic);
                        }
                        dst->i64 = dividend / divisor;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(DivR4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->f32 = src1->f32 / src2->f32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(DivR8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->f64 = src1->f64 / src2->f64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(DivUnI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        uint32_t dividend = src1->u32;
                        uint32_t divisor = src2->u32;
                        if (divisor == 0)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                        }
                        dst->u32 = dividend / divisor;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(DivUnI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        uint64_t dividend = src1->u64;
                        uint64_t divisor = src2->u64;
                        if (divisor == 0)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                        }
                        dst->u64 = dividend / divisor;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(RemI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        int32_t dividend = src1->i32;
                        int32_t divisor = src2->i32;
                        if (divisor == 0)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                        }
                        if (divisor == -1 && dividend == INT32_MIN)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Arithmetic);
                        }
                        else
                        {
                            dst->i32 = dividend % divisor;
                        }
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(RemI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        int64_t dividend = src1->i64;
                        int64_t divisor = src2->i64;
                        if (divisor == 0)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                        }
                        if (divisor == -1 && dividend == INT64_MIN)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Arithmetic);
                        }
                        else
                        {
                            dst->i64 = dividend % divisor;
                        }
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(RemR4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->f32 = std::fmod(src1->f32, src2->f32);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(RemR8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->f64 = std::fmod(src1->f64, src2->f64);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(RemUnI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        uint32_t dividend = src1->u32;
                        uint32_t divisor = src2->u32;
                        if (divisor == 0)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                        }
                        dst->u32 = dividend % divisor;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(RemUnI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        uint64_t dividend = src1->u64;
                        uint64_t divisor = src2->u64;
                        if (divisor == 0)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::DivideByZero);
                        }
                        dst->u64 = dividend % divisor;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(AndI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i32 = src1->i32 & src2->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(AndI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i64 = src1->i64 & src2->i64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(OrI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i32 = src1->i32 | src2->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(OrI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i64 = src1->i64 | src2->i64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(XorI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i32 = src1->i32 ^ src2->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(XorI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i64 = src1->i64 ^ src2->i64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(ShlI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i32 = src1->i32 << src2->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(ShlI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i64 = src1->i64 << src2->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(ShrI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i32 = src1->i32 >> src2->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(ShrI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->i64 = src1->i64 >> src2->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(ShrUnI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->u32 = src1->u32 >> src2->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(ShrUnI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src1 = eval_stack_base + ir->arg1;
                        RtStackObject* src2 = eval_stack_base + ir->arg2;
                        dst->u64 = src1->u64 >> src2->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(NegI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i32 = -src->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(NegI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i64 = -src->i64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(NegR4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->f32 = -src->f32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(NegR8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->f64 = -src->f64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(NotI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i32 = ~src->i32;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(NotI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* src = eval_stack_base + ir->src;
                        dst->i64 = ~src->i64;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CeqI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        dst->i32 = (op1->i32 == op2->i32) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CeqI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        dst->i32 = (op1->i64 == op2->i64) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CeqR4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        dst->i32 = (left == right && !std::isunordered(left, right)) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CeqR8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        dst->i32 = (left == right && !std::isunordered(left, right)) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CgtI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        dst->i32 = (op1->i32 > op2->i32) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CgtI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        dst->i32 = (op1->i64 > op2->i64) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CgtR4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        dst->i32 = (left > right && !std::isunordered(left, right)) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CgtR8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        dst->i32 = (left > right && !std::isunordered(left, right)) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CgtUnI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        dst->i32 = (op1->u32 > op2->u32) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CgtUnI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        dst->i32 = (op1->u64 > op2->u64) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CgtUnR4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        dst->i32 = (left > right || std::isunordered(left, right)) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CgtUnR8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        dst->i32 = (left > right || std::isunordered(left, right)) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CltI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        dst->i32 = (op1->i32 < op2->i32) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CltI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        dst->i32 = (op1->i64 < op2->i64) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CltR4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        dst->i32 = (left < right && !std::isunordered(left, right)) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CltR8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        dst->i32 = (left < right && !std::isunordered(left, right)) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CltUnI4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        dst->i32 = (op1->u32 < op2->u32) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CltUnI8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        dst->i32 = (op1->u64 < op2->u64) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CltUnR4)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        float left = op1->f32;
                        float right = op2->f32;
                        dst->i32 = (left < right || std::isunordered(left, right)) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CltUnR8)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* op1 = eval_stack_base + ir->arg1;
                        RtStackObject* op2 = eval_stack_base + ir->arg2;
                        double left = op1->f64;
                        double right = op2->f64;
                        dst->i32 = (left < right || std::isunordered(left, right)) ? 1 : 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(InitObjI1)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->addr;
                        int8_t* addr = reinterpret_cast<int8_t*>(addr_obj->ptr);
                        *addr = 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(InitObjI2)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->addr;
                        int16_t* addr = reinterpret_cast<int16_t*>(addr_obj->ptr);
                        *addr = 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(InitObjI4)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->addr;
                        int32_t* addr = reinterpret_cast<int32_t*>(addr_obj->ptr);
                        *addr = 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(InitObjI8)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->addr;
                        int64_t* addr = reinterpret_cast<int64_t*>(addr_obj->ptr);
                        *addr = 0;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(InitObjAny)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->addr;
                        uint8_t* addr = reinterpret_cast<uint8_t*>(addr_obj->ptr);
                        std::memset(addr, 0, ir->size);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CpObjI1)
                    {
                        RtStackObject* dst_obj = eval_stack_base + ir->dst;
                        RtStackObject* src_obj = eval_stack_base + ir->src;
                        int8_t* dst_addr = reinterpret_cast<int8_t*>(dst_obj->ptr);
                        const int8_t* src_addr = reinterpret_cast<const int8_t*>(src_obj->ptr);
                        *dst_addr = *src_addr;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CpObjI2)
                    {
                        RtStackObject* dst_obj = eval_stack_base + ir->dst;
                        RtStackObject* src_obj = eval_stack_base + ir->src;
                        int16_t* dst_addr = reinterpret_cast<int16_t*>(dst_obj->ptr);
                        const int16_t* src_addr = reinterpret_cast<const int16_t*>(src_obj->ptr);
                        *dst_addr = *src_addr;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CpObjI4)
                    {
                        RtStackObject* dst_obj = eval_stack_base + ir->dst;
                        RtStackObject* src_obj = eval_stack_base + ir->src;
                        int32_t* dst_addr = reinterpret_cast<int32_t*>(dst_obj->ptr);
                        const int32_t* src_addr = reinterpret_cast<const int32_t*>(src_obj->ptr);
                        *dst_addr = *src_addr;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CpObjI8)
                    {
                        RtStackObject* dst_obj = eval_stack_base + ir->dst;
                        RtStackObject* src_obj = eval_stack_base + ir->src;
                        int64_t* dst_addr = reinterpret_cast<int64_t*>(dst_obj->ptr);
                        const int64_t* src_addr = reinterpret_cast<const int64_t*>(src_obj->ptr);
                        *dst_addr = *src_addr;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CpObjAny)
                    {
                        RtStackObject* dst_obj = eval_stack_base + ir->dst;
                        RtStackObject* src_obj = eval_stack_base + ir->src;
                        std::memcpy(dst_obj->ptr, src_obj->cptr, ir->size);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdObjAny)
                    {
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        RtStackObject* addr_obj = eval_stack_base + ir->addr;
                        std::memcpy(dst, addr_obj->cptr, ir->size);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StObjAny)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->addr;
                        RtStackObject* src = eval_stack_base + ir->src;
                        std::memcpy(addr_obj->ptr, src, ir->size);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CastClass)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (obj)
                        {
                            metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                            if (!vm::Class::is_assignable_from(obj->klass, to_class))
                            {
                                RAISE_RUNTIME_ERROR(RtErr::InvalidCast);
                            }
                        }
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(IsInst)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        vm::RtObject* result;
                        if (obj)
                        {
                            metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                            result = vm::Class::is_assignable_from(obj->klass, to_class) ? obj : nullptr;
                        }
                        else
                        {
                            result = nullptr;
                        }
                        set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(Box)
                    {
                        RtStackObject* src = eval_stack_base + ir->src;
                        metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                        HANDLE_RAISE_RUNTIME_ERROR(vm::RtObject*, boxed_obj, vm::Object::box_object(to_class, src));
                        set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, boxed_obj);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(Unbox)
                    {
                        vm::RtObject* boxed_obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!boxed_obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                        HANDLE_RAISE_RUNTIME_ERROR(const void*, unbox_addr, vm::Object::unbox_ex(boxed_obj, to_class));
                        set_stack_value_at(eval_stack_base, ir->dst, unbox_addr);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(UnboxAny)
                    {
                        vm::RtObject* boxed_obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        metadata::RtClass* to_class = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        HANDLE_RAISE_RUNTIME_ERROR_VOID(vm::Object::unbox_any(boxed_obj, to_class, dst, true));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(NewArr)
                    {
                        int32_t length = get_stack_value_at<int32_t>(eval_stack_base, ir->length);
                        metadata::RtClass* element_class = get_resolved_data<metadata::RtClass>(imi, ir->arr_klass_idx);
                        HANDLE_RAISE_RUNTIME_ERROR(vm::RtArray*, new_array, vm::Array::new_szarray_from_array_klass(element_class, length));
                        set_stack_value_at<vm::RtArray*>(eval_stack_base, ir->dst, new_array);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdLen)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        set_stack_value_at(eval_stack_base, ir->dst, vm::Array::get_array_length(array));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(Ldelema)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        const metadata::RtClass* arr_klass = array->klass;
                        const metadata::RtClass* element_klass = vm::Class::get_array_element_class(arr_klass);
                        const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->ele_klass_idx);
                        if (!vm::Class::is_pointer_element_compatible_with(element_klass, check_klass))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
                        }
                        const void* element_addr = vm::Array::get_array_element_address_as_ptr_void(array, index);
                        set_stack_value_at(eval_stack_base, ir->dst, element_addr);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdelemI1)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 1);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        int8_t value = vm::Array::get_array_data_at<int8_t>(array, index);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdelemU1)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 1);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        uint8_t value = vm::Array::get_array_data_at<uint8_t>(array, index);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdelemI2)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 2);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        int16_t value = vm::Array::get_array_data_at<int16_t>(array, index);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdelemU2)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 2);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        uint16_t value = vm::Array::get_array_data_at<uint16_t>(array, index);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdelemI4)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 4);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        int32_t value = vm::Array::get_array_data_at<int32_t>(array, index);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdelemI8)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 8);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        int64_t value = vm::Array::get_array_data_at<int64_t>(array, index);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdelemI)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == PTR_SIZE);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        intptr_t value = vm::Array::get_array_data_at<intptr_t>(array, index);
                        set_stack_value_at<intptr_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdelemR4)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 4);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        float value = vm::Array::get_array_data_at<float>(array, index);
                        set_stack_value_at<float>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdelemR8)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 8);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        double value = vm::Array::get_array_data_at<double>(array, index);
                        set_stack_value_at<double>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdelemRef)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == PTR_SIZE);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        vm::RtObject* value = vm::Array::get_array_data_at<vm::RtObject*>(array, index);
                        set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdelemAnyRef)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        vm::RtObject* value = vm::Array::get_array_data_at<vm::RtObject*>(array, index);
                        const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->ele_klass_idx);
                        if (value && !vm::Class::is_assignable_from(value->klass, check_klass))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
                        }
                        set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdelemAnyVal)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        // const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(array);
                        // const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->ele_klass_idx);
                        // if (!vm::Class::is_pointer_element_compatible_with(ele_klass, check_klass))
                        // {
                        //     RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
                        // }
                        assert(ir->ele_size == vm::Array::get_array_element_size(array));
                        const void* src_addr = vm::Array::get_array_element_address_with_size_as_ptr_void(array, index, ir->ele_size);
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        std::memcpy(dst, src_addr, ir->ele_size);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StelemI1)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 1);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        int8_t value = get_stack_value_at<int8_t>(eval_stack_base, ir->value);
                        vm::Array::set_array_data_at<int8_t>(array, index, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StelemI2)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 2);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->value);
                        vm::Array::set_array_data_at<int16_t>(array, index, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StelemI4)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 4);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
                        vm::Array::set_array_data_at<int32_t>(array, index, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StelemI8)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 8);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
                        vm::Array::set_array_data_at<int64_t>(array, index, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StelemI)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == PTR_SIZE);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        intptr_t value = get_stack_value_at<intptr_t>(eval_stack_base, ir->value);
                        vm::Array::set_array_data_at<intptr_t>(array, index, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StelemR4)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 4);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        float value = get_stack_value_at<float>(eval_stack_base, ir->value);
                        vm::Array::set_array_data_at<float>(array, index, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StelemR8)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == 8);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        double value = get_stack_value_at<double>(eval_stack_base, ir->value);
                        vm::Array::set_array_data_at<double>(array, index, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StelemRef)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        assert(vm::Array::get_array_element_size(array) == PTR_SIZE);
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        vm::RtObject* value = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->value);
                        const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(array);
                        if (value && !vm::Class::is_assignable_from(value->klass, ele_klass))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
                        }
                        vm::Array::set_array_data_at<vm::RtObject*>(array, index, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StelemAnyRef)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        vm::RtObject* value = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->value);
                        const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(array);
                        if (value && !vm::Class::is_assignable_from(value->klass, ele_klass))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
                        }
                        vm::Array::set_array_data_at<vm::RtObject*>(array, index, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StelemAnyVal)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        const metadata::RtClass* ele_klass = vm::Array::get_array_element_class(array);
                        const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->ele_klass_idx);
                        if (!vm::Class::is_pointer_element_compatible_with(ele_klass, check_klass))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::ArrayTypeMismatch);
                        }
                        assert(ir->ele_size == vm::Array::get_array_element_size(array));
                        RtStackObject* src = eval_stack_base + ir->value;
                        void* dst_addr = const_cast<void*>(vm::Array::get_array_element_address_with_size_as_ptr_void(array, index, ir->ele_size));
                        std::memcpy(dst_addr, src, ir->ele_size);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(MkRefAny)
                    {
                        const metadata::RtClass* klass = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                        const void* src_addr = get_stack_value_at<const void*>(eval_stack_base, ir->addr);
                        vm::RtTypedReference* dst = get_ptr_stack_value_at<vm::RtTypedReference>(eval_stack_base, ir->dst);
                        dst->type_handle = klass->by_val;
                        dst->value = src_addr;
                        dst->klass = klass;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(RefAnyVal)
                    {
                        vm::RtTypedReference* src = get_ptr_stack_value_at<vm::RtTypedReference>(eval_stack_base, ir->src);
                        const metadata::RtClass* check_klass = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                        if (src->klass != check_klass)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::InvalidCast);
                        }
                        set_stack_value_at<const void*>(eval_stack_base, ir->dst, src->value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(RefAnyType)
                    {
                        vm::RtTypedReference* src = get_ptr_stack_value_at<vm::RtTypedReference>(eval_stack_base, ir->src);
                        set_stack_value_at<const metadata::RtTypeSig*>(eval_stack_base, ir->dst, src->type_handle);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdToken)
                    {
                        const void* token_data = get_resolved_data<void>(imi, ir->token_idx);
                        set_stack_value_at<const void*>(eval_stack_base, ir->dst, token_data);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CkfiniteR4)
                    {
                        float value = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (!std::isfinite(value))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(CkfiniteR8)
                    {
                        double value = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (!std::isfinite(value))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LocAlloc)
                    {
                        uint32_t size = get_stack_value_at<uint32_t>(eval_stack_base, ir->size);
                        void* data;
                        if (size > 0)
                        {
                            data = imi->init_locals ? alloc::GeneralAllocation::malloc_zeroed(size) : alloc::GeneralAllocation::malloc(size);
                        }
                        else
                        {
                            data = nullptr;
                        }
                        set_stack_value_at<void*>(eval_stack_base, ir->dst, data);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(Ldftn)
                    {
                        metadata::RtMethodInfo* method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        set_stack_value_at(eval_stack_base, ir->dst, reinterpret_cast<void*>(method));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(Ldvirtftn)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const metadata::RtMethodInfo* method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        assert(method->slot < method->parent->vtable_count);
                        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, actual_method, vm::Method::get_virtual_method_impl(obj, method));
                        set_stack_value_at(eval_stack_base, ir->dst, actual_method);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdfldI1)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const int8_t* field_addr = reinterpret_cast<const int8_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                        int8_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdfldU1)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                        uint8_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdfldI2)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const int16_t* field_addr = reinterpret_cast<const int16_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                        int16_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdfldU2)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint16_t* field_addr = reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                        uint16_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdfldI4)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const int32_t* field_addr = reinterpret_cast<const int32_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                        int32_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdfldI8)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const int64_t* field_addr = reinterpret_cast<const int64_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                        int64_t value = *field_addr;
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdfldAny)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        std::memcpy(dst, field_addr, ir->size);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdvfldI1)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->obj;
                        const int8_t* field_addr = reinterpret_cast<const int8_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
                        int8_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdvfldU1)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->obj;
                        const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset;
                        uint8_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdvfldI2)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->obj;
                        const int16_t* field_addr = reinterpret_cast<const int16_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
                        int16_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdvfldU2)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->obj;
                        const uint16_t* field_addr = reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
                        uint16_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdvfldI4)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->obj;
                        const int32_t* field_addr = reinterpret_cast<const int32_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
                        int32_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdvfldI8)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->obj;
                        const int64_t* field_addr = reinterpret_cast<const int64_t*>(reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset);
                        int64_t value = *field_addr;
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdvfldAny)
                    {
                        RtStackObject* addr_obj = eval_stack_base + ir->obj;
                        const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(addr_obj) + ir->offset;
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        std::memmove(dst, field_addr, ir->size);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(Ldflda)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                        set_stack_value_at(eval_stack_base, ir->dst, field_addr);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StfldI1)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int8_t value = get_stack_value_at<int8_t>(eval_stack_base, ir->value);
                        int8_t* field_addr = reinterpret_cast<int8_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
                        *field_addr = value;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StfldI2)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->value);
                        int16_t* field_addr = reinterpret_cast<int16_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
                        *field_addr = value;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StfldI4)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
                        int32_t* field_addr = reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
                        *field_addr = value;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StfldI8)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
                        int64_t* field_addr = reinterpret_cast<int64_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
                        *field_addr = value;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StfldAny)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        RtStackObject* src = eval_stack_base + ir->value;
                        uint8_t* field_addr = reinterpret_cast<uint8_t*>(obj) + ir->offset;
                        std::memcpy(field_addr, src, ir->size);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdsfldI1)
                    {
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                        const int8_t* field_addr = get_static_field_address<int8_t>(field);
                        int8_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdsfldU1)
                    {
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                        const uint8_t* field_addr = get_static_field_address<uint8_t>(field);
                        uint8_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdsfldI2)
                    {
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                        const int16_t* field_addr = get_static_field_address<int16_t>(field);
                        int16_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdsfldU2)
                    {
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                        const uint16_t* field_addr = get_static_field_address<uint16_t>(field);
                        uint16_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdsfldI4)
                    {
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                        const int32_t* field_addr = get_static_field_address<int32_t>(field);
                        int32_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdsfldI8)
                    {
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                        const int64_t* field_addr = get_static_field_address<int64_t>(field);
                        int64_t value = *field_addr;
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdsfldAny)
                    {
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        TRY_RUN_CLASS_STATIC_CCTOR(field->parent);
                        const uint8_t* field_addr = get_static_field_address<uint8_t>(field);
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        std::memcpy(dst, field_addr, ir->size);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(Ldsflda)
                    {
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        metadata::RtClass* klass = field->parent;
                        TRY_RUN_CLASS_STATIC_CCTOR(klass);
                        const void* field_addr = get_static_field_address<void>(field);
                        set_stack_value_at(eval_stack_base, ir->dst, field_addr);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(LdsfldRvaData)
                    {
                        const void* rva_data = get_resolved_data<void>(imi, ir->data);
                        set_stack_value_at(eval_stack_base, ir->dst, rva_data);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StsfldI1)
                    {
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        metadata::RtClass* klass = field->parent;
                        TRY_RUN_CLASS_STATIC_CCTOR(klass);
                        int8_t* field_addr = get_static_field_address<int8_t>(field);
                        *field_addr = static_cast<int8_t>(value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StsfldI2)
                    {
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        metadata::RtClass* klass = field->parent;
                        TRY_RUN_CLASS_STATIC_CCTOR(klass);
                        int16_t* field_addr = get_static_field_address<int16_t>(field);
                        *field_addr = static_cast<int16_t>(value);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StsfldI4)
                    {
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        metadata::RtClass* klass = field->parent;
                        TRY_RUN_CLASS_STATIC_CCTOR(klass);
                        int32_t* field_addr = get_static_field_address<int32_t>(field);
                        *field_addr = value;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StsfldI8)
                    {
                        int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        metadata::RtClass* klass = field->parent;
                        TRY_RUN_CLASS_STATIC_CCTOR(klass);
                        int64_t* field_addr = get_static_field_address<int64_t>(field);
                        *field_addr = value;
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(StsfldAny)
                    {
                        RtStackObject* src = eval_stack_base + ir->value;
                        const metadata::RtFieldInfo* field = get_resolved_data<metadata::RtFieldInfo>(imi, ir->field_idx);
                        metadata::RtClass* klass = field->parent;
                        TRY_RUN_CLASS_STATIC_CCTOR(klass);
                        uint8_t* field_addr = get_static_field_address<uint8_t>(field);
                        std::memcpy(field_addr, src, ir->size);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(RetVoid)
                    {
                        LEAVE_FRAME();
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(RetI4)
                    {
                        eval_stack_base->i32 = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        LEAVE_FRAME();
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(RetI8)
                    {
                        eval_stack_base->i64 = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        LEAVE_FRAME();
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(RetAny)
                    {
                        std::memcpy(eval_stack_base, eval_stack_base + ir->src, ir->size * sizeof(RtStackObject));
                        LEAVE_FRAME();
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN_LITE1(CallInterp)
                    {
                        const auto* ir = reinterpret_cast<const ll::CallInterp*>(ip);
                        const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        if (vm::Method::is_static(target_method))
                        {
                            TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                        }
                        const uint8_t* next_ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        ENTER_INTERP_FRAME(target_method, ir->frame_base, next_ip);
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(CallVirtInterp)
                    {
                        const auto* ir = reinterpret_cast<const ll::CallVirtInterp*>(ip);
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->frame_base);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const metadata::RtMethodInfo* original_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, actual_method,
                                                                vm::Method::get_virtual_method_impl(obj, original_method));
                        if (actual_method->invoker_type == metadata::RtInvokerType::Interpreter)
                        {
                            if (vm::Class::is_value_type(actual_method->parent))
                            {
                                set_stack_value_at(eval_stack_base, ir->frame_base, obj + 1);
                            }
                            ENTER_INTERP_FRAME(actual_method, ir->frame_base, reinterpret_cast<const uint8_t*>(ir + 1));
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                            RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                            HANDLE_RAISE_RUNTIME_ERROR_VOID(CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(actual_method->invoke_method_ptr)(
                                actual_method->virtual_method_ptr, actual_method, frame_base, frame_base));
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(CallInternalCall)
                    {
                        const auto* ir = reinterpret_cast<const ll::CallInternalCall*>(ip);
                        const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        if (vm::Method::is_static(target_method))
                        {
                            TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                        }
                        ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                        HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(CallIntrinsic)
                    {
                        const auto* ir = reinterpret_cast<const ll::CallIntrinsic*>(ip);
                        const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        if (vm::Method::is_static(target_method))
                        {
                            TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                        }
                        ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                        HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(CallPInvoke)
                    {
                        const auto* ir = reinterpret_cast<const ll::CallPInvoke*>(ip);
                        const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        if (vm::Method::is_static(target_method))
                        {
                            TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                        }
                        ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                        HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(CallAot)
                    {
                        const auto* ir = reinterpret_cast<const ll::CallAot*>(ip);
                        const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        if (vm::Method::is_static(target_method))
                        {
                            TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                        }
                        ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                        HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(CallRuntimeImplemented)
                    {
                        const auto* ir = reinterpret_cast<const ll::CallRuntimeImplemented*>(ip);
                        const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        if (vm::Method::is_static(target_method))
                        {
                            TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                        }
                        ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                        HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(CalliInterp)
                    {
                        const auto* ir = reinterpret_cast<const ll::CalliInterp*>(ip);
                        const metadata::RtMethodSig* method_sig = get_resolved_data<const metadata::RtMethodSig>(imi, ir->method_sig_idx);
                        const uint8_t* next_ip = reinterpret_cast<const uint8_t*>(ir + 1);
                        const metadata::RtMethodInfo* target_method = get_stack_value_at<const metadata::RtMethodInfo*>(eval_stack_base, ir->method_idx);
                        if (target_method->parameter_count != method_sig->params.size())
                        {
                            RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
                        }

                        if (target_method->invoker_type == metadata::RtInvokerType::Interpreter)
                        {
                            ENTER_INTERP_FRAME(target_method, ir->frame_base, reinterpret_cast<const uint8_t*>(ir + 1));
                        }
                        else
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                            RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                            HANDLE_RAISE_RUNTIME_ERROR_VOID(target_method->invoke_method_ptr(target_method->method_ptr, target_method, frame_base, frame_base));
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN1(BoxRefInplace)
                    {
                        const void* src_ptr = get_stack_value_at<const void*>(eval_stack_base, ir->src);
                        metadata::RtClass* to_klass = get_resolved_data<metadata::RtClass>(imi, ir->klass_idx);
                        HANDLE_RAISE_RUNTIME_ERROR(vm::RtObject*, boxed_obj, vm::Object::box_object(to_klass, src_ptr));
                        set_stack_value_at<vm::RtObject*>(eval_stack_base, ir->dst, boxed_obj);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN_LITE1(NewObjInterp)
                    {
                        const auto* ir = reinterpret_cast<const ll::NewObjInterp*>(ip);
                        const metadata::RtMethodInfo* ctor = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        const metadata::RtClass* klass = ctor->parent;
                        TRY_RUN_CLASS_STATIC_CCTOR(klass);
                        HANDLE_RAISE_RUNTIME_ERROR(vm::RtObject*, obj, vm::Object::new_object(klass));
                        RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                        std::memmove(frame_base + 1, frame_base, static_cast<size_t>(ir->total_params_stack_object_size) * sizeof(RtStackObject));
                        frame_base->obj = obj;
                        ENTER_INTERP_FRAME(ctor, ir->frame_base, reinterpret_cast<const uint8_t*>(ir + 1));
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(NewValueTypeInterp)
                    {
                        const auto* ir = reinterpret_cast<const ll::NewValueTypeInterp*>(ip);
                        const metadata::RtMethodInfo* ctor = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        const metadata::RtClass* klass = ctor->parent;
                        RtStackObject* original_frame_base = eval_stack_base + ir->frame_base;
                        const size_t value_stack_objects = InterpDefs::get_stack_object_size_by_byte_size(klass->instance_size_without_header);
                        RtStackObject* final_frame_base = original_frame_base + value_stack_objects;
                        std::memmove(final_frame_base + 1, original_frame_base,
                                     static_cast<size_t>(ir->total_params_stack_object_size) * sizeof(RtStackObject));
                        final_frame_base->ptr = original_frame_base;
                        std::memset(original_frame_base, 0, value_stack_objects * sizeof(RtStackObject));
                        ENTER_INTERP_FRAME(ctor, ir->frame_base + value_stack_objects, reinterpret_cast<const uint8_t*>(ir + 1));
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN1(NewObjInternalCall)
                    {
                        const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                        vm::InternalCallInvoker invoker = vm::InternalCalls::get_internal_call_invoker_by_id_unchecked(ir->invoker_idx);
                        RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                        HANDLE_RAISE_RUNTIME_ERROR_VOID(invoker(target_method->method_ptr, target_method, frame_base, frame_base));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(NewObjIntrinsic)
                    {
                        const metadata::RtMethodInfo* target_method = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        TRY_RUN_CLASS_STATIC_CCTOR(target_method->parent);
                        vm::InternalCallInvoker invoker = vm::Intrinsics::get_intrinsic_invoker_by_id_unchecked(ir->invoker_idx);
                        RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                        HANDLE_RAISE_RUNTIME_ERROR_VOID(invoker(target_method->method_ptr, target_method, frame_base, frame_base));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(NewObjAot)
                    {
                        const metadata::RtMethodInfo* ctor = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        const metadata::RtClass* klass = ctor->parent;
                        TRY_RUN_CLASS_STATIC_CCTOR(klass);
                        HANDLE_RAISE_RUNTIME_ERROR(vm::RtObject*, obj, vm::Object::new_object(klass));
                        RtStackObject* frame_base = eval_stack_base + ir->frame_base;
                        std::memmove(frame_base + 1, frame_base, static_cast<size_t>(ir->total_params_stack_object_size) * sizeof(RtStackObject));
                        frame_base->obj = obj;
                        auto invoker = CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(ctor->invoke_method_ptr);
                        HANDLE_RAISE_RUNTIME_ERROR_VOID(invoker(ctor->method_ptr, ctor, frame_base, frame_base));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(NewValueTypeAot)
                    {
                        const metadata::RtMethodInfo* ctor = get_resolved_data<metadata::RtMethodInfo>(imi, ir->method_idx);
                        const metadata::RtClass* klass = ctor->parent;
                        RtStackObject* original_frame_base = eval_stack_base + ir->frame_base;
                        const size_t value_stack_objects = InterpDefs::get_stack_object_size_by_byte_size(klass->instance_size_without_header);
                        RtStackObject* final_frame_base = original_frame_base + value_stack_objects;
                        std::memmove(final_frame_base + 1, original_frame_base,
                                     static_cast<size_t>(ir->total_params_stack_object_size) * sizeof(RtStackObject));
                        final_frame_base->ptr = original_frame_base;
                        std::memset(original_frame_base, 0, value_stack_objects * sizeof(RtStackObject));
                        auto invoker = CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(ctor->invoke_method_ptr);
                        HANDLE_RAISE_RUNTIME_ERROR_VOID(invoker(ctor->method_ptr, ctor, final_frame_base, final_frame_base));
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(Throw)
                    {
                        vm::RtException* ex = get_stack_value_at<vm::RtException*>(eval_stack_base, ir->ex);
                        if (!ex)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        if (!vm::Class::is_exception_sub_class(ex->klass))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::InvalidCast);
                        }
                        RAISE_RUNTIME_EXCEPTION(ex);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN1(Rethrow)
                    {
                        vm::RtException* ex = find_exception_in_enclosing_throw_flow(frame, static_cast<uint32_t>(ip - imi->codes));
                        if (!ex)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
                        }
                        RAISE_RUNTIME_EXCEPTION(ex);
                    }
                    LEANCLR_CASE_END1()
                    LEANCLR_CASE_BEGIN_LITE1(LeaveTryWithFinally)
                    {
                        const auto* ir = reinterpret_cast<const ll::LeaveTryWithFinally*>(ip);
                        assert(ir->finally_clauses_count > 0);
                        assert(ir->first_finally_clause_index < imi->exception_clause_count);
                        const uint8_t* target_ip = ip + ir->target_offset;
                        const RtInterpExceptionClause* finally_clause = &imi->exception_clauses[ir->first_finally_clause_index];
                        push_leave_flow(frame, ip, target_ip, finally_clause, ir->first_finally_clause_index + 1, ir->finally_clauses_count - 1);
                        ip = imi->codes + finally_clause->handler_begin_offset;
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(LeaveCatchWithFinally)
                    {
                        const auto* ir = reinterpret_cast<const ll::LeaveCatchWithFinally*>(ip);
                        assert(ir->finally_clauses_count > 0);
                        assert(ir->first_finally_clause_index < imi->exception_clause_count);
                        vm::RtException* ex = get_exception_in_last_throw_flow(frame, static_cast<uint32_t>(ip - imi->codes));
                        pop_throw_flow(ex, frame);
                        const uint8_t* target_ip = ip + ir->target_offset;
                        const RtInterpExceptionClause* finally_clause = &imi->exception_clauses[ir->first_finally_clause_index];
                        push_leave_flow(frame, ip, target_ip, finally_clause, ir->first_finally_clause_index + 1, ir->finally_clauses_count - 1);
                        ip = imi->codes + finally_clause->handler_begin_offset;
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(LeaveCatchWithoutFinally)
                    {
                        const auto* ir = reinterpret_cast<const ll::LeaveCatchWithoutFinally*>(ip);
                        vm::RtException* ex = get_exception_in_last_throw_flow(frame, static_cast<uint32_t>(ip - imi->codes));
                        pop_throw_flow(ex, frame);
                        ip += ir->target_offset;
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(EndFilter)
                    {
                        const auto* ir = reinterpret_cast<const ll::EndFilter*>(ip);
                        int32_t cond = get_stack_value_at<int32_t>(eval_stack_base, ir->cond);
                        if (cond)
                        {
                            ip = reinterpret_cast<const uint8_t*>(ir + 1);
                            setup_filter_handler(imi, frame, ip);
                            vm::RtException* ex = get_exception_in_last_throw_flow(frame, static_cast<uint32_t>(ip - imi->codes));
                            set_stack_value_at<vm::RtObject*>(eval_stack_base, imi->total_arg_and_local_stack_object_size, ex);
                        }
                        else
                        {
                            goto unwind_exception_handler;
                        }
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(EndFinally)
                    {
                        goto unwind_exception_handler;
                    }
                    LEANCLR_CASE_END_LITE1()
                    LEANCLR_CASE_BEGIN_LITE1(EndFault)
                    {
                        goto unwind_exception_handler;
                    }
                    LEANCLR_CASE_END_LITE1()
#if !LEANCLR_USE_COMPUTED_GOTO_DISPATCHER
                default:
                {
                    assert(false && "Invalid opcode");
                    RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
                }
#endif
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(Prefix2)
            {
                LEANCLR_SWITCH2()
                {
                    LEANCLR_CASE_BEGIN2(LdIndI1)
                    {
                        int8_t value = get_ind_stack_value_at<int8_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(LdIndU1)
                    {
                        uint8_t value = get_ind_stack_value_at<uint8_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(LdIndI2)
                    {
                        int16_t value = get_ind_stack_value_at<int16_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(LdIndU2)
                    {
                        uint16_t value = get_ind_stack_value_at<uint16_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(LdIndI4)
                    {
                        int32_t value = get_ind_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(LdIndI8)
                    {
                        int64_t value = get_ind_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(StIndI1)
                    {
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        set_ind_stack_value_at<int8_t>(eval_stack_base, ir->dst, static_cast<int8_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(StIndI2)
                    {
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        set_ind_stack_value_at<int16_t>(eval_stack_base, ir->dst, static_cast<int16_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(StIndI4)
                    {
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        set_ind_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(StIndI8)
                    {
                        int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        set_ind_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(StIndI8I4)
                    {
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        set_ind_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(StIndI8U4)
                    {
                        uint32_t value = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
                        set_ind_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI1I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        int8_t value = static_cast<int8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI1I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        int8_t value = static_cast<int8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI1R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        int32_t value = cast_float_to_small_int<float, int8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI1R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        int32_t value = cast_float_to_small_int<double, int8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU1I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        uint8_t value = static_cast<uint8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU1I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        uint8_t value = static_cast<uint8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU1R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        int32_t value = cast_float_to_small_int<float, uint8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU1R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        int32_t value = cast_float_to_small_int<double, uint8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI2I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        int16_t value = static_cast<int16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI2I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        int16_t value = static_cast<int16_t>(src);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI2R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        int32_t value = cast_float_to_small_int<float, int16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI2R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        int32_t value = cast_float_to_small_int<double, int16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU2I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        uint16_t value = static_cast<uint16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU2I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        uint16_t value = static_cast<uint16_t>(src);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(value));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU2R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        int32_t value = cast_float_to_small_int<float, uint16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU2R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        int32_t value = cast_float_to_small_int<double, uint16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI4I8)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, src);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI4R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        int32_t value = cast_float_to_i32<float, int32_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI4R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        int32_t value = cast_float_to_i32<double, int32_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU4I8)
                    {
                        uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
                        set_stack_value_at<uint32_t>(eval_stack_base, ir->dst, src);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU4R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        int32_t value = cast_float_to_i32<float, uint32_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU4R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        int32_t value = cast_float_to_i32<double, uint32_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI8I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(src));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI8U4)
                    {
                        uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(src));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI8R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        int64_t value = cast_float_to_i64<float, int64_t>(src);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvI8R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        int64_t value = cast_float_to_i64<double, int64_t>(src);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU8I4)
                    {
                        uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
                        set_stack_value_at<uint64_t>(eval_stack_base, ir->dst, static_cast<uint64_t>(src));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU8R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        int64_t value = cast_float_to_i64<float, uint64_t>(src);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvU8R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        int64_t value = cast_float_to_i64<double, uint64_t>(src);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvR4I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        set_stack_value_at<float>(eval_stack_base, ir->dst, static_cast<float>(src));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvR4I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        set_stack_value_at<float>(eval_stack_base, ir->dst, static_cast<float>(src));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvR4R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        set_stack_value_at<float>(eval_stack_base, ir->dst, static_cast<float>(src));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvR8I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        set_stack_value_at<double>(eval_stack_base, ir->dst, static_cast<double>(src));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvR8I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        set_stack_value_at<double>(eval_stack_base, ir->dst, static_cast<double>(src));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(ConvR8R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        set_stack_value_at<double>(eval_stack_base, ir->dst, static_cast<double>(src));
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(LdelemaReadOnly)
                    {
                        vm::RtArray* array = get_stack_value_at<vm::RtArray*>(eval_stack_base, ir->arr);
                        if (!array)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int32_t index = get_stack_value_at<int32_t>(eval_stack_base, ir->index);
                        if (vm::Array::is_out_of_range(array, index))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::IndexOutOfRange);
                        }
                        const void* element_addr = vm::Array::get_array_element_address_as_ptr_void(array, index);
                        set_stack_value_at(eval_stack_base, ir->dst, element_addr);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(InitBlk)
                    {
                        void* dst = get_stack_value_at<void*>(eval_stack_base, ir->addr);
                        uint8_t value = get_stack_value_at<uint8_t>(eval_stack_base, ir->value);
                        uint32_t size = get_stack_value_at<uint32_t>(eval_stack_base, ir->size);
                        std::memset(dst, value, size);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(CpBlk)
                    {
                        void* dst = get_stack_value_at<void*>(eval_stack_base, ir->dst);
                        const void* src = get_stack_value_at<const void*>(eval_stack_base, ir->src);
                        uint32_t size = get_stack_value_at<uint32_t>(eval_stack_base, ir->size);
                        std::memcpy(dst, src, size);
                    }
                    LEANCLR_CASE_END2()
                    LEANCLR_CASE_BEGIN2(GetEnumLongHashCode)
                    {
                        int64_t value = get_ind_stack_value_at<int64_t>(eval_stack_base, ir->value_ptr);
                        int32_t hash = vm::Enum::get_enum_long_hash_code(value);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, hash);
                    }
                    LEANCLR_CASE_END2()
#if !LEANCLR_USE_COMPUTED_GOTO_DISPATCHER
                default:
                {
                    assert(false && "Invalid opcode");
                    RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
                }
#endif
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(Prefix3)
            {
                LEANCLR_SWITCH3()
                {
                    LEANCLR_CASE_BEGIN3(LdIndI2Unaligned)
                    {
                        const uint8_t* addr = get_stack_value_at<const uint8_t*>(eval_stack_base, ir->src);
                        int16_t value = utils::MemOp::read_i16_may_unaligned(addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdIndU2Unaligned)
                    {
                        const uint8_t* addr = get_stack_value_at<const uint8_t*>(eval_stack_base, ir->src);
                        uint16_t value = utils::MemOp::read_u16_may_unaligned(addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdIndI4Unaligned)
                    {
                        const uint8_t* addr = get_stack_value_at<const uint8_t*>(eval_stack_base, ir->src);
                        int32_t value = utils::MemOp::read_i32_may_unaligned(addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdIndI8Unaligned)
                    {
                        const uint8_t* addr = get_stack_value_at<const uint8_t*>(eval_stack_base, ir->src);
                        int64_t value = utils::MemOp::read_i64_may_unaligned(addr);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(StIndI2Unaligned)
                    {
                        uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->dst);
                        int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->src);
                        utils::MemOp::write_i16_may_unaligned(addr, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(StIndI4Unaligned)
                    {
                        uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->dst);
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        utils::MemOp::write_i32_may_unaligned(addr, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(StIndI8Unaligned)
                    {
                        uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->dst);
                        int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        utils::MemOp::write_i64_may_unaligned(addr, value);
                    }
                    LEANCLR_CASE_END3()

                    LEANCLR_CASE_BEGIN3(StIndI8I4Unaligned)
                    {
                        uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->dst);
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        utils::MemOp::write_i64_may_unaligned(addr, static_cast<int64_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(StIndI8U4Unaligned)
                    {
                        uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->dst);
                        uint32_t value = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
                        utils::MemOp::write_i64_may_unaligned(addr, static_cast<int64_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(AddOvfI4)
                    {
                        int32_t left = get_stack_value_at<int32_t>(eval_stack_base, ir->arg1);
                        int32_t right = get_stack_value_at<int32_t>(eval_stack_base, ir->arg2);
                        int32_t result;
                        if (CHECK_ADD_OVERFLOW_I32(left, right, &result))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(AddOvfI8)
                    {
                        int64_t left = get_stack_value_at<int64_t>(eval_stack_base, ir->arg1);
                        int64_t right = get_stack_value_at<int64_t>(eval_stack_base, ir->arg2);
                        int64_t result;
                        if (CHECK_ADD_OVERFLOW_I64(left, right, &result))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(AddOvfUnI4)
                    {
                        uint32_t left = get_stack_value_at<uint32_t>(eval_stack_base, ir->arg1);
                        uint32_t right = get_stack_value_at<uint32_t>(eval_stack_base, ir->arg2);
                        uint32_t result;
                        if (CHECK_ADD_OVERFLOW_U32(left, right, &result))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<uint32_t>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(AddOvfUnI8)
                    {
                        uint64_t left = get_stack_value_at<uint64_t>(eval_stack_base, ir->arg1);
                        uint64_t right = get_stack_value_at<uint64_t>(eval_stack_base, ir->arg2);
                        uint64_t result;
                        if (CHECK_ADD_OVERFLOW_U64(left, right, &result))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<uint64_t>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(MulOvfI4)
                    {
                        int32_t left = get_stack_value_at<int32_t>(eval_stack_base, ir->arg1);
                        int32_t right = get_stack_value_at<int32_t>(eval_stack_base, ir->arg2);
                        int32_t result;
                        if (CHECK_MUL_OVERFLOW_I32(left, right, &result))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(MulOvfI8)
                    {
                        int64_t left = get_stack_value_at<int64_t>(eval_stack_base, ir->arg1);
                        int64_t right = get_stack_value_at<int64_t>(eval_stack_base, ir->arg2);
                        int64_t result;
                        if (CHECK_MUL_OVERFLOW_I64(left, right, &result))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(MulOvfUnI4)
                    {
                        uint32_t left = get_stack_value_at<uint32_t>(eval_stack_base, ir->arg1);
                        uint32_t right = get_stack_value_at<uint32_t>(eval_stack_base, ir->arg2);
                        uint32_t result;
                        if (CHECK_MUL_OVERFLOW_U32(left, right, &result))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<uint32_t>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(MulOvfUnI8)
                    {
                        uint64_t left = get_stack_value_at<uint64_t>(eval_stack_base, ir->arg1);
                        uint64_t right = get_stack_value_at<uint64_t>(eval_stack_base, ir->arg2);
                        uint64_t result;
                        if (CHECK_MUL_OVERFLOW_U64(left, right, &result))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<uint64_t>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(SubOvfI4)
                    {
                        int32_t left = get_stack_value_at<int32_t>(eval_stack_base, ir->arg1);
                        int32_t right = get_stack_value_at<int32_t>(eval_stack_base, ir->arg2);
                        int32_t result;
                        if (CHECK_SUB_OVERFLOW_I32(left, right, &result))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(SubOvfI8)
                    {
                        int64_t left = get_stack_value_at<int64_t>(eval_stack_base, ir->arg1);
                        int64_t right = get_stack_value_at<int64_t>(eval_stack_base, ir->arg2);
                        int64_t result;
                        if (CHECK_SUB_OVERFLOW_I64(left, right, &result))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(SubOvfUnI4)
                    {
                        uint32_t left = get_stack_value_at<uint32_t>(eval_stack_base, ir->arg1);
                        uint32_t right = get_stack_value_at<uint32_t>(eval_stack_base, ir->arg2);
                        uint32_t result;
                        if (CHECK_SUB_OVERFLOW_U32(left, right, &result))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<uint32_t>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(SubOvfUnI8)
                    {
                        uint64_t left = get_stack_value_at<uint64_t>(eval_stack_base, ir->arg1);
                        uint64_t right = get_stack_value_at<uint64_t>(eval_stack_base, ir->arg2);
                        uint64_t result;
                        if (CHECK_SUB_OVERFLOW_U64(left, right, &result))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<uint64_t>(eval_stack_base, ir->dst, result);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI1I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        if (src < std::numeric_limits<int8_t>::min() || src > std::numeric_limits<int8_t>::max())
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        int8_t value = static_cast<int8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI1I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        if (src < std::numeric_limits<int8_t>::min() || src > std::numeric_limits<int8_t>::max())
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        int8_t value = static_cast<int8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI1R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < static_cast<float>(std::numeric_limits<int8_t>::min()) || src > static_cast<float>(std::numeric_limits<int8_t>::max()) ||
                            std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, int8_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI1R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < static_cast<double>(std::numeric_limits<int8_t>::min()) || src > static_cast<double>(std::numeric_limits<int8_t>::max()) ||
                            std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, int8_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU1I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        if (src < std::numeric_limits<uint8_t>::min() || src > std::numeric_limits<uint8_t>::max())
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        uint8_t value = static_cast<uint8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU1I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        if (src < std::numeric_limits<uint8_t>::min() || src > std::numeric_limits<uint8_t>::max())
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        uint8_t value = static_cast<uint8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU1R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < static_cast<float>(std::numeric_limits<uint8_t>::min()) || src > static_cast<float>(std::numeric_limits<uint8_t>::max()) ||
                            std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, uint8_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU1R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < static_cast<double>(std::numeric_limits<uint8_t>::min()) || src > static_cast<double>(std::numeric_limits<uint8_t>::max()) ||
                            std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, uint8_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI2I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        if (src < std::numeric_limits<int16_t>::min() || src > std::numeric_limits<int16_t>::max())
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        int16_t value = static_cast<int16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI2I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        if (src < std::numeric_limits<int16_t>::min() || src > std::numeric_limits<int16_t>::max())
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        int16_t value = static_cast<int16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI2R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < static_cast<float>(std::numeric_limits<int16_t>::min()) || src > static_cast<float>(std::numeric_limits<int16_t>::max()) ||
                            std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, int16_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI2R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < static_cast<double>(std::numeric_limits<int16_t>::min()) || src > static_cast<double>(std::numeric_limits<int16_t>::max()) ||
                            std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, int16_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU2I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        if (src < std::numeric_limits<uint16_t>::min() || src > std::numeric_limits<uint16_t>::max())
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        uint16_t value = static_cast<uint16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU2I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        if (src < std::numeric_limits<uint16_t>::min() || src > std::numeric_limits<uint16_t>::max())
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        uint16_t value = static_cast<uint16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU2R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < static_cast<float>(std::numeric_limits<uint16_t>::min()) || src > static_cast<float>(std::numeric_limits<uint16_t>::max()) ||
                            std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, uint16_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU2R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < static_cast<double>(std::numeric_limits<uint16_t>::min()) ||
                            src > static_cast<double>(std::numeric_limits<uint16_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, uint16_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI4I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        if (src < std::numeric_limits<int32_t>::min() || src > std::numeric_limits<int32_t>::max())
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        int32_t value = static_cast<int32_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI4R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < static_cast<float>(std::numeric_limits<int32_t>::min()) || src > static_cast<float>(std::numeric_limits<int32_t>::max()) ||
                            std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, int32_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI4R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < static_cast<double>(std::numeric_limits<int32_t>::min()) || src > static_cast<double>(std::numeric_limits<int32_t>::max()) ||
                            std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, int32_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU4I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        if (src < 0)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        uint32_t value = static_cast<uint32_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU4I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        if (src < 0 || src > static_cast<int64_t>(std::numeric_limits<uint32_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        uint32_t value = static_cast<uint32_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU4R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < 0.0f || src > static_cast<float>(std::numeric_limits<uint32_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, uint32_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU4R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < 0.0 || src > static_cast<double>(std::numeric_limits<uint32_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, uint32_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI8R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < static_cast<float>(std::numeric_limits<int64_t>::min()) || src > static_cast<float>(std::numeric_limits<int64_t>::max()) ||
                            std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, cast_float_to_i64<float, int64_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI8R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < static_cast<double>(std::numeric_limits<int64_t>::min()) || src > static_cast<double>(std::numeric_limits<int64_t>::max()) ||
                            std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, cast_float_to_i64<double, int64_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU8I4)
                    {
                        int32_t src = get_stack_value_at<int32_t>(eval_stack_base, ir->src);
                        if (src < 0)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        uint64_t value = static_cast<uint64_t>(static_cast<uint32_t>(src));
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, static_cast<int64_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU8I8)
                    {
                        int64_t src = get_stack_value_at<int64_t>(eval_stack_base, ir->src);
                        if (src < 0)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, src);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU8R4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < 0.0f || src > static_cast<float>(std::numeric_limits<uint64_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, cast_float_to_i64<float, uint64_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU8R8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < 0.0 || src > static_cast<double>(std::numeric_limits<uint64_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, cast_float_to_i64<double, uint64_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI1UnI4)
                    {
                        uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
                        if (src > static_cast<uint32_t>(std::numeric_limits<int8_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        int8_t value = static_cast<int8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI1UnI8)
                    {
                        uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
                        if (src > static_cast<uint64_t>(std::numeric_limits<int8_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        int8_t value = static_cast<int8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI1UnR4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < 0.0f || src > static_cast<float>(std::numeric_limits<int8_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, int8_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI1UnR8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < 0.0 || src > static_cast<double>(std::numeric_limits<int8_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, int8_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU1UnI4)
                    {
                        uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
                        if (src > static_cast<uint32_t>(std::numeric_limits<uint8_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        uint8_t value = static_cast<uint8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU1UnI8)
                    {
                        uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
                        if (src > static_cast<uint64_t>(std::numeric_limits<uint8_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        uint8_t value = static_cast<uint8_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU1UnR4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < 0.0f || src > static_cast<float>(std::numeric_limits<uint8_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, uint8_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU1UnR8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < 0.0 || src > static_cast<double>(std::numeric_limits<uint8_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, uint8_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI2UnI4)
                    {
                        uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
                        if (src > static_cast<uint32_t>(std::numeric_limits<int16_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        int16_t value = static_cast<int16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI2UnI8)
                    {
                        uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
                        if (src > static_cast<uint64_t>(std::numeric_limits<int16_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        int16_t value = static_cast<int16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI2UnR4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < 0.0f || src > static_cast<float>(std::numeric_limits<int16_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, int16_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI2UnR8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < 0.0 || src > static_cast<double>(std::numeric_limits<int16_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, int16_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU2UnI4)
                    {
                        uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
                        if (src > static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        uint16_t value = static_cast<uint16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU2UnI8)
                    {
                        uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
                        if (src > static_cast<uint64_t>(std::numeric_limits<uint16_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        uint16_t value = static_cast<uint16_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU2UnR4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < 0.0f || src > static_cast<float>(std::numeric_limits<uint16_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, uint16_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU2UnR8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < 0.0 || src > static_cast<double>(std::numeric_limits<uint16_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, uint16_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI4UnI4)
                    {
                        uint32_t src = get_stack_value_at<uint32_t>(eval_stack_base, ir->src);
                        if (src > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        int32_t value = static_cast<int32_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI4UnI8)
                    {
                        uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
                        if (src > static_cast<uint64_t>(std::numeric_limits<int32_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        int32_t value = static_cast<int32_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI4UnR4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < 0.0f || src > static_cast<float>(std::numeric_limits<int32_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, int32_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI4UnR8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < 0.0 || src > static_cast<double>(std::numeric_limits<int32_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, int32_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU4UnI8)
                    {
                        uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
                        if (src > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        uint32_t value = static_cast<uint32_t>(src);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU4UnR4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < 0.0f || src > static_cast<float>(std::numeric_limits<uint32_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<float, uint32_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU4UnR8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < 0.0 || src > static_cast<double>(std::numeric_limits<uint32_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, cast_float_to_i32<double, uint32_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI8UnI8)
                    {
                        uint64_t src = get_stack_value_at<uint64_t>(eval_stack_base, ir->src);
                        if (src > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        int64_t value = static_cast<int64_t>(src);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI8UnR4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < 0.0f || src > static_cast<float>(std::numeric_limits<int64_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, cast_float_to_i64<float, int64_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfI8UnR8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < 0.0 || src > static_cast<double>(std::numeric_limits<int64_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, cast_float_to_i64<double, int64_t>(src));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU8UnR4)
                    {
                        float src = get_stack_value_at<float>(eval_stack_base, ir->src);
                        if (src < 0.0f || src > static_cast<float>(std::numeric_limits<uint64_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<uint64_t>(eval_stack_base, ir->dst, static_cast<uint64_t>(cast_float_to_i64<float, uint64_t>(src)));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(ConvOvfU8UnR8)
                    {
                        double src = get_stack_value_at<double>(eval_stack_base, ir->src);
                        if (src < 0.0 || src > static_cast<double>(std::numeric_limits<uint64_t>::max()) || std::isnan(src))
                        {
                            RAISE_RUNTIME_ERROR(RtErr::Overflow);
                        }
                        set_stack_value_at<uint64_t>(eval_stack_base, ir->dst, static_cast<uint64_t>(cast_float_to_i64<double, uint64_t>(src)));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(InitObjI2Unaligned)
                    {
                        uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->addr);
                        utils::MemOp::write_i16_may_unaligned(addr, 0);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(InitObjI4Unaligned)
                    {
                        uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->addr);
                        utils::MemOp::write_i32_may_unaligned(addr, 0);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(InitObjI8Unaligned)
                    {
                        uint8_t* addr = get_stack_value_at<uint8_t*>(eval_stack_base, ir->addr);
                        utils::MemOp::write_i64_may_unaligned(addr, 0);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdfldI1Large)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const int8_t* field_addr = reinterpret_cast<const int8_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                        int8_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdfldU1Large)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                        uint8_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdfldI2Large)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const int16_t* field_addr = reinterpret_cast<const int16_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                        int16_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdfldI2Unaligned)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                        int16_t value = utils::MemOp::read_i16_may_unaligned(field_addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdfldU2Large)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint16_t* field_addr = reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                        uint16_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdfldU2Unaligned)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                        uint16_t value = utils::MemOp::read_u16_may_unaligned(field_addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdfldI4Large)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const int32_t* field_addr = reinterpret_cast<const int32_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                        int32_t value = *field_addr;
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdfldI4Unaligned)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                        int32_t value = utils::MemOp::read_i32_may_unaligned(field_addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdfldI8Large)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const int64_t* field_addr = reinterpret_cast<const int64_t*>(reinterpret_cast<const uint8_t*>(obj) + ir->offset);
                        int64_t value = *field_addr;
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdfldI8Unaligned)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                        int64_t value = utils::MemOp::read_i64_may_unaligned(field_addr);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdfldAnyLarge)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        std::memcpy(dst, src_addr, ir->size);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdvfldI1Large)
                    {
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
                        int8_t value = *reinterpret_cast<const int8_t*>(src_addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdvfldU1Large)
                    {
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
                        uint8_t value = *reinterpret_cast<const uint8_t*>(src_addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdvfldI2Large)
                    {
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
                        int16_t value = *reinterpret_cast<const int16_t*>(src_addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdvfldI2Unaligned)
                    {
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
                        int16_t value;
                        std::memcpy(&value, src_addr, sizeof(value));
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdvfldU2Large)
                    {
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
                        uint16_t value = *reinterpret_cast<const uint16_t*>(src_addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdvfldU2Unaligned)
                    {
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
                        uint16_t value = utils::MemOp::read_u16_may_unaligned(src_addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, static_cast<int32_t>(value));
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdvfldI4Large)
                    {
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
                        int32_t value = *reinterpret_cast<const int32_t*>(src_addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdvfldI4Unaligned)
                    {
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
                        int32_t value = utils::MemOp::read_i32_may_unaligned(src_addr);
                        set_stack_value_at<int32_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdvfldI8Large)
                    {
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
                        int64_t value = *reinterpret_cast<const int64_t*>(src_addr);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdvfldI8Unaligned)
                    {
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
                        int64_t value = utils::MemOp::read_i64_may_unaligned(src_addr);
                        set_stack_value_at<int64_t>(eval_stack_base, ir->dst, value);
                    }

                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdvfldAnyLarge)
                    {
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->obj) + ir->offset;
                        RtStackObject* dst = eval_stack_base + ir->dst;
                        // may overlap
                        std::memmove(dst, src_addr, ir->size);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(LdfldaLarge)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint8_t* field_addr = reinterpret_cast<const uint8_t*>(obj) + ir->offset;
                        set_stack_value_at(eval_stack_base, ir->dst, field_addr);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(StfldI1Large)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int8_t value = get_stack_value_at<int8_t>(eval_stack_base, ir->value);
                        int8_t* field_addr = reinterpret_cast<int8_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
                        *field_addr = value;
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(StfldI2Large)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->value);
                        int16_t* field_addr = reinterpret_cast<int16_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
                        *field_addr = value;
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(StfldI2Unaligned)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int16_t value = get_stack_value_at<int16_t>(eval_stack_base, ir->value);
                        uint8_t* field_addr = reinterpret_cast<uint8_t*>(obj) + ir->offset;
                        utils::MemOp::write_i16_may_unaligned(field_addr, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(StfldI4Large)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
                        int32_t* field_addr = reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
                        *field_addr = value;
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(StfldI4Unaligned)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int32_t value = get_stack_value_at<int32_t>(eval_stack_base, ir->value);
                        uint8_t* field_addr = reinterpret_cast<uint8_t*>(obj) + ir->offset;
                        utils::MemOp::write_i32_may_unaligned(field_addr, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(StfldI8Large)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
                        int64_t* field_addr = reinterpret_cast<int64_t*>(reinterpret_cast<uint8_t*>(obj) + ir->offset);
                        *field_addr = value;
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(StfldI8Unaligned)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        int64_t value = get_stack_value_at<int64_t>(eval_stack_base, ir->value);
                        uint8_t* field_addr = reinterpret_cast<uint8_t*>(obj) + ir->offset;
                        utils::MemOp::write_i64_may_unaligned(field_addr, value);
                    }
                    LEANCLR_CASE_END3()
                    LEANCLR_CASE_BEGIN3(StfldAnyLarge)
                    {
                        vm::RtObject* obj = get_stack_value_at<vm::RtObject*>(eval_stack_base, ir->obj);
                        if (!obj)
                        {
                            RAISE_RUNTIME_ERROR(RtErr::NullReference);
                        }
                        const uint8_t* src_addr = reinterpret_cast<const uint8_t*>(eval_stack_base + ir->value);
                        uint8_t* dst_addr = reinterpret_cast<uint8_t*>(obj) + ir->offset;
                        std::memmove(dst_addr, src_addr, ir->size);
                    }
                    LEANCLR_CASE_END3()
#if !LEANCLR_USE_COMPUTED_GOTO_DISPATCHER
                default:
                {
                    assert(false && "Invalid opcode");
                    RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
                }
#endif
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(Prefix4)
            {
                LEANCLR_SWITCH4()
                {
                    LEANCLR_CASE_BEGIN4(Illegal)
                    {
                        assert(false && "never reach here");
                        RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
                    }
                    LEANCLR_CASE_END4()
                    LEANCLR_CASE_BEGIN4(Nop)
                    {
                        // do nothing
                    }
                    LEANCLR_CASE_END4()
                    LEANCLR_CASE_BEGIN4(Arglist)
                    {
                        assert(false && "Not implemented");
                        RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
                    }
                    LEANCLR_CASE_END4()
#if !LEANCLR_USE_COMPUTED_GOTO_DISPATCHER
                default:
                {
                    assert(false && "Invalid opcode");
                    RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
                }
#endif
                }
            }
            LEANCLR_CASE_END_LITE0()
            LEANCLR_CASE_BEGIN_LITE0(Prefix5)
            {
                assert(false && "Not implemented");
                RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
            }
            LEANCLR_CASE_END_LITE0()
#if !LEANCLR_USE_COMPUTED_GOTO_DISPATCHER
        default:
        {
            assert(false && "Invalid opcode");
            RAISE_RUNTIME_ERROR(RtErr::ExecutionEngine);
        }
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
                    if (!clause->ex_klass || vm::Class::is_assignable_from(ex->klass, clause->ex_klass))
                    {
                        ip = imi->codes + clause->handler_begin_offset;
                        setup_catch_handler(imi, frame, clause, ip);
                        set_stack_value_at<vm::RtObject*>(eval_stack_base, imi->total_arg_and_local_stack_object_size, ex);
                        handled = true;
                    }
                    break;
                }
                case metadata::RtILExceptionClauseType::Filter:
                {
                    ip = imi->codes + clause->handler_begin_offset;
                    setup_filter_checker(clause);
                    set_stack_value_at<vm::RtObject*>(eval_stack_base, imi->total_arg_and_local_stack_object_size, ex);
                    handled = true;
                    break;
                }
                case metadata::RtILExceptionClauseType::Finally:
                case metadata::RtILExceptionClauseType::Fault:
                {
                    ip = imi->codes + clause->handler_begin_offset;
                    setup_finally_or_fault_handler(imi, clause, ip);
                    set_stack_value_at<vm::RtObject*>(eval_stack_base, imi->total_arg_and_local_stack_object_size, ex);
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

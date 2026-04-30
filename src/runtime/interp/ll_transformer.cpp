#include <climits>
#include <cstdint>
#include "ll_transformer.h"
#include "hl_transformer.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/rt_string.h"
#include "vm/assembly.h"
#include "vm/array_class.h"
#include "metadata/metadata_const.h"
#include "metadata/module_def.h"
#include "utils/platform.h"
#include "const_strs.h"

namespace leanclr
{
namespace interp
{
namespace ll
{

GeneralInst::GeneralInst(const hl::GeneralInst& hl_inst)
{
    arg1_or_src = hl_inst.arg1_or_src;
    arg2 = hl_inst.arg2;
    arg3 = hl_inst.arg3;
    dst_or_ret = hl_inst.dst_or_ret;
    extra_data = hl_inst.extra_data;
#if LEANCLR_DEBUG
    extra_data2.value = 0;
    resolved_data_idx = 0;
    ir_offset = 0;
#endif
    il_offset = hl_inst.get_il_offset();
}

RtResult<BasicBlock*> Transformer::translate_hl_basic_to_ll_basic(const hl::BasicBlock* hl_bb)
{
    auto it = _hl_2_ll_bb_map.find(hl_bb);
    if (it != _hl_2_ll_bb_map.end())
    {
        RET_OK(it->second);
    }
    RET_ERR(core::RtErr::ExecutionEngine);
}

RtResultVoid Transformer::transform_basic_blocks()
{
    const hl::BasicBlock* hl_basic_blocks = _hl_transformer.get_basic_blocks();
    BasicBlock* prev_ll_bb = nullptr;

    for (size_t i = 0; i < _hl_transformer.get_basic_block_count(); ++i)
    {
        const hl::BasicBlock* hl_bb = hl_basic_blocks + i;
        BasicBlock* ll_bb = _mem_pool.new_any<BasicBlock>(&_mem_pool, hl_bb);
        _hl_2_ll_bb_map.insert({hl_bb, ll_bb});

        if (prev_ll_bb != nullptr)
        {
            prev_ll_bb->next_bb = ll_bb;
        }
        else
        {
            _bb_head = ll_bb;
        }
        prev_ll_bb = ll_bb;
    }
    RET_VOID_OK();
}

size_t Transformer::get_resolved_data_index(const void* data)
{
    auto it = _resolved_data_2_index_map.find(data);
    if (it != _resolved_data_2_index_map.end())
    {
        return it->second;
    }

    size_t index = _resolved_datas.size();
    _resolved_datas.push_back(data);
    _resolved_data_2_index_map.insert({data, index});
    return index;
}

void Transformer::setup_inst_resolved_data(GeneralInst* ll_inst, const void* data)
{
    size_t index = get_resolved_data_index(data);
    ll_inst->set_resolved_data_index(index);
}

void Transformer::setup_inst_klass(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst)
{
    metadata::RtClass* klass = hl_inst->get_class();
    setup_inst_resolved_data(ll_inst, klass);
}

void Transformer::setup_inst_method(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst)
{
    const metadata::RtMethodInfo* method = hl_inst->get_method();
    setup_inst_resolved_data(ll_inst, method);
}

utils::NotFreeList<size_t> Transformer::find_finally_clause_idx_of_leave_target(const BasicBlock* leave_src, const BasicBlock* leave_target)
{
    uint32_t il_src = static_cast<uint32_t>(leave_src->hl_bb->il_begin_offset);
    uint32_t il_target = static_cast<uint32_t>(leave_target->hl_bb->il_begin_offset);
    utils::NotFreeList<size_t> result(&_mem_pool);

    auto* method_body = _hl_transformer.get_method_body();
    const auto& exception_clauses = method_body->exception_clauses;

    for (size_t i = 0; i < exception_clauses.size(); ++i)
    {
        const auto& clause = exception_clauses[i];
        if (clause.flags == metadata::RtILExceptionClauseType::Finally && clause.is_in_try_block(il_src) && !clause.is_in_try_block(il_target))
        {
            result.push_back(i);
        }
    }

    return result;
}

Transformer::LeaveSurroundingBlockType Transformer::get_leave_surrounding_block_type(const BasicBlock* leave_src, const BasicBlock* leave_target)
{
    uint32_t il_src = static_cast<uint32_t>(leave_src->hl_bb->il_begin_offset);
    uint32_t il_target = static_cast<uint32_t>(leave_target->hl_bb->il_begin_offset);

    auto* method_body = _hl_transformer.get_method_body();
    const auto& exception_clauses = method_body->exception_clauses;

    LeaveSurroundingBlockType result = LeaveSurroundingBlockType::None;

    for (const auto& clause : exception_clauses)
    {
        if (clause.flags == metadata::RtILExceptionClauseType::Fault)
        {
            continue;
        }

        if (clause.is_in_try_block(il_src) && !clause.is_in_try_block(il_target))
        {
            result = LeaveSurroundingBlockType::Try;
        }
        else if (clause.is_in_handler_block(il_src) && !clause.is_in_handler_block(il_target))
        {
            result = LeaveSurroundingBlockType::CatchOrFilter;
        }
    }

    return result;
}

metadata::RtILExceptionClauseType Transformer::get_cur_endfinally_or_fault_clause_type(const BasicBlock* cur_bb)
{
    uint32_t il_src = static_cast<uint32_t>(cur_bb->hl_bb->il_begin_offset);

    auto* method_body = _hl_transformer.get_method_body();
    const auto& exception_clauses = method_body->exception_clauses;

    metadata::RtILExceptionClauseType result = metadata::RtILExceptionClauseType::Exception;

    for (const auto& clause : exception_clauses)
    {
        if (clause.flags != metadata::RtILExceptionClauseType::Finally && clause.flags != metadata::RtILExceptionClauseType::Fault)
        {
            continue;
        }

        if (clause.is_in_handler_block(il_src))
        {
            result = clause.flags;
        }
    }

    return result;
}

RtResultVoid Transformer::transform_condition_branch(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8,
                                                     OpCodeEnum opcode_r4, OpCodeEnum opcode_r8)
{
    const Variable* src = hl_inst->get_var_src();

    switch (src->data_type)
    {
    case RtEvalStackDataType::I4:
        ll_inst->set_opcode(opcode_i4); // Set opcode for I4
        break;
    case RtEvalStackDataType::I8:
        ll_inst->set_opcode(opcode_i8); // Set opcode for I8
        break;
    case RtEvalStackDataType::R4:
        ll_inst->set_opcode(opcode_r4); // Set opcode for R4
        break;
    case RtEvalStackDataType::R8:
        ll_inst->set_opcode(opcode_r8); // Set opcode for R8
        break;
    case RtEvalStackDataType::RefOrPtr:
        ll_inst->set_opcode(utils::Platform::select_arch(opcode_i4, opcode_i8)); // Set opcode for RefOrPtr
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    const hl::BasicBlock* target_hl_bb = hl_inst->get_branch_target();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(BasicBlock*, target_ll_bb, translate_hl_basic_to_ll_basic(target_hl_bb));
    ll_inst->update_branch_target(target_ll_bb);

    RET_VOID_OK();
}

RtResultVoid Transformer::transform_brtruefalse(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8)
{
    const Variable* src = hl_inst->get_var_src();
    OpCodeEnum ll_op;

    switch (src->data_type)
    {
    case RtEvalStackDataType::I4:
        ll_op = opcode_i4;
        break;
    case RtEvalStackDataType::I8:
        ll_op = opcode_i8;
        break;
    case RtEvalStackDataType::RefOrPtr:
        ll_op = utils::Platform::select_arch(opcode_i4, opcode_i8);
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    ll_inst->set_opcode(ll_op);

    const hl::BasicBlock* target_hl_bb = hl_inst->get_branch_target();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(BasicBlock*, target_ll_bb, translate_hl_basic_to_ll_basic(target_hl_bb));
    ll_inst->update_branch_target(target_ll_bb);

    RET_VOID_OK();
}

RtResultVoid Transformer::transform_bin_arith_op(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8,
                                                 OpCodeEnum opcode_r4, OpCodeEnum opcode_r8)
{
    const Variable* dst = hl_inst->get_var_dst();

    switch (dst->data_type)
    {
    case RtEvalStackDataType::I4:
        ll_inst->set_opcode(opcode_i4);
        break;
    case RtEvalStackDataType::I8:
        ll_inst->set_opcode(opcode_i8);
        break;
    case RtEvalStackDataType::R4:
        ll_inst->set_opcode(opcode_r4);
        break;
    case RtEvalStackDataType::R8:
        ll_inst->set_opcode(opcode_r8);
        break;
    case RtEvalStackDataType::RefOrPtr:
        ll_inst->set_opcode(utils::Platform::select_arch(opcode_i4, opcode_i8));
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    RET_VOID_OK();
}

RtResultVoid Transformer::transform_bin_arith_ovf_op(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8)
{
    const Variable* dst = hl_inst->get_var_dst();

    switch (dst->data_type)
    {
    case RtEvalStackDataType::I4:
        ll_inst->set_opcode(opcode_i4); // Set opcode for I4
        break;
    case RtEvalStackDataType::I8:
        ll_inst->set_opcode(opcode_i8); // Set opcode for I8
        break;
    case RtEvalStackDataType::RefOrPtr:
        ll_inst->set_opcode(utils::Platform::select_arch(opcode_i4, opcode_i8)); // Set opcode for RefOrPtr
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    RET_VOID_OK();
}

RtResultVoid Transformer::transform_bin_bit_op(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8)
{
    const Variable* dst = hl_inst->get_var_dst();

    switch (dst->data_type)
    {
    case RtEvalStackDataType::I4:
        ll_inst->set_opcode(opcode_i4); // Set opcode for I4
        break;
    case RtEvalStackDataType::I8:
        ll_inst->set_opcode(opcode_i8); // Set opcode for I8
        break;
    case RtEvalStackDataType::RefOrPtr:
        ll_inst->set_opcode(utils::Platform::select_arch(opcode_i4, opcode_i8)); // Set opcode for RefOrPtr
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    RET_VOID_OK();
}

RtResultVoid Transformer::transform_bin_bit_shift_op(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8)
{
    const Variable* shift_amount = hl_inst->get_var_arg2();
    const Variable* dst = hl_inst->get_var_dst();

    if (shift_amount->data_type != RtEvalStackDataType::I4 && shift_amount->data_type != RtEvalStackDataType::RefOrPtr)
    {
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    switch (dst->data_type)
    {
    case RtEvalStackDataType::I4:
        ll_inst->set_opcode(opcode_i4);
        break;
    case RtEvalStackDataType::I8:
        ll_inst->set_opcode(opcode_i8);
        break;
    case RtEvalStackDataType::RefOrPtr:
        ll_inst->set_opcode(utils::Platform::select_arch(opcode_i4, opcode_i8));
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    RET_VOID_OK();
}

RtResultVoid Transformer::transform_conv(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8, OpCodeEnum opcode_r4,
                                         OpCodeEnum opcode_r8)
{
    const Variable* src = hl_inst->get_var_src();
    OpCodeEnum opcode;

    switch (src->data_type)
    {
    case RtEvalStackDataType::I4:
        opcode = opcode_i4;
        break;
    case RtEvalStackDataType::I8:
        opcode = opcode_i8;
        break;
    case RtEvalStackDataType::R4:
        opcode = opcode_r4;
        break;
    case RtEvalStackDataType::R8:
        opcode = opcode_r8;
        break;
    case RtEvalStackDataType::RefOrPtr:
        opcode = utils::Platform::select_arch(opcode_i4, opcode_i8);
        break;
    default:
        RETURN_NOT_IMPLEMENTED_ERROR();
    }

    ll_inst->set_opcode(opcode);
    RET_VOID_OK();
}

RtResultVoid Transformer::transform_cmp_op(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8,
                                           OpCodeEnum opcode_r4, OpCodeEnum opcode_r8)
{
    const Variable* op1 = hl_inst->get_var_arg1();

    switch (op1->data_type)
    {
    case RtEvalStackDataType::I4:
        ll_inst->set_opcode(opcode_i4);
        break;
    case RtEvalStackDataType::I8:
        ll_inst->set_opcode(opcode_i8);
        break;
    case RtEvalStackDataType::R4:
        ll_inst->set_opcode(opcode_r4);
        break;
    case RtEvalStackDataType::R8:
        ll_inst->set_opcode(opcode_r8);
        break;
    case RtEvalStackDataType::RefOrPtr:
        ll_inst->set_opcode(utils::Platform::select_arch(opcode_i4, opcode_i8));
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    RET_VOID_OK();
}

RtResult<bool> Transformer::transform_special_call_methods(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst)
{
    const metadata::RtMethodInfo* method = hl_inst->get_method();
    const metadata::RtClass* klass = method->parent;

    if (!klass->image->is_corlib())
    {
        RET_OK(false);
    }

    ll_inst->set_opcode(OpCodeEnum::Illegal);

    const vm::CorLibTypes& corlib_types = vm::Class::get_corlib_types();
    const char* klass_name = klass->name;
    const char* method_name = method->name;
    size_t param_count = static_cast<size_t>(method->parameter_count);

    const Variable* const* params = ll_inst->get_params();

    if (klass == corlib_types.cls_object)
    {
        if (std::strcmp(method_name, STR_CTOR) == 0)
        {
            ll_inst->set_opcode(OpCodeEnum::Nop);
        }
    }
    else if (klass == corlib_types.cls_array)
    {
        if (std::strcmp(method_name, "UnsafeMov") == 0)
        {
            ll_inst->set_opcode(OpCodeEnum::Nop);
        }
    }
    else if (klass == corlib_types.cls_intptr)
    {
        if (std::strcmp(method_name, STR_CTOR) == 0)
        {
            if (param_count == 1)
            {
                const Variable* arg_this = params[0];
                const Variable* arg_value = params[1];
                const metadata::RtTypeSig* param_type = method->parameters[0];

                switch (param_type->ele_type)
                {
                case metadata::RtElementType::I4:
                    ll_inst->set_opcode(utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8I4));
                    ll_inst->update_var_src(arg_value);
                    ll_inst->update_var_dst(arg_this);
                    break;
                case metadata::RtElementType::I8:
                case metadata::RtElementType::U8:
                case metadata::RtElementType::I:
                case metadata::RtElementType::U:
                case metadata::RtElementType::Ptr:
                    ll_inst->set_opcode(utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8));
                    ll_inst->update_var_src(arg_value);
                    ll_inst->update_var_dst(arg_this);
                    break;
                default:
                    break;
                }
            }
        }
        // TODO: explicit operator, Subtract, ToXXX
    }
    else if (klass == corlib_types.cls_uintptr)
    {
        if (std::strcmp(method_name, STR_CTOR) == 0)
        {
            if (param_count == 1)
            {
                const Variable* arg_this = params[0];
                const Variable* arg_value = params[1];
                const metadata::RtTypeSig* param_type = method->parameters[0];

                switch (param_type->ele_type)
                {
                case metadata::RtElementType::U4:
                    ll_inst->set_opcode(utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8U4));
                    ll_inst->update_var_src(arg_value);
                    ll_inst->update_var_dst(arg_this);
                    break;
                case metadata::RtElementType::I8:
                case metadata::RtElementType::U8:
                case metadata::RtElementType::I:
                case metadata::RtElementType::U:
                case metadata::RtElementType::Ptr:
                    ll_inst->set_opcode(utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8));
                    ll_inst->update_var_src(arg_value);
                    ll_inst->update_var_dst(arg_this);
                    break;
                default:
                    break;
                }
            }
        }
        // TODO: explicit operator, Subtract, ToXXX
    }
    else if (std::strcmp(klass_name, "RuntimeHelpers") == 0)
    {
        if (std::strcmp(method_name, "get_OffsetToStringData") == 0)
        {
            int32_t offset = vm::String::get_offset_to_string_data();
            ll_inst->set_opcode(offset <= INT16_MAX ? OpCodeEnum::LdcI4I2 : OpCodeEnum::LdcI4I4);
            ll_inst->set_i4(offset);
            ll_inst->update_var_dst(ll_inst->get_var_ret());
        }
    }
    else if (klass == corlib_types.cls_appdomain)
    {
        if (std::strcmp(method_name, "IsAppXModel") == 0)
        {
            ll_inst->set_opcode(OpCodeEnum::LdcI4I2);
            ll_inst->set_i4(0); // false
            ll_inst->update_var_dst(ll_inst->get_var_ret());
        }
    }
    else if (std::strcmp(klass_name, "ByReference`1") == 0)
    {
        if (std::strcmp(method_name, STR_CTOR) == 0)
        {
            ll_inst->set_opcode(OpCodeEnum::Nop);
        }
        else if (std::strcmp(method_name, "get_Value") == 0)
        {
            const Variable* arg_this = params[0];
            const Variable* dst = ll_inst->get_var_ret();
            ll_inst->set_opcode(utils::Platform::select_arch(OpCodeEnum::LdIndI4, OpCodeEnum::LdIndI8));
            ll_inst->update_var_src(arg_this);
            ll_inst->update_var_dst(dst);
        }
    }
    else if (std::strcmp(klass_name, "JitHelpers") == 0)
    {
        if (std::strcmp(method_name, "UnsafeCast") == 0 || std::strcmp(method_name, "UnsafeEnumCast") == 0 || std::strcmp(method_name, "UnsafeEnumCastLong"))
        {
            ll_inst->set_opcode(OpCodeEnum::Nop);
        }
    }

    RET_OK(ll_inst->get_opcode() != OpCodeEnum::Illegal);
}

RtResult<bool> Transformer::transform_special_newobj_methods(GeneralInst* ll_inst, const hl::GeneralInst* hl_inst)
{
    ll_inst->set_opcode(OpCodeEnum::Illegal);
    const metadata::RtMethodInfo* method = hl_inst->get_method();
    const metadata::RtClass* klass = method->parent;
    if (!klass->image->is_corlib())
    {
        RET_OK(false);
    }

    const vm::CorLibTypes& corlib_types = vm::Class::get_corlib_types();
    const char* klass_name = klass->name;
    size_t param_count = static_cast<size_t>(method->parameter_count);

    const Variable* const* params = ll_inst->get_params();

    if (klass == corlib_types.cls_object)
    {
        // TODO: add instruction for new object
    }
    else if (klass == corlib_types.cls_intptr)
    {
        if (param_count == 1)
        {
            const Variable* arg_value = params[0];
            const metadata::RtTypeSig* param_type = method->parameters[0];

            switch (param_type->ele_type)
            {
            case metadata::RtElementType::I4:
                ll_inst->set_opcode(utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvI8I4));
                ll_inst->update_var_src(arg_value);
                ll_inst->update_var_dst(arg_value);
                break;
            case metadata::RtElementType::I8:
            case metadata::RtElementType::U8:
            case metadata::RtElementType::I:
            case metadata::RtElementType::U:
            case metadata::RtElementType::Ptr:
                ll_inst->set_opcode(OpCodeEnum::Nop);
                break;
            default:
                break;
            }
        }
        // TODO: explicit operator, Subtract, ToXXX
    }
    else if (klass == corlib_types.cls_uintptr)
    {
        if (param_count == 1)
        {
            const Variable* arg_value = params[0];
            const metadata::RtTypeSig* param_type = method->parameters[0];

            switch (param_type->ele_type)
            {
            case metadata::RtElementType::U4:
                ll_inst->set_opcode(utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvI8U4));
                ll_inst->update_var_src(arg_value);
                ll_inst->update_var_dst(arg_value);
                break;
            case metadata::RtElementType::I8:
            case metadata::RtElementType::U8:
            case metadata::RtElementType::I:
            case metadata::RtElementType::U:
            case metadata::RtElementType::Ptr:
                ll_inst->set_opcode(OpCodeEnum::Nop);
                break;
            default:
                break;
            }
        }
        // TODO: explicit operator, Subtract, ToXXX
    }
    else if (std::strcmp(klass_name, "ByReference`1") == 0)
    {
        assert(param_count == 1);
        ll_inst->set_opcode(OpCodeEnum::Nop);
    }

    RET_OK(ll_inst->get_opcode() != OpCodeEnum::Illegal);
}

RtResultVoid Transformer::transform_instructions()
{
    BasicBlock* cur_bb = _bb_head;

    while (cur_bb != nullptr)
    {
        const hl::BasicBlock* hl_bb = cur_bb->hl_bb;
        cur_bb->insts.reserve(hl_bb->insts.size());

        for (const hl::GeneralInst* hl_inst : hl_bb->insts)
        {
            GeneralInst* ll_inst = _mem_pool.new_any<GeneralInst>(*hl_inst);
            bool add = true;
            hl::OpCodeEnum opcode = hl_inst->get_opcode();
            switch (opcode)
            {
            case hl::OpCodeEnum::Nop:
                add = false;
                break;
            case hl::OpCodeEnum::Arglist:
                ll_inst->set_opcode(OpCodeEnum::Arglist);
                break;
            case hl::OpCodeEnum::InitLocals:
            {
                size_t offset = hl_inst->get_locals_offset();
                size_t local_size = hl_inst->get_size();
                if (offset <= UINT8_MAX && local_size <= 4)
                {
                    switch (local_size)
                    {
                    case 0:
                        add = false;
                        break;
                    case 1:
                        ll_inst->set_opcode(OpCodeEnum::InitLocals1Short);
                        break;
                    case 2:
                        ll_inst->set_opcode(OpCodeEnum::InitLocals2Short);
                        break;
                    case 3:
                        ll_inst->set_opcode(OpCodeEnum::InitLocals3Short);
                        break;
                    case 4:
                        ll_inst->set_opcode(OpCodeEnum::InitLocals4Short);
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    ll_inst->set_opcode(OpCodeEnum::InitLocals);
                }
                // use different fields to store size
                // ll_inst->set_size(local_size);
                break;
            }

            case hl::OpCodeEnum::LdArg:
            case hl::OpCodeEnum::LdLoc:
            {
                const Variable* src = hl_inst->get_var_arg1();
                switch (src->reduce_type)
                {
                case metadata::RtArgOrLocOrFieldReduceType::I1:
                    ll_inst->set_opcode(OpCodeEnum::LdLocI1);
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::U1:
                    ll_inst->set_opcode(OpCodeEnum::LdLocU1);
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I2:
                    ll_inst->set_opcode(OpCodeEnum::LdLocI2);
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::U2:
                    ll_inst->set_opcode(OpCodeEnum::LdLocU2);
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I4:
                case metadata::RtArgOrLocOrFieldReduceType::R4:
                    ll_inst->set_opcode(OpCodeEnum::LdLocI4);
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I8:
                case metadata::RtArgOrLocOrFieldReduceType::R8:
                    ll_inst->set_opcode(OpCodeEnum::LdLocI8);
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I:
                case metadata::RtArgOrLocOrFieldReduceType::Ref:
                    ll_inst->set_opcode(utils::Platform::select_arch(OpCodeEnum::LdLocI4, OpCodeEnum::LdLocI8));
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::Other:
                    ll_inst->set_opcode(OpCodeEnum::LdLocAny);
                    ll_inst->set_stack_object_size(src->stack_object_size);
                    break;
                default:
                    RET_ERR(core::RtErr::ExecutionEngine);
                }
                break;
            }

            case hl::OpCodeEnum::LdArga:
            case hl::OpCodeEnum::LdLoca:
                ll_inst->set_opcode(OpCodeEnum::LdLoca);
                break;

            case hl::OpCodeEnum::StArg:
            case hl::OpCodeEnum::StLoc:
            {
                const Variable* dst = hl_inst->get_var_dst();
                switch (dst->reduce_type)
                {
                case metadata::RtArgOrLocOrFieldReduceType::I1:
                case metadata::RtArgOrLocOrFieldReduceType::U1:
                    ll_inst->set_opcode(OpCodeEnum::StLocI1);
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I2:
                case metadata::RtArgOrLocOrFieldReduceType::U2:
                    ll_inst->set_opcode(OpCodeEnum::StLocI2);
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I4:
                case metadata::RtArgOrLocOrFieldReduceType::R4:
                    ll_inst->set_opcode(OpCodeEnum::StLocI4);
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I8:
                case metadata::RtArgOrLocOrFieldReduceType::R8:
                    ll_inst->set_opcode(OpCodeEnum::StLocI8);
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I:
                case metadata::RtArgOrLocOrFieldReduceType::Ref:
                    ll_inst->set_opcode(utils::Platform::select_arch(OpCodeEnum::StLocI4, OpCodeEnum::StLocI8));
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::Other:
                    ll_inst->set_opcode(OpCodeEnum::StLocAny);
                    ll_inst->set_stack_object_size(dst->stack_object_size);
                    break;
                default:
                    RET_ERR(core::RtErr::ExecutionEngine);
                }
                break;
            }

            case hl::OpCodeEnum::LdNull:
                ll_inst->set_opcode(OpCodeEnum::LdNull);
                break;

            case hl::OpCodeEnum::LdcI4:
            {
                int32_t value = hl_inst->get_i4();
                if (value >= INT16_MIN && value <= INT16_MAX)
                {
                    ll_inst->set_opcode(OpCodeEnum::LdcI4I2);
                }
                else
                {
                    ll_inst->set_opcode(OpCodeEnum::LdcI4I4);
                }
                break;
            }

            case hl::OpCodeEnum::LdcI8:
            {
                int64_t value = hl_inst->get_i8();
                if (value >= INT16_MIN && value <= INT16_MAX)
                {
                    ll_inst->set_opcode(OpCodeEnum::LdcI8I2);
                }
                else if (value >= INT32_MIN && value <= INT32_MAX)
                {
                    ll_inst->set_opcode(OpCodeEnum::LdcI8I4);
                }
                else
                {
                    ll_inst->set_opcode(OpCodeEnum::LdcI8I8);
                }
                break;
            }

            case hl::OpCodeEnum::LdcR4:
                ll_inst->set_opcode(OpCodeEnum::LdcI4I4);
                break;

            case hl::OpCodeEnum::LdcR8:
                ll_inst->set_opcode(OpCodeEnum::LdcI8I8);
                break;

            case hl::OpCodeEnum::LdStr:
                ll_inst->set_opcode(OpCodeEnum::LdStr);
                setup_inst_resolved_data(ll_inst, hl_inst->get_user_string());
                break;

            case hl::OpCodeEnum::Dup:
            {
                const Variable* src = hl_inst->get_var_src();
                switch (src->data_type)
                {
                case RtEvalStackDataType::I4:
                case RtEvalStackDataType::R4:
                    ll_inst->set_opcode(OpCodeEnum::LdLocI4);
                    break;
                case RtEvalStackDataType::I8:
                case RtEvalStackDataType::R8:
                    ll_inst->set_opcode(OpCodeEnum::LdLocI8);
                    break;
                case RtEvalStackDataType::RefOrPtr:
                    ll_inst->set_opcode(utils::Platform::select_arch(OpCodeEnum::LdLocI4, OpCodeEnum::LdLocI8));
                    break;
                case RtEvalStackDataType::Other:
                    ll_inst->set_opcode(OpCodeEnum::LdLocAny);
                    ll_inst->set_stack_object_size(src->stack_object_size);
                    break;
                default:
                    RET_ERR(core::RtErr::ExecutionEngine);
                }
                break;
            }

            case hl::OpCodeEnum::Ret:
            {
                const Variable* ret = _hl_transformer.get_ret_var();
                if (ret == nullptr)
                {
                    ll_inst->set_opcode(OpCodeEnum::RetVoid);
                }
                else if (hl_inst->get_var_src_eval_stack_idx() == 0)
                {
                    ll_inst->set_opcode(OpCodeEnum::RetNopShort);
                }
                else
                {
                    OpCodeEnum op = OpCodeEnum::Illegal;
                    switch (ret->reduce_type)
                    {
                    case metadata::RtArgOrLocOrFieldReduceType::I1:
                    case metadata::RtArgOrLocOrFieldReduceType::U1:
                    case metadata::RtArgOrLocOrFieldReduceType::I2:
                    case metadata::RtArgOrLocOrFieldReduceType::U2:
                    case metadata::RtArgOrLocOrFieldReduceType::I4:
                    case metadata::RtArgOrLocOrFieldReduceType::R4:
                        op = OpCodeEnum::RetI4;
                        break;
                    case metadata::RtArgOrLocOrFieldReduceType::I8:
                    case metadata::RtArgOrLocOrFieldReduceType::R8:
                        op = OpCodeEnum::RetI8;
                        break;
                    case metadata::RtArgOrLocOrFieldReduceType::I:
                    case metadata::RtArgOrLocOrFieldReduceType::Ref:
                        op = utils::Platform::select_arch(OpCodeEnum::RetI4, OpCodeEnum::RetI8);
                        break;
                    case metadata::RtArgOrLocOrFieldReduceType::Other:
                        ll_inst->set_size(ret->stack_object_size);
                        op = OpCodeEnum::RetAny;
                        break;
                    default:
                        RET_ERR(core::RtErr::ExecutionEngine);
                    }
                    ll_inst->set_opcode(op);
                }
                break;
            }

            case hl::OpCodeEnum::Br:
            {
                ll_inst->set_opcode(OpCodeEnum::Br);
                const hl::BasicBlock* target_hl_bb = hl_inst->get_branch_target();
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(BasicBlock*, target_ll_bb, translate_hl_basic_to_ll_basic(target_hl_bb));
                ll_inst->update_branch_target(target_ll_bb);
                break;
            }

            case hl::OpCodeEnum::BrTrue:
                RET_ERR_ON_FAIL(transform_brtruefalse(ll_inst, hl_inst, OpCodeEnum::BrTrueI4, OpCodeEnum::BrTrueI8));
                break;

            case hl::OpCodeEnum::BrFalse:
                RET_ERR_ON_FAIL(transform_brtruefalse(ll_inst, hl_inst, OpCodeEnum::BrFalseI4, OpCodeEnum::BrFalseI8));
                break;

            case hl::OpCodeEnum::Beq:
                RET_ERR_ON_FAIL(transform_condition_branch(ll_inst, hl_inst, OpCodeEnum::BeqI4, OpCodeEnum::BeqI8, OpCodeEnum::BeqR4, OpCodeEnum::BeqR8));
                break;

            case hl::OpCodeEnum::Bge:
                RET_ERR_ON_FAIL(transform_condition_branch(ll_inst, hl_inst, OpCodeEnum::BgeI4, OpCodeEnum::BgeI8, OpCodeEnum::BgeR4, OpCodeEnum::BgeR8));
                break;

            case hl::OpCodeEnum::Bgt:
                RET_ERR_ON_FAIL(transform_condition_branch(ll_inst, hl_inst, OpCodeEnum::BgtI4, OpCodeEnum::BgtI8, OpCodeEnum::BgtR4, OpCodeEnum::BgtR8));
                break;

            case hl::OpCodeEnum::Ble:
                RET_ERR_ON_FAIL(transform_condition_branch(ll_inst, hl_inst, OpCodeEnum::BleI4, OpCodeEnum::BleI8, OpCodeEnum::BleR4, OpCodeEnum::BleR8));
                break;

            case hl::OpCodeEnum::Blt:
                RET_ERR_ON_FAIL(transform_condition_branch(ll_inst, hl_inst, OpCodeEnum::BltI4, OpCodeEnum::BltI8, OpCodeEnum::BltR4, OpCodeEnum::BltR8));
                break;

            case hl::OpCodeEnum::BneUn:
                RET_ERR_ON_FAIL(
                    transform_condition_branch(ll_inst, hl_inst, OpCodeEnum::BneUnI4, OpCodeEnum::BneUnI8, OpCodeEnum::BneUnR4, OpCodeEnum::BneUnR8));
                break;

            case hl::OpCodeEnum::BgeUn:
                RET_ERR_ON_FAIL(
                    transform_condition_branch(ll_inst, hl_inst, OpCodeEnum::BgeUnI4, OpCodeEnum::BgeUnI8, OpCodeEnum::BgeUnR4, OpCodeEnum::BgeUnR8));
                break;

            case hl::OpCodeEnum::BgtUn:
                RET_ERR_ON_FAIL(
                    transform_condition_branch(ll_inst, hl_inst, OpCodeEnum::BgtUnI4, OpCodeEnum::BgtUnI8, OpCodeEnum::BgtUnR4, OpCodeEnum::BgtUnR8));
                break;

            case hl::OpCodeEnum::BleUn:
                RET_ERR_ON_FAIL(
                    transform_condition_branch(ll_inst, hl_inst, OpCodeEnum::BleUnI4, OpCodeEnum::BleUnI8, OpCodeEnum::BleUnR4, OpCodeEnum::BleUnR8));
                break;

            case hl::OpCodeEnum::BltUn:
                RET_ERR_ON_FAIL(
                    transform_condition_branch(ll_inst, hl_inst, OpCodeEnum::BltUnI4, OpCodeEnum::BltUnI8, OpCodeEnum::BltUnR4, OpCodeEnum::BltUnR8));
                break;

            case hl::OpCodeEnum::Switch:
            {
                ll_inst->set_opcode(OpCodeEnum::Switch);
                auto switch_targets = hl_inst->get_switch_targets();
                const hl::BasicBlock** target_hl_bbs = switch_targets.first;
                size_t target_count = switch_targets.second;
                const BasicBlock** target_ll_bbs_array = _mem_pool.calloc_any<const BasicBlock*>(target_count);
                for (size_t i = 0; i < target_count; ++i)
                {
                    const hl::BasicBlock* target_hl_bb = target_hl_bbs[i];
                    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(BasicBlock*, target_ll_bb, translate_hl_basic_to_ll_basic(target_hl_bb));
                    target_ll_bbs_array[i] = target_ll_bb;
                }
                ll_inst->update_switch_targets(target_ll_bbs_array, target_count);
                break;
            }

            case hl::OpCodeEnum::LdIndI1:
                ll_inst->set_opcode(OpCodeEnum::LdIndI1);
                break;

            case hl::OpCodeEnum::LdIndU1:
                ll_inst->set_opcode(OpCodeEnum::LdIndU1);
                break;

            case hl::OpCodeEnum::LdIndI2:
                ll_inst->set_opcode(hl_inst->contains_prefix_unaligned() ? OpCodeEnum::LdIndI2Unaligned : OpCodeEnum::LdIndI2);
                break;

            case hl::OpCodeEnum::LdIndU2:
                ll_inst->set_opcode(hl_inst->contains_prefix_unaligned() ? OpCodeEnum::LdIndU2Unaligned : OpCodeEnum::LdIndU2);
                break;

            case hl::OpCodeEnum::LdIndI4:
            case hl::OpCodeEnum::LdIndR4:
                ll_inst->set_opcode(hl_inst->contains_prefix_unaligned() ? OpCodeEnum::LdIndI4Unaligned : OpCodeEnum::LdIndI4);
                break;

            case hl::OpCodeEnum::LdIndI8:
            case hl::OpCodeEnum::LdIndR8:
                ll_inst->set_opcode(hl_inst->contains_prefix_unaligned() ? OpCodeEnum::LdIndI8Unaligned : OpCodeEnum::LdIndI8);
                break;

            case hl::OpCodeEnum::LdIndRef:
                ll_inst->set_opcode(hl_inst->contains_prefix_unaligned()
                                        ? utils::Platform::select_arch(OpCodeEnum::LdIndI4Unaligned, OpCodeEnum::LdIndI8Unaligned)
                                        : utils::Platform::select_arch(OpCodeEnum::LdIndI4, OpCodeEnum::LdIndI8));
                break;

            case hl::OpCodeEnum::StIndI1:
                ll_inst->set_opcode(OpCodeEnum::StIndI1);
                break;

            case hl::OpCodeEnum::StIndI2:
                ll_inst->set_opcode(hl_inst->contains_prefix_unaligned() ? OpCodeEnum::StIndI2Unaligned : OpCodeEnum::StIndI2);
                break;

            case hl::OpCodeEnum::StIndI4:
            case hl::OpCodeEnum::StIndR4:
                ll_inst->set_opcode(hl_inst->contains_prefix_unaligned() ? OpCodeEnum::StIndI4Unaligned : OpCodeEnum::StIndI4);
                break;

            case hl::OpCodeEnum::StIndI8:
            case hl::OpCodeEnum::StIndR8:
                ll_inst->set_opcode(hl_inst->contains_prefix_unaligned() ? OpCodeEnum::StIndI8Unaligned : OpCodeEnum::StIndI8);
                break;

            case hl::OpCodeEnum::StIndRef:
                ll_inst->set_opcode(hl_inst->contains_prefix_unaligned()
                                        ? utils::Platform::select_arch(OpCodeEnum::StIndI4Unaligned, OpCodeEnum::StIndI8Unaligned)
                                        : utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8));
                break;

            case hl::OpCodeEnum::Add:
                RET_ERR_ON_FAIL(transform_bin_arith_op(ll_inst, hl_inst, OpCodeEnum::AddI4, OpCodeEnum::AddI8, OpCodeEnum::AddR4, OpCodeEnum::AddR8));
                break;

            case hl::OpCodeEnum::Sub:
                RET_ERR_ON_FAIL(transform_bin_arith_op(ll_inst, hl_inst, OpCodeEnum::SubI4, OpCodeEnum::SubI8, OpCodeEnum::SubR4, OpCodeEnum::SubR8));
                break;

            case hl::OpCodeEnum::Mul:
                RET_ERR_ON_FAIL(transform_bin_arith_op(ll_inst, hl_inst, OpCodeEnum::MulI4, OpCodeEnum::MulI8, OpCodeEnum::MulR4, OpCodeEnum::MulR8));
                break;

            case hl::OpCodeEnum::Div:
                RET_ERR_ON_FAIL(transform_bin_arith_op(ll_inst, hl_inst, OpCodeEnum::DivI4, OpCodeEnum::DivI8, OpCodeEnum::DivR4, OpCodeEnum::DivR8));
                break;

            case hl::OpCodeEnum::Rem:
                RET_ERR_ON_FAIL(transform_bin_arith_op(ll_inst, hl_inst, OpCodeEnum::RemI4, OpCodeEnum::RemI8, OpCodeEnum::RemR4, OpCodeEnum::RemR8));
                break;

            case hl::OpCodeEnum::DivUn:
                RET_ERR_ON_FAIL(transform_bin_arith_op(ll_inst, hl_inst, OpCodeEnum::DivUnI4, OpCodeEnum::DivUnI8, OpCodeEnum::DivR4, OpCodeEnum::DivR8));
                break;

            case hl::OpCodeEnum::RemUn:
                RET_ERR_ON_FAIL(transform_bin_arith_op(ll_inst, hl_inst, OpCodeEnum::RemUnI4, OpCodeEnum::RemUnI8, OpCodeEnum::RemR4, OpCodeEnum::RemR8));
                break;

            case hl::OpCodeEnum::And:
                RET_ERR_ON_FAIL(transform_bin_bit_op(ll_inst, hl_inst, OpCodeEnum::AndI4, OpCodeEnum::AndI8));
                break;

            case hl::OpCodeEnum::Or:
                RET_ERR_ON_FAIL(transform_bin_bit_op(ll_inst, hl_inst, OpCodeEnum::OrI4, OpCodeEnum::OrI8));
                break;

            case hl::OpCodeEnum::Xor:
                RET_ERR_ON_FAIL(transform_bin_bit_op(ll_inst, hl_inst, OpCodeEnum::XorI4, OpCodeEnum::XorI8));
                break;

            case hl::OpCodeEnum::Shl:
                RET_ERR_ON_FAIL(transform_bin_bit_shift_op(ll_inst, hl_inst, OpCodeEnum::ShlI4, OpCodeEnum::ShlI8));
                break;

            case hl::OpCodeEnum::Shr:
                RET_ERR_ON_FAIL(transform_bin_bit_shift_op(ll_inst, hl_inst, OpCodeEnum::ShrI4, OpCodeEnum::ShrI8));
                break;

            case hl::OpCodeEnum::ShrUn:
                RET_ERR_ON_FAIL(transform_bin_bit_shift_op(ll_inst, hl_inst, OpCodeEnum::ShrUnI4, OpCodeEnum::ShrUnI8));
                break;

            case hl::OpCodeEnum::Neg:
            {
                const Variable* dst = hl_inst->get_var_dst();
                OpCodeEnum op = OpCodeEnum::Illegal;
                switch (dst->data_type)
                {
                case RtEvalStackDataType::I4:
                    op = OpCodeEnum::NegI4;
                    break;
                case RtEvalStackDataType::I8:
                    op = OpCodeEnum::NegI8;
                    break;
                case RtEvalStackDataType::R4:
                    op = OpCodeEnum::NegR4;
                    break;
                case RtEvalStackDataType::R8:
                    op = OpCodeEnum::NegR8;
                    break;
                case RtEvalStackDataType::RefOrPtr:
                    op = utils::Platform::select_arch(OpCodeEnum::NegI4, OpCodeEnum::NegI8);
                    break;
                default:
                    RET_ERR(core::RtErr::ExecutionEngine);
                }
                ll_inst->set_opcode(op);
                break;
            }

            case hl::OpCodeEnum::Not:
            {
                const Variable* dst = hl_inst->get_var_dst();
                OpCodeEnum op = OpCodeEnum::Illegal;
                switch (dst->data_type)
                {
                case RtEvalStackDataType::I4:
                    op = OpCodeEnum::NotI4;
                    break;
                case RtEvalStackDataType::I8:
                    op = OpCodeEnum::NotI8;
                    break;
                case RtEvalStackDataType::RefOrPtr:
                    op = utils::Platform::select_arch(OpCodeEnum::NotI4, OpCodeEnum::NotI8);
                    break;
                default:
                    RET_ERR(core::RtErr::ExecutionEngine);
                }
                ll_inst->set_opcode(op);
                break;
            }

            case hl::OpCodeEnum::AddOvf:
                RET_ERR_ON_FAIL(transform_bin_arith_ovf_op(ll_inst, hl_inst, OpCodeEnum::AddOvfI4, OpCodeEnum::AddOvfI8));
                break;

            case hl::OpCodeEnum::AddOvfUn:
                RET_ERR_ON_FAIL(transform_bin_arith_ovf_op(ll_inst, hl_inst, OpCodeEnum::AddOvfUnI4, OpCodeEnum::AddOvfUnI8));
                break;

            case hl::OpCodeEnum::SubOvf:
                RET_ERR_ON_FAIL(transform_bin_arith_ovf_op(ll_inst, hl_inst, OpCodeEnum::SubOvfI4, OpCodeEnum::SubOvfI8));
                break;

            case hl::OpCodeEnum::SubOvfUn:
                RET_ERR_ON_FAIL(transform_bin_arith_ovf_op(ll_inst, hl_inst, OpCodeEnum::SubOvfUnI4, OpCodeEnum::SubOvfUnI8));
                break;

            case hl::OpCodeEnum::MulOvf:
                RET_ERR_ON_FAIL(transform_bin_arith_ovf_op(ll_inst, hl_inst, OpCodeEnum::MulOvfI4, OpCodeEnum::MulOvfI8));
                break;

            case hl::OpCodeEnum::MulOvfUn:
                RET_ERR_ON_FAIL(transform_bin_arith_ovf_op(ll_inst, hl_inst, OpCodeEnum::MulOvfUnI4, OpCodeEnum::MulOvfUnI8));
                break;

            case hl::OpCodeEnum::ConvI1:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvI1I4, OpCodeEnum::ConvI1I8, OpCodeEnum::ConvI1R4, OpCodeEnum::ConvI1R8));
                break;

            case hl::OpCodeEnum::ConvU1:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvU1I4, OpCodeEnum::ConvU1I8, OpCodeEnum::ConvU1R4, OpCodeEnum::ConvU1R8));
                break;

            case hl::OpCodeEnum::ConvI2:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvI2I4, OpCodeEnum::ConvI2I8, OpCodeEnum::ConvI2R4, OpCodeEnum::ConvI2R8));
                break;

            case hl::OpCodeEnum::ConvU2:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvU2I4, OpCodeEnum::ConvU2I8, OpCodeEnum::ConvU2R4, OpCodeEnum::ConvU2R8));
                break;

            case hl::OpCodeEnum::ConvI4:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::Nop, OpCodeEnum::ConvI4I8, OpCodeEnum::ConvI4R4, OpCodeEnum::ConvI4R8));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvU4:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::Nop, OpCodeEnum::ConvU4I8, OpCodeEnum::ConvU4R4, OpCodeEnum::ConvU4R8));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvI8:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvI8I4, OpCodeEnum::Nop, OpCodeEnum::ConvI8R4, OpCodeEnum::ConvI8R8));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvU8:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvU8I4, OpCodeEnum::Nop, OpCodeEnum::ConvU8R4, OpCodeEnum::ConvU8R8));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvI:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvI8I4),
                                               utils::Platform::select_arch(OpCodeEnum::ConvI4I8, OpCodeEnum::Nop),
                                               utils::Platform::select_arch(OpCodeEnum::ConvI4R4, OpCodeEnum::ConvI8R4),
                                               utils::Platform::select_arch(OpCodeEnum::ConvI4R8, OpCodeEnum::ConvI8R8)));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvU:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvU8I4),
                                               utils::Platform::select_arch(OpCodeEnum::ConvU4I8, OpCodeEnum::Nop),
                                               utils::Platform::select_arch(OpCodeEnum::ConvU4R4, OpCodeEnum::ConvU8R4),
                                               utils::Platform::select_arch(OpCodeEnum::ConvU4R8, OpCodeEnum::ConvU8R8)));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvR4:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvR4I4, OpCodeEnum::ConvR4I8, OpCodeEnum::Nop, OpCodeEnum::ConvR4R8));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvR8:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvR8I4, OpCodeEnum::ConvR8I8, OpCodeEnum::ConvR8R4, OpCodeEnum::Nop));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvOvfI1:
                RET_ERR_ON_FAIL(
                    transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvOvfI1I4, OpCodeEnum::ConvOvfI1I8, OpCodeEnum::ConvOvfI1R4, OpCodeEnum::ConvOvfI1R8));
                break;

            case hl::OpCodeEnum::ConvOvfU1:
                RET_ERR_ON_FAIL(
                    transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvOvfU1I4, OpCodeEnum::ConvOvfU1I8, OpCodeEnum::ConvOvfU1R4, OpCodeEnum::ConvOvfU1R8));
                break;

            case hl::OpCodeEnum::ConvOvfI2:
                RET_ERR_ON_FAIL(
                    transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvOvfI2I4, OpCodeEnum::ConvOvfI2I8, OpCodeEnum::ConvOvfI2R4, OpCodeEnum::ConvOvfI2R8));
                break;

            case hl::OpCodeEnum::ConvOvfU2:
                RET_ERR_ON_FAIL(
                    transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvOvfU2I4, OpCodeEnum::ConvOvfU2I8, OpCodeEnum::ConvOvfU2R4, OpCodeEnum::ConvOvfU2R8));
                break;

            case hl::OpCodeEnum::ConvOvfI4:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::Nop, OpCodeEnum::ConvOvfI4I8, OpCodeEnum::ConvOvfI4R4, OpCodeEnum::ConvOvfI4R8));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvOvfU4:
                RET_ERR_ON_FAIL(
                    transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvOvfU4I4, OpCodeEnum::ConvOvfU4I8, OpCodeEnum::ConvOvfU4R4, OpCodeEnum::ConvOvfU4R8));
                break;

            case hl::OpCodeEnum::ConvOvfI8:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvI8I4, OpCodeEnum::Nop, OpCodeEnum::ConvOvfI8R4, OpCodeEnum::ConvOvfI8R8));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvOvfU8:
                RET_ERR_ON_FAIL(
                    transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvOvfU8I4, OpCodeEnum::ConvOvfU8I8, OpCodeEnum::ConvOvfU8R4, OpCodeEnum::ConvOvfU8R8));
                break;

            case hl::OpCodeEnum::ConvOvfI:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvI8I4),
                                               utils::Platform::select_arch(OpCodeEnum::ConvOvfI4I8, OpCodeEnum::Nop),
                                               utils::Platform::select_arch(OpCodeEnum::ConvOvfI4R4, OpCodeEnum::ConvOvfI8R4),
                                               utils::Platform::select_arch(OpCodeEnum::ConvOvfI4R8, OpCodeEnum::ConvOvfI8R8)));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvOvfU:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, utils::Platform::select_arch(OpCodeEnum::ConvOvfU4I4, OpCodeEnum::ConvOvfU8I4),
                                               utils::Platform::select_arch(OpCodeEnum::ConvOvfU4I8, OpCodeEnum::ConvOvfU8I8),
                                               utils::Platform::select_arch(OpCodeEnum::ConvOvfU4R4, OpCodeEnum::ConvOvfU8R4),
                                               utils::Platform::select_arch(OpCodeEnum::ConvOvfU4R8, OpCodeEnum::ConvOvfU8R8)));
                break;

            case hl::OpCodeEnum::ConvOvfI1Un:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvOvfI1UnI4, OpCodeEnum::ConvOvfI1UnI8, OpCodeEnum::ConvOvfI1UnR4,
                                               OpCodeEnum::ConvOvfI1UnR8));
                break;

            case hl::OpCodeEnum::ConvOvfU1Un:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvOvfU1UnI4, OpCodeEnum::ConvOvfU1UnI8, OpCodeEnum::ConvOvfU1UnR4,
                                               OpCodeEnum::ConvOvfU1UnR8));
                break;

            case hl::OpCodeEnum::ConvOvfI2Un:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvOvfI2UnI4, OpCodeEnum::ConvOvfI2UnI8, OpCodeEnum::ConvOvfI2UnR4,
                                               OpCodeEnum::ConvOvfI2UnR8));
                break;

            case hl::OpCodeEnum::ConvOvfU2Un:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvOvfU2UnI4, OpCodeEnum::ConvOvfU2UnI8, OpCodeEnum::ConvOvfU2UnR4,
                                               OpCodeEnum::ConvOvfU2UnR8));
                break;

            case hl::OpCodeEnum::ConvOvfI4Un:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvOvfI4UnI4, OpCodeEnum::ConvOvfI4UnI8, OpCodeEnum::ConvOvfI4UnR4,
                                               OpCodeEnum::ConvOvfI4UnR8));
                break;

            case hl::OpCodeEnum::ConvOvfU4Un:
                RET_ERR_ON_FAIL(
                    transform_conv(ll_inst, hl_inst, OpCodeEnum::Nop, OpCodeEnum::ConvOvfU4UnI8, OpCodeEnum::ConvOvfU4UnR4, OpCodeEnum::ConvOvfU4UnR8));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvOvfI8Un:
                RET_ERR_ON_FAIL(
                    transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvI8U4, OpCodeEnum::ConvOvfI8UnI8, OpCodeEnum::ConvOvfI8UnR4, OpCodeEnum::ConvOvfI8UnR8));
                break;

            case hl::OpCodeEnum::ConvOvfU8Un:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, OpCodeEnum::ConvI8U4, OpCodeEnum::Nop, OpCodeEnum::ConvOvfU8UnR4, OpCodeEnum::ConvOvfU8UnR8));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::ConvOvfIUn:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, utils::Platform::select_arch(OpCodeEnum::ConvOvfI4UnI4, OpCodeEnum::ConvI8U4),
                                               utils::Platform::select_arch(OpCodeEnum::ConvOvfI4UnI8, OpCodeEnum::ConvOvfI8UnI8),
                                               utils::Platform::select_arch(OpCodeEnum::ConvOvfI4UnR4, OpCodeEnum::ConvOvfI8UnR4),
                                               utils::Platform::select_arch(OpCodeEnum::ConvOvfI4UnR8, OpCodeEnum::ConvOvfI8UnR8)));
                break;

            case hl::OpCodeEnum::ConvOvfUUn:
                RET_ERR_ON_FAIL(transform_conv(ll_inst, hl_inst, utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvI8U4),
                                               utils::Platform::select_arch(OpCodeEnum::ConvOvfU4UnI8, OpCodeEnum::Nop),
                                               utils::Platform::select_arch(OpCodeEnum::ConvOvfU4UnR4, OpCodeEnum::ConvOvfU8UnR4),
                                               utils::Platform::select_arch(OpCodeEnum::ConvOvfU4UnR8, OpCodeEnum::ConvOvfU8UnR8)));
                add = ll_inst->get_opcode() != OpCodeEnum::Nop;
                break;

            case hl::OpCodeEnum::Ceq:
                RET_ERR_ON_FAIL(transform_cmp_op(ll_inst, hl_inst, OpCodeEnum::CeqI4, OpCodeEnum::CeqI8, OpCodeEnum::CeqR4, OpCodeEnum::CeqR8));
                break;

            case hl::OpCodeEnum::Cgt:
                RET_ERR_ON_FAIL(transform_cmp_op(ll_inst, hl_inst, OpCodeEnum::CgtI4, OpCodeEnum::CgtI8, OpCodeEnum::CgtR4, OpCodeEnum::CgtR8));
                break;

            case hl::OpCodeEnum::Clt:
                RET_ERR_ON_FAIL(transform_cmp_op(ll_inst, hl_inst, OpCodeEnum::CltI4, OpCodeEnum::CltI8, OpCodeEnum::CltR4, OpCodeEnum::CltR8));
                break;

            case hl::OpCodeEnum::CgtUn:
                RET_ERR_ON_FAIL(transform_cmp_op(ll_inst, hl_inst, OpCodeEnum::CgtUnI4, OpCodeEnum::CgtUnI8, OpCodeEnum::CgtUnR4, OpCodeEnum::CgtUnR8));
                break;

            case hl::OpCodeEnum::CltUn:
                RET_ERR_ON_FAIL(transform_cmp_op(ll_inst, hl_inst, OpCodeEnum::CltUnI4, OpCodeEnum::CltUnI8, OpCodeEnum::CltUnR4, OpCodeEnum::CltUnR8));
                break;

            case hl::OpCodeEnum::InitObjI1:
                ll_inst->set_opcode(OpCodeEnum::InitObjI1);
                break;

            case hl::OpCodeEnum::InitObjI2:
                ll_inst->set_opcode(OpCodeEnum::InitObjI2);
                break;

            case hl::OpCodeEnum::InitObjI4:
                ll_inst->set_opcode(OpCodeEnum::InitObjI4);
                break;

            case hl::OpCodeEnum::InitObjI8:
                ll_inst->set_opcode(OpCodeEnum::InitObjI8);
                break;

            case hl::OpCodeEnum::InitObjRef:
                ll_inst->set_opcode(utils::Platform::select_arch(OpCodeEnum::InitObjI4, OpCodeEnum::InitObjI8));
                break;

            case hl::OpCodeEnum::InitObjAny:
                ll_inst->set_opcode(OpCodeEnum::InitObjAny);
                break;

            case hl::OpCodeEnum::CpObjI1:
                ll_inst->set_opcode(OpCodeEnum::CpObjI1);
                break;

            case hl::OpCodeEnum::CpObjI2:
                ll_inst->set_opcode(OpCodeEnum::CpObjI2);
                break;

            case hl::OpCodeEnum::CpObjI4:
                ll_inst->set_opcode(OpCodeEnum::CpObjI4);
                break;

            case hl::OpCodeEnum::CpObjI8:
                ll_inst->set_opcode(OpCodeEnum::CpObjI8);
                break;

            case hl::OpCodeEnum::CpObjRef:
                ll_inst->set_opcode(utils::Platform::select_arch(OpCodeEnum::CpObjI4, OpCodeEnum::CpObjI8));
                break;

            case hl::OpCodeEnum::CpObjAny:
                ll_inst->set_opcode(OpCodeEnum::CpObjAny);
                break;

            case hl::OpCodeEnum::LdObjAny:
                ll_inst->set_opcode(OpCodeEnum::LdObjAny);
                break;

            case hl::OpCodeEnum::StObjAny:
                ll_inst->set_opcode(OpCodeEnum::StObjAny);
                break;

            case hl::OpCodeEnum::CastClass:
                ll_inst->set_opcode(OpCodeEnum::CastClass);
                setup_inst_klass(ll_inst, hl_inst);
                assert(ll_inst->get_var_src_eval_stack_idx() == ll_inst->get_var_dst_eval_stack_idx());
                break;

            case hl::OpCodeEnum::IsInst:
                ll_inst->set_opcode(OpCodeEnum::IsInst);
                setup_inst_klass(ll_inst, hl_inst);
                break;

            case hl::OpCodeEnum::Box:
                ll_inst->set_opcode(OpCodeEnum::Box);
                setup_inst_klass(ll_inst, hl_inst);
                break;

            case hl::OpCodeEnum::BoxRefInplace:
                ll_inst->set_opcode(OpCodeEnum::BoxRefInplace);
                setup_inst_klass(ll_inst, hl_inst);
                break;

            case hl::OpCodeEnum::Unbox:
                ll_inst->set_opcode(OpCodeEnum::Unbox);
                setup_inst_klass(ll_inst, hl_inst);
                break;

            case hl::OpCodeEnum::UnboxAny:
                ll_inst->set_opcode(OpCodeEnum::UnboxAny);
                setup_inst_klass(ll_inst, hl_inst);
                break;

            case hl::OpCodeEnum::NewArr:
            {
                ll_inst->set_opcode(OpCodeEnum::NewArr);
                metadata::RtClass* ele_klass = hl_inst->get_class();
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, arr_klass, vm::ArrayClass::get_szarray_class_from_element_class(ele_klass));
                setup_inst_resolved_data(ll_inst, arr_klass);
                break;
            }

            case hl::OpCodeEnum::LdLen:
                ll_inst->set_opcode(OpCodeEnum::LdLen);
                break;

            case hl::OpCodeEnum::Ldelema:
                ll_inst->set_opcode(hl_inst->contains_prefix_readonly() ? OpCodeEnum::LdelemaReadOnly : OpCodeEnum::Ldelema);
                setup_inst_klass(ll_inst, hl_inst);
                break;

            case hl::OpCodeEnum::LdelemI1:
                ll_inst->set_opcode(OpCodeEnum::LdelemI1);
                break;

            case hl::OpCodeEnum::LdelemU1:
                ll_inst->set_opcode(OpCodeEnum::LdelemU1);
                break;

            case hl::OpCodeEnum::LdelemI2:
                ll_inst->set_opcode(OpCodeEnum::LdelemI2);
                break;

            case hl::OpCodeEnum::LdelemU2:
                ll_inst->set_opcode(OpCodeEnum::LdelemU2);
                break;

            case hl::OpCodeEnum::LdelemI4:
                ll_inst->set_opcode(OpCodeEnum::LdelemI4);
                break;

            case hl::OpCodeEnum::LdelemI8:
                ll_inst->set_opcode(OpCodeEnum::LdelemI8);
                break;

            case hl::OpCodeEnum::LdelemI:
                ll_inst->set_opcode(OpCodeEnum::LdelemI);
                break;

            case hl::OpCodeEnum::LdelemR4:
                ll_inst->set_opcode(OpCodeEnum::LdelemR4);
                break;

            case hl::OpCodeEnum::LdelemR8:
                ll_inst->set_opcode(OpCodeEnum::LdelemR8);
                break;

            case hl::OpCodeEnum::LdelemRef:
                ll_inst->set_opcode(OpCodeEnum::LdelemRef);
                break;

            case hl::OpCodeEnum::LdelemAny:
            {
                metadata::RtClass* ele_klass = hl_inst->get_class();
                ll_inst->set_opcode(vm::Class::is_reference_type(ele_klass) ? OpCodeEnum::LdelemAnyRef : OpCodeEnum::LdelemAnyVal);
                ll_inst->set_element_size(vm::Class::get_stack_location_size(ele_klass));
                setup_inst_klass(ll_inst, hl_inst);
                break;
            }

            case hl::OpCodeEnum::StelemI1:
                ll_inst->set_opcode(OpCodeEnum::StelemI1);
                break;

            case hl::OpCodeEnum::StelemI2:
                ll_inst->set_opcode(OpCodeEnum::StelemI2);
                break;

            case hl::OpCodeEnum::StelemI4:
                ll_inst->set_opcode(OpCodeEnum::StelemI4);
                break;

            case hl::OpCodeEnum::StelemI8:
                ll_inst->set_opcode(OpCodeEnum::StelemI8);
                break;

            case hl::OpCodeEnum::StelemI:
                ll_inst->set_opcode(OpCodeEnum::StelemI);
                break;

            case hl::OpCodeEnum::StelemR4:
                ll_inst->set_opcode(OpCodeEnum::StelemR4);
                break;

            case hl::OpCodeEnum::StelemR8:
                ll_inst->set_opcode(OpCodeEnum::StelemR8);
                break;

            case hl::OpCodeEnum::StelemRef:
                ll_inst->set_opcode(OpCodeEnum::StelemRef);
                setup_inst_klass(ll_inst, hl_inst);
                break;
            case hl::OpCodeEnum::StelemAny:
            {
                metadata::RtClass* ele_klass = hl_inst->get_class();
                ll_inst->set_opcode(vm::Class::is_reference_type(ele_klass) ? OpCodeEnum::StelemAnyRef : OpCodeEnum::StelemAnyVal);
                ll_inst->set_element_size(vm::Class::get_stack_location_size(ele_klass));
                setup_inst_klass(ll_inst, hl_inst);
                break;
            }
            case hl::OpCodeEnum::MkRefAny:
                ll_inst->set_opcode(OpCodeEnum::MkRefAny);
                setup_inst_klass(ll_inst, hl_inst);
                break;

            case hl::OpCodeEnum::RefAnyVal:
                ll_inst->set_opcode(OpCodeEnum::RefAnyVal);
                setup_inst_klass(ll_inst, hl_inst);
                break;

            case hl::OpCodeEnum::RefAnyType:
                ll_inst->set_opcode(OpCodeEnum::RefAnyType);
                break;

            case hl::OpCodeEnum::LdToken:
            {
                ll_inst->set_opcode(OpCodeEnum::LdToken);
                setup_inst_resolved_data(ll_inst, (const void*)hl_inst->get_runtime_handle().get_handle_without_type());
                break;
            }
            case hl::OpCodeEnum::Ckfinite:
            {
                const Variable* dst = hl_inst->get_var_dst();
                OpCodeEnum op = OpCodeEnum::Illegal;
                switch (dst->data_type)
                {
                case RtEvalStackDataType::R8:
                    op = OpCodeEnum::CkfiniteR8;
                    break;
                case RtEvalStackDataType::R4:
                    op = OpCodeEnum::CkfiniteR4;
                    break;
                default:
                    RET_ERR(core::RtErr::ExecutionEngine);
                }
                ll_inst->set_opcode(op);
                break;
            }
            case hl::OpCodeEnum::LocAlloc:
            {
                ll_inst->set_opcode(OpCodeEnum::LocAlloc);
                break;
            }
            case hl::OpCodeEnum::InitBlk:
            {
                ll_inst->set_opcode(OpCodeEnum::InitBlk);
                break;
            }
            case hl::OpCodeEnum::CpBlk:
            {
                ll_inst->set_opcode(OpCodeEnum::CpBlk);
                break;
            }
            case hl::OpCodeEnum::Ldfld:
            {
                size_t field_offset = ll_inst->get_field_offset();
                bool use_large_addressing = field_offset > UINT16_MAX;
                const metadata::RtFieldInfo* field = hl_inst->get_field();
                const Variable* dst = hl_inst->get_var_dst();
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(field->type_sig));
                OpCodeEnum op = OpCodeEnum::Illegal;
                switch (type_and_size.reduce_type)
                {
                case metadata::RtArgOrLocOrFieldReduceType::I1:
                    op = use_large_addressing ? OpCodeEnum::LdfldI1Large : OpCodeEnum::LdfldI1;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::U1:
                    op = use_large_addressing ? OpCodeEnum::LdfldU1Large : OpCodeEnum::LdfldU1;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I2:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = OpCodeEnum::LdfldI2Unaligned;
                    }
                    else
                    {
                        op = use_large_addressing ? OpCodeEnum::LdfldI2Large : OpCodeEnum::LdfldI2;
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::U2:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = OpCodeEnum::LdfldU2Unaligned;
                    }
                    else
                    {
                        op = use_large_addressing ? OpCodeEnum::LdfldU2Large : OpCodeEnum::LdfldU2;
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I4:
                case metadata::RtArgOrLocOrFieldReduceType::R4:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = OpCodeEnum::LdfldI4Unaligned;
                    }
                    else
                    {
                        op = use_large_addressing ? OpCodeEnum::LdfldI4Large : OpCodeEnum::LdfldI4;
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I8:
                case metadata::RtArgOrLocOrFieldReduceType::R8:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = OpCodeEnum::LdfldI8Unaligned;
                    }
                    else
                    {
                        op = use_large_addressing ? OpCodeEnum::LdfldI8Large : OpCodeEnum::LdfldI8;
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I:
                case metadata::RtArgOrLocOrFieldReduceType::Ref:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = utils::Platform::select_arch(OpCodeEnum::LdfldI4Unaligned, OpCodeEnum::LdfldI8Unaligned);
                    }
                    else
                    {
                        op = use_large_addressing ? utils::Platform::select_arch(OpCodeEnum::LdfldI4Large, OpCodeEnum::LdfldI8Large)
                                                  : utils::Platform::select_arch(OpCodeEnum::LdfldI4, OpCodeEnum::LdfldI8);
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::Other:
                    op = use_large_addressing ? OpCodeEnum::LdfldAnyLarge : OpCodeEnum::LdfldAny;
                    break;
                default:
                    RETURN_NOT_IMPLEMENTED_ERROR();
                }
                ll_inst->set_opcode(op);
                break;
            }
            case hl::OpCodeEnum::Ldvfld:
            {
                size_t field_offset = ll_inst->get_field_offset();
                bool use_large_addressing = field_offset > UINT16_MAX;
                const metadata::RtFieldInfo* field = hl_inst->get_field();
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(field->type_sig));
                OpCodeEnum op = OpCodeEnum::Illegal;
                switch (type_and_size.reduce_type)
                {
                case metadata::RtArgOrLocOrFieldReduceType::I1:
                    op = use_large_addressing ? OpCodeEnum::LdvfldI1Large : OpCodeEnum::LdvfldI1;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::U1:
                    op = use_large_addressing ? OpCodeEnum::LdvfldU1Large : OpCodeEnum::LdvfldU1;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I2:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = OpCodeEnum::LdvfldI2Unaligned;
                    }
                    else
                    {
                        op = use_large_addressing ? OpCodeEnum::LdvfldI2Large : OpCodeEnum::LdvfldI2;
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::U2:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = OpCodeEnum::LdvfldU2Unaligned;
                    }
                    else
                    {
                        op = use_large_addressing ? OpCodeEnum::LdvfldU2Large : OpCodeEnum::LdvfldU2;
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I4:
                case metadata::RtArgOrLocOrFieldReduceType::R4:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = OpCodeEnum::LdvfldI4Unaligned;
                    }
                    else
                    {
                        op = use_large_addressing ? OpCodeEnum::LdvfldI4Large : OpCodeEnum::LdvfldI4;
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I8:
                case metadata::RtArgOrLocOrFieldReduceType::R8:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = OpCodeEnum::LdvfldI8Unaligned;
                    }
                    else
                    {
                        op = use_large_addressing ? OpCodeEnum::LdvfldI8Large : OpCodeEnum::LdvfldI8;
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I:
                case metadata::RtArgOrLocOrFieldReduceType::Ref:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = utils::Platform::select_arch(OpCodeEnum::LdvfldI4Unaligned, OpCodeEnum::LdvfldI8Unaligned);
                    }
                    else
                    {
                        op = use_large_addressing ? utils::Platform::select_arch(OpCodeEnum::LdvfldI4Large, OpCodeEnum::LdvfldI8Large)
                                                  : utils::Platform::select_arch(OpCodeEnum::LdvfldI4, OpCodeEnum::LdvfldI8);
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::Other:
                    op = use_large_addressing ? OpCodeEnum::LdvfldAnyLarge : OpCodeEnum::LdvfldAny;
                    break;
                default:
                    RETURN_NOT_IMPLEMENTED_ERROR();
                }

                if (ll_inst->get_field_offset() == 0 && ll_inst->get_var_src_eval_stack_idx() == ll_inst->get_var_dst_eval_stack_idx())
                {
                    switch (op)
                    {
                    case OpCodeEnum::LdvfldI1:
                        op = OpCodeEnum::ConvI1I4;
                        break;
                    case OpCodeEnum::LdvfldU1:
                        op = OpCodeEnum::ConvU1I4;
                        break;
                    case OpCodeEnum::LdvfldI2:
                        op = OpCodeEnum::ConvI2I4;
                        break;
                    case OpCodeEnum::LdvfldU2:
                        op = OpCodeEnum::ConvU2I4;
                        break;
                    case OpCodeEnum::LdvfldI4:
                    case OpCodeEnum::LdvfldI8:
                    case OpCodeEnum::LdvfldAny:
                        op = OpCodeEnum::Nop;
                        break;
                    default:
                        break;
                    }
                }

                ll_inst->set_opcode(op);
                break;
            }
            case hl::OpCodeEnum::Ldflda:
            {
                size_t field_offset = ll_inst->get_field_offset();
                bool use_large_addressing = field_offset > UINT16_MAX;
                ll_inst->set_opcode(use_large_addressing ? OpCodeEnum::LdfldaLarge : OpCodeEnum::Ldflda);
                break;
            }
            case hl::OpCodeEnum::Stfld:
            {
                size_t field_offset = ll_inst->get_field_offset();
                bool use_large_addressing = field_offset > UINT16_MAX;
                const metadata::RtFieldInfo* field = hl_inst->get_field();
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(field->type_sig));
                OpCodeEnum op = OpCodeEnum::Illegal;
                switch (type_and_size.reduce_type)
                {
                case metadata::RtArgOrLocOrFieldReduceType::I1:
                case metadata::RtArgOrLocOrFieldReduceType::U1:
                    op = OpCodeEnum::StfldI1;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I2:
                case metadata::RtArgOrLocOrFieldReduceType::U2:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = OpCodeEnum::StfldI2Unaligned;
                    }
                    else
                    {
                        op = use_large_addressing ? OpCodeEnum::StfldI2Large : OpCodeEnum::StfldI2;
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I4:
                case metadata::RtArgOrLocOrFieldReduceType::R4:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = OpCodeEnum::StfldI4Unaligned;
                    }
                    else
                    {
                        op = use_large_addressing ? OpCodeEnum::StfldI4Large : OpCodeEnum::StfldI4;
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I8:
                case metadata::RtArgOrLocOrFieldReduceType::R8:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = OpCodeEnum::StfldI8Unaligned;
                    }
                    else
                    {
                        op = use_large_addressing ? OpCodeEnum::StfldI8Large : OpCodeEnum::StfldI8;
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I:
                case metadata::RtArgOrLocOrFieldReduceType::Ref:
                    if (hl_inst->contains_prefix_unaligned())
                    {
                        op = utils::Platform::select_arch(OpCodeEnum::StfldI4Unaligned, OpCodeEnum::StfldI8Unaligned);
                    }
                    else
                    {
                        op = use_large_addressing ? utils::Platform::select_arch(OpCodeEnum::StfldI4Large, OpCodeEnum::StfldI8Large)
                                                  : utils::Platform::select_arch(OpCodeEnum::StfldI4, OpCodeEnum::StfldI8);
                    }
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::Other:
                    op = use_large_addressing ? OpCodeEnum::StfldAnyLarge : OpCodeEnum::StfldAny;
                    break;
                default:
                    RETURN_NOT_IMPLEMENTED_ERROR();
                }
                ll_inst->set_opcode(op);
                break;
            }
            case hl::OpCodeEnum::Ldsfld:
            {
                const metadata::RtFieldInfo* field = hl_inst->get_field();
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(field->type_sig));
                OpCodeEnum op = OpCodeEnum::Illegal;
                switch (type_and_size.reduce_type)
                {
                case metadata::RtArgOrLocOrFieldReduceType::I1:
                    op = OpCodeEnum::LdsfldI1;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::U1:
                    op = OpCodeEnum::LdsfldU1;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I2:
                    op = OpCodeEnum::LdsfldI2;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::U2:
                    op = OpCodeEnum::LdsfldU2;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I4:
                case metadata::RtArgOrLocOrFieldReduceType::R4:
                    op = OpCodeEnum::LdsfldI4;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I8:
                case metadata::RtArgOrLocOrFieldReduceType::R8:
                    op = OpCodeEnum::LdsfldI8;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I:
                case metadata::RtArgOrLocOrFieldReduceType::Ref:
                    op = utils::Platform::select_arch(OpCodeEnum::LdsfldI4, OpCodeEnum::LdsfldI8);
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::Other:
                    op = OpCodeEnum::LdsfldAny;
                    break;
                default:
                    RETURN_NOT_IMPLEMENTED_ERROR();
                }
                ll_inst->set_opcode(op);
                setup_inst_resolved_data(ll_inst, field);
                break;
            }
            case hl::OpCodeEnum::Ldsflda:
            {
                const metadata::RtFieldInfo* field = hl_inst->get_field();
                if (vm::Field::is_static_rva(field))
                {
                    ll_inst->set_opcode(OpCodeEnum::LdsfldRvaData);
                    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const uint8_t*, rva_data, vm::Field::get_field_rva_data(field));
                    setup_inst_resolved_data(ll_inst, rva_data);
                }
                else
                {
                    ll_inst->set_opcode(OpCodeEnum::Ldsflda);
                    setup_inst_resolved_data(ll_inst, field);
                }
                break;
            }
            case hl::OpCodeEnum::Stsfld:
            {
                const metadata::RtFieldInfo* field = hl_inst->get_field();
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(ReduceTypeAndSize, type_and_size, InterpDefs::get_reduce_type_and_size_by_typesig(field->type_sig));
                OpCodeEnum op = OpCodeEnum::Illegal;
                switch (type_and_size.reduce_type)
                {
                case metadata::RtArgOrLocOrFieldReduceType::I1:
                case metadata::RtArgOrLocOrFieldReduceType::U1:
                    op = OpCodeEnum::StsfldI1;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I2:
                case metadata::RtArgOrLocOrFieldReduceType::U2:
                    op = OpCodeEnum::StsfldI2;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I4:
                case metadata::RtArgOrLocOrFieldReduceType::R4:
                    op = OpCodeEnum::StsfldI4;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I8:
                case metadata::RtArgOrLocOrFieldReduceType::R8:
                    op = OpCodeEnum::StsfldI8;
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::I:
                case metadata::RtArgOrLocOrFieldReduceType::Ref:
                    op = utils::Platform::select_arch(OpCodeEnum::StsfldI4, OpCodeEnum::StsfldI8);
                    break;
                case metadata::RtArgOrLocOrFieldReduceType::Other:
                    op = OpCodeEnum::StsfldAny;
                    break;
                default:
                    RETURN_NOT_IMPLEMENTED_ERROR();
                }
                ll_inst->set_opcode(op);
                setup_inst_resolved_data(ll_inst, field);
                break;
            }
            case hl::OpCodeEnum::Call:
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_call_methods(ll_inst, hl_inst));
                if (!handled)
                {
                    ll_inst->set_opcode(OpCodeEnum::CallInterp);
                    setup_inst_method(ll_inst, hl_inst);
                }
                break;
            }
            case hl::OpCodeEnum::CallVirt:
                ll_inst->set_opcode(OpCodeEnum::CallVirtInterp);
                setup_inst_method(ll_inst, hl_inst);
                break;
            case hl::OpCodeEnum::CallInternalCall:
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_call_methods(ll_inst, hl_inst));
                if (!handled)
                {
                    ll_inst->set_opcode(OpCodeEnum::CallInternalCall);
                    setup_inst_method(ll_inst, hl_inst);
                }
                break;
            }
            case hl::OpCodeEnum::CallIntrinsic:
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_call_methods(ll_inst, hl_inst));
                if (!handled)
                {
                    ll_inst->set_opcode(OpCodeEnum::CallIntrinsic);
                    setup_inst_method(ll_inst, hl_inst);
                }
                break;
            }
            case hl::OpCodeEnum::CallPInvoke:
            {
                ll_inst->set_opcode(OpCodeEnum::CallPInvoke);
                setup_inst_method(ll_inst, hl_inst);
                break;
            }
            case hl::OpCodeEnum::CallAot:
            {
                ll_inst->set_opcode(OpCodeEnum::CallAot);
                setup_inst_method(ll_inst, hl_inst);
                break;
            }
            case hl::OpCodeEnum::CallRuntimeImplemented:
            {
                ll_inst->set_opcode(OpCodeEnum::CallRuntimeImplemented);
                setup_inst_method(ll_inst, hl_inst);
                break;
            }
            case hl::OpCodeEnum::Calli:
            {
                ll_inst->set_opcode(OpCodeEnum::CalliInterp);
                setup_inst_resolved_data(ll_inst, hl_inst->get_method_sig());
                // ll_inst->set_frame_base(hl_inst->get_frame_base());
                //  method_idx stores in arg3, it has been setup on start.
                break;
            }
            case hl::OpCodeEnum::NewObj:
            {
                const metadata::RtMethodInfo* method = hl_inst->get_method();
                OpCodeEnum op = vm::Class::is_value_type(method->parent) ? OpCodeEnum::NewValueTypeInterp : OpCodeEnum::NewObjInterp;
                ll_inst->set_opcode(op);
                setup_inst_method(ll_inst, hl_inst);
                break;
            }
            case hl::OpCodeEnum::NewObjInternalCall:
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_newobj_methods(ll_inst, hl_inst));
                if (!handled)
                {
                    ll_inst->set_opcode(OpCodeEnum::NewObjInternalCall);
                    setup_inst_method(ll_inst, hl_inst);
                }
                break;
            }
            case hl::OpCodeEnum::NewObjIntrinsic:
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_newobj_methods(ll_inst, hl_inst));
                if (!handled)
                {
                    ll_inst->set_opcode(OpCodeEnum::NewObjIntrinsic);
                    setup_inst_method(ll_inst, hl_inst);
                }
                break;
            }
            case hl::OpCodeEnum::NewObjAot:
            {
                const metadata::RtMethodInfo* method = hl_inst->get_method();
                OpCodeEnum op = vm::Class::is_value_type(method->parent) ? OpCodeEnum::NewValueTypeAot : OpCodeEnum::NewObjAot;
                ll_inst->set_opcode(op);
                setup_inst_method(ll_inst, hl_inst);
                break;
            }
            case hl::OpCodeEnum::Ldftn:
            {
                ll_inst->set_opcode(OpCodeEnum::Ldftn);
                setup_inst_resolved_data(ll_inst, hl_inst->get_method());
                break;
            }
            case hl::OpCodeEnum::Ldvirtftn:
            {
                ll_inst->set_opcode(OpCodeEnum::Ldvirtftn);
                setup_inst_resolved_data(ll_inst, hl_inst->get_method());
                break;
            }
            case hl::OpCodeEnum::Throw:
            {
                ll_inst->set_opcode(OpCodeEnum::Throw);
                break;
            }
            case hl::OpCodeEnum::Rethrow:
            {
                ll_inst->set_opcode(OpCodeEnum::Rethrow);
                break;
            }
            case hl::OpCodeEnum::Leave:
            {
                DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(BasicBlock*, leave_target, translate_hl_basic_to_ll_basic(hl_inst->get_leave_target()));
                switch (get_leave_surrounding_block_type(cur_bb, leave_target))
                {
                case LeaveSurroundingBlockType::None:
                    ll_inst->set_opcode(OpCodeEnum::Br);
                    ll_inst->update_branch_target(leave_target);
                    break;
                case LeaveSurroundingBlockType::Try:
                {
                    utils::NotFreeList<size_t> finally_clause_idx = find_finally_clause_idx_of_leave_target(cur_bb, leave_target);
                    if (finally_clause_idx.empty())
                    {
                        ll_inst->set_opcode(OpCodeEnum::Br);
                        ll_inst->update_branch_target(leave_target);
                    }
                    else
                    {
                        ll_inst->set_opcode(OpCodeEnum::LeaveTryWithFinally);
                        ll_inst->set_first_finally_clause_index(finally_clause_idx[0]);
                        ll_inst->set_finally_clauses_count(finally_clause_idx.size());
                        ll_inst->update_leave_target(leave_target);
                    }
                    break;
                }
                case LeaveSurroundingBlockType::CatchOrFilter:
                {
                    utils::NotFreeList<size_t> finally_clause_idx = find_finally_clause_idx_of_leave_target(cur_bb, leave_target);
                    if (finally_clause_idx.empty())
                    {
                        ll_inst->set_opcode(OpCodeEnum::LeaveCatchWithoutFinally);
                        ll_inst->update_leave_target(leave_target);
                    }
                    else
                    {
                        ll_inst->set_opcode(OpCodeEnum::LeaveCatchWithFinally);
                        ll_inst->set_first_finally_clause_index(finally_clause_idx[0]);
                        ll_inst->set_finally_clauses_count(finally_clause_idx.size());
                        ll_inst->update_leave_target(leave_target);
                    }
                    break;
                }
                }
                break;
            }
            case hl::OpCodeEnum::EndFilter:
            {
                ll_inst->set_opcode(OpCodeEnum::EndFilter);
                break;
            }
            case hl::OpCodeEnum::EndFinallyOrFault:
            {
                metadata::RtILExceptionClauseType clause_type = get_cur_endfinally_or_fault_clause_type(cur_bb);
                OpCodeEnum op = OpCodeEnum::Illegal;
                if (clause_type == metadata::RtILExceptionClauseType::Finally)
                {
                    op = OpCodeEnum::EndFinally;
                }
                else if (clause_type == metadata::RtILExceptionClauseType::Fault)
                {
                    op = OpCodeEnum::EndFault;
                }
                else
                {
                    RET_ERR(core::RtErr::ExecutionEngine);
                }
                ll_inst->set_opcode(op);
                break;
            }

            case hl::OpCodeEnum::GetEnumLongHashCode:
            {
                ll_inst->set_opcode(OpCodeEnum::GetEnumLongHashCode);
                break;
            }
            default:
                RETURN_NOT_IMPLEMENTED_ERROR();
            }

            if (add && ll_inst->get_opcode() != OpCodeEnum::Nop)
            {
                cur_bb->insts.push_back(ll_inst);
            }
        }

        cur_bb = cur_bb->next_bb;
    }

    RET_VOID_OK();
}

RtResultVoid Transformer::optimize_short_instructions()
{
    // TODO: Implement instruction optimization
    RET_VOID_OK();
}

RtResultVoid Transformer::build_exception_clauses(RtInterpMethodInfo* interp_method)
{
    const metadata::RtMethodBody* method_body = _hl_transformer.get_method_body();
    const auto& src_clauses = method_body->exception_clauses;
    size_t exception_clause_count = src_clauses.size();

    if (exception_clause_count > UINT8_MAX)
    {
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    metadata::RtModuleDef* ass = _hl_transformer.get_module();
    RtInterpExceptionClause* exception_clauses = ass->get_mem_pool().calloc_any<RtInterpExceptionClause>(exception_clause_count);
    interp_method->exception_clauses = exception_clauses;
    interp_method->exception_clause_count = static_cast<uint8_t>(exception_clause_count);

    for (size_t i = 0; i < exception_clause_count; ++i)
    {
        const metadata::RtExceptionClause& src = src_clauses[i];
        RtInterpExceptionClause& dst = exception_clauses[i];

        uint32_t filter_begin_offset;
        metadata::RtClass* ex_klass;

        if (src.flags == metadata::RtILExceptionClauseType::Filter)
        {
            filter_begin_offset = src.class_token_or_filter_offset;
            ex_klass = nullptr;
        }
        else if (src.flags == metadata::RtILExceptionClauseType::Exception)
        {
            filter_begin_offset = 0;
            metadata::RtToken token = metadata::RtToken::decode(src.class_token_or_filter_offset);
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(
                const metadata::RtTypeSig*, final_type_sig,
                ass->get_typesig_by_type_def_ref_spec_token(token, _hl_transformer.get_generic_container_context(), _hl_transformer.get_generic_context()));
            UNWRAP_OR_RET_ERR_ON_FAIL(ex_klass, vm::Class::get_class_from_typesig(final_type_sig));
        }
        else
        {
            filter_begin_offset = 0;
            ex_klass = nullptr;
        }

        dst.flags = src.flags;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, try_begin, translate_il_offset_to_ir_offset(src.try_offset));
        dst.try_begin_offset = try_begin;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, try_end, translate_il_offset_to_ir_offset(src.try_offset + src.try_length));
        dst.try_end_offset = try_end;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, handler_begin, translate_il_offset_to_ir_offset(src.handler_offset));
        dst.handler_begin_offset = handler_begin;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, handler_end, translate_il_offset_to_ir_offset(src.handler_offset + src.handler_length));
        dst.handler_end_offset = handler_end;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(uint32_t, filter_begin, translate_il_offset_to_ir_offset(filter_begin_offset));
        dst.filter_begin_offset = filter_begin;
        dst.ex_klass = ex_klass;
    }

    RET_VOID_OK();
}

RtResult<uint32_t> Transformer::translate_il_offset_to_ir_offset(uint32_t il_offset)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const hl::BasicBlock*, hl_bb, _hl_transformer.get_branch_target_bb(static_cast<size_t>(il_offset)));

    auto it = _hl_2_ll_bb_map.find(hl_bb);
    if (it != _hl_2_ll_bb_map.end())
    {
        RET_OK(static_cast<uint32_t>(it->second->ir_offset));
    }

    RET_ERR(core::RtErr::ExecutionEngine);
}

RtResultVoid Transformer::build_codes(RtInterpMethodInfo* interp_method)
{
    metadata::RtModuleDef* mod = _hl_transformer.get_module();

    // First pass: calculate total size and set offsets
    size_t total_ir_size = 0;
    BasicBlock* cur_bb = _bb_head;

    size_t total_ir_count = 0;
    for (BasicBlock* cur_bb = _bb_head; cur_bb != nullptr; cur_bb = cur_bb->next_bb)
    {
        cur_bb->ir_offset = static_cast<int32_t>(total_ir_size);
        total_ir_count += cur_bb->insts.size();
        for (const GeneralInst* inst : cur_bb->insts)
        {
            GeneralInst* mutable_inst = const_cast<GeneralInst*>(inst);
            mutable_inst->set_ir_offset(static_cast<int32_t>(total_ir_size));
            total_ir_size += ll::OpCodes::get_instruction_size(inst->get_opcode(), *inst);
        }
    }

    interp_method->code_size = static_cast<uint32_t>(total_ir_size);

    // Second pass: write instructions
    uint8_t* codes = (uint8_t*)mod->get_mem_pool().calloc_any<uint8_t>(total_ir_size);
    interp_method->codes = codes;

    metadata::PdbImage* pdb_image = mod->get_pdb_image();

    uint8_t* codes_cur = codes;
    if (pdb_image == nullptr)
    {
        for (BasicBlock* cur_bb = _bb_head; cur_bb != nullptr; cur_bb = cur_bb->next_bb)
        {
            for (const GeneralInst* inst : cur_bb->insts)
            {
                codes_cur = ll::OpCodes::write_instruction_to_data(codes_cur, *inst);
            }
        }
    }
    else
    {
        metadata::IR2ILMapEntry* ir2il_map_entries = ir2il_map_entries = mod->get_mem_pool().calloc_any<metadata::IR2ILMapEntry>(total_ir_count);
        size_t ir_index = 0;
        for (BasicBlock* cur_bb = _bb_head; cur_bb != nullptr; cur_bb = cur_bb->next_bb)
        {
            for (const GeneralInst* inst : cur_bb->insts)
            {
                codes_cur = ll::OpCodes::write_instruction_to_data(codes_cur, *inst);
                ir2il_map_entries[ir_index++] = {inst->get_il_offset(), inst->get_ir_offset()};
            }
        }
        pdb_image->add_ir2il_map_for_method(_hl_transformer.get_method_info(), {utils::Span<metadata::IR2ILMapEntry>{ir2il_map_entries, total_ir_count}});
    }

    assert(codes_cur == codes + total_ir_size);
    RET_VOID_OK();
}

RtResultVoid Transformer::transform()
{
    RET_ERR_ON_FAIL(transform_basic_blocks());
    RET_ERR_ON_FAIL(transform_instructions());
    RET_ERR_ON_FAIL(optimize_short_instructions());
    RET_VOID_OK();
}

RtResult<const RtInterpMethodInfo*> Transformer::build_interp_method_info()
{
    const metadata::RtMethodInfo* method = _hl_transformer.get_method_info();
    metadata::RtModuleDef* mod = _hl_transformer.get_module();
    alloc::MemPool& pool = mod->get_mem_pool();

    RtInterpMethodInfo* interp_method = pool.malloc_any_zeroed<RtInterpMethodInfo>();

    if (!_resolved_datas.empty())
    {
        const void** resolved_datas = (const void**)pool.calloc_any<const void*>(_resolved_datas.size());
        std::memcpy(resolved_datas, _resolved_datas.data(), _resolved_datas.size() * sizeof(const void*));
        interp_method->resolved_datas = resolved_datas;
    }

    interp_method->total_arg_and_local_stack_object_size = static_cast<uint16_t>(_hl_transformer.get_total_arg_and_local_stack_object_size());
    interp_method->max_stack_object_size = static_cast<uint16_t>(_hl_transformer.get_max_stack_size());
    interp_method->init_locals = _hl_transformer.need_init_locals();

    RET_ERR_ON_FAIL(build_codes(interp_method));
    RET_ERR_ON_FAIL(build_exception_clauses(interp_method));

    RET_OK(interp_method);
}

} // namespace ll
} // namespace interp
} // namespace leanclr

#include "transformer.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/rt_string.h"
#include "vm/assembly.h"
#include "vm/array_class.h"
#include "metadata/metadata_const.h"
#include "metadata/module_def.h"
#include "utils/platform.h"
#include "const_strs.h"
#include <climits>

namespace leanclr
{
namespace interp
{
using ll::OpCodeEnum;

RtResultVoid Transformer::flush_pending_instructions()
{
    for (const PendingInst& pending : _pending_insts)
    {
        const GeneralInst input = *pending.inst;
        RET_ERR_ON_FAIL((this->*pending.emit)(pending.inst, &input));
        if (pending.inst->get_opcode() != OpCodeEnum::Nop)
            _cur_bb->insts.push_back(pending.inst);
    }
    _pending_insts.clear();
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

void Transformer::setup_inst_resolved_data(GeneralInst* output, const void* data)
{
    size_t index = get_resolved_data_index(data);
    output->set_resolved_data_index(index);
}

void Transformer::setup_inst_klass(GeneralInst* output, const GeneralInst* input)
{
    metadata::RtClass* klass = input->get_class();
    setup_inst_resolved_data(output, klass);
}

void Transformer::setup_inst_method(GeneralInst* output, const GeneralInst* input)
{
    const metadata::RtMethodInfo* method = input->get_method();
    setup_inst_resolved_data(output, method);
}

utils::NotFreeList<size_t> Transformer::find_finally_clause_idx_of_leave_target(const BasicBlock* leave_src, const BasicBlock* leave_target)
{
    uint32_t il_src = static_cast<uint32_t>(leave_src->il_begin_offset);
    uint32_t il_target = static_cast<uint32_t>(leave_target->il_begin_offset);
    utils::NotFreeList<size_t> result(_pool);

    auto* method_body = get_method_body();
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
    uint32_t il_src = static_cast<uint32_t>(leave_src->il_begin_offset);
    uint32_t il_target = static_cast<uint32_t>(leave_target->il_begin_offset);

    auto* method_body = get_method_body();
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
    uint32_t il_src = static_cast<uint32_t>(cur_bb->il_begin_offset);

    auto* method_body = get_method_body();
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

RtResultVoid Transformer::transform_condition_branch(GeneralInst* output, const GeneralInst* input, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8,
                                                     OpCodeEnum opcode_r4, OpCodeEnum opcode_r8)
{
    const Variable* src = input->get_var_src();

    switch (src->data_type)
    {
    case RtEvalStackDataType::I4:
        output->set_opcode(opcode_i4); // Set opcode for I4
        break;
    case RtEvalStackDataType::I8:
        output->set_opcode(opcode_i8); // Set opcode for I8
        break;
    case RtEvalStackDataType::R4:
        output->set_opcode(opcode_r4); // Set opcode for R4
        break;
    case RtEvalStackDataType::R8:
        output->set_opcode(opcode_r8); // Set opcode for R8
        break;
    case RtEvalStackDataType::RefOrPtr:
        output->set_opcode(utils::Platform::select_arch(opcode_i4, opcode_i8)); // Set opcode for RefOrPtr
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    output->update_branch_target(input->get_branch_target());

    RET_VOID_OK();
}

RtResultVoid Transformer::transform_brtruefalse(GeneralInst* output, const GeneralInst* input, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8)
{
    const Variable* src = input->get_var_src();
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

    output->set_opcode(ll_op);

    output->update_branch_target(input->get_branch_target());

    RET_VOID_OK();
}

RtResultVoid Transformer::transform_bin_arith_op(GeneralInst* output, const GeneralInst* input, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8,
                                                 OpCodeEnum opcode_r4, OpCodeEnum opcode_r8)
{
    const Variable* dst = input->get_var_dst();

    switch (dst->data_type)
    {
    case RtEvalStackDataType::I4:
        output->set_opcode(opcode_i4);
        break;
    case RtEvalStackDataType::I8:
        output->set_opcode(opcode_i8);
        break;
    case RtEvalStackDataType::R4:
        output->set_opcode(opcode_r4);
        break;
    case RtEvalStackDataType::R8:
        output->set_opcode(opcode_r8);
        break;
    case RtEvalStackDataType::RefOrPtr:
        output->set_opcode(utils::Platform::select_arch(opcode_i4, opcode_i8));
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    RET_VOID_OK();
}

RtResultVoid Transformer::transform_bin_arith_ovf_op(GeneralInst* output, const GeneralInst* input, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8)
{
    const Variable* dst = input->get_var_dst();

    switch (dst->data_type)
    {
    case RtEvalStackDataType::I4:
        output->set_opcode(opcode_i4); // Set opcode for I4
        break;
    case RtEvalStackDataType::I8:
        output->set_opcode(opcode_i8); // Set opcode for I8
        break;
    case RtEvalStackDataType::RefOrPtr:
        output->set_opcode(utils::Platform::select_arch(opcode_i4, opcode_i8)); // Set opcode for RefOrPtr
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    RET_VOID_OK();
}

RtResultVoid Transformer::transform_bin_bit_op(GeneralInst* output, const GeneralInst* input, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8)
{
    const Variable* dst = input->get_var_dst();

    switch (dst->data_type)
    {
    case RtEvalStackDataType::I4:
        output->set_opcode(opcode_i4); // Set opcode for I4
        break;
    case RtEvalStackDataType::I8:
        output->set_opcode(opcode_i8); // Set opcode for I8
        break;
    case RtEvalStackDataType::RefOrPtr:
        output->set_opcode(utils::Platform::select_arch(opcode_i4, opcode_i8)); // Set opcode for RefOrPtr
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    RET_VOID_OK();
}

RtResultVoid Transformer::transform_bin_bit_shift_op(GeneralInst* output, const GeneralInst* input, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8)
{
    const Variable* shift_amount = input->get_var_arg2();
    const Variable* dst = input->get_var_dst();

    if (shift_amount->data_type != RtEvalStackDataType::I4 && shift_amount->data_type != RtEvalStackDataType::RefOrPtr)
    {
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    switch (dst->data_type)
    {
    case RtEvalStackDataType::I4:
        output->set_opcode(opcode_i4);
        break;
    case RtEvalStackDataType::I8:
        output->set_opcode(opcode_i8);
        break;
    case RtEvalStackDataType::RefOrPtr:
        output->set_opcode(utils::Platform::select_arch(opcode_i4, opcode_i8));
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    RET_VOID_OK();
}

RtResultVoid Transformer::transform_conv(GeneralInst* output, const GeneralInst* input, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8, OpCodeEnum opcode_r4,
                                         OpCodeEnum opcode_r8)
{
    const Variable* src = input->get_var_src();
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

    output->set_opcode(opcode);
    RET_VOID_OK();
}

RtResultVoid Transformer::transform_cmp_op(GeneralInst* output, const GeneralInst* input, OpCodeEnum opcode_i4, OpCodeEnum opcode_i8, OpCodeEnum opcode_r4,
                                           OpCodeEnum opcode_r8)
{
    const Variable* op1 = input->get_var_arg1();

    switch (op1->data_type)
    {
    case RtEvalStackDataType::I4:
        output->set_opcode(opcode_i4);
        break;
    case RtEvalStackDataType::I8:
        output->set_opcode(opcode_i8);
        break;
    case RtEvalStackDataType::R4:
        output->set_opcode(opcode_r4);
        break;
    case RtEvalStackDataType::R8:
        output->set_opcode(opcode_r8);
        break;
    case RtEvalStackDataType::RefOrPtr:
        output->set_opcode(utils::Platform::select_arch(opcode_i4, opcode_i8));
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    RET_VOID_OK();
}

RtResult<bool> Transformer::transform_special_call_methods(GeneralInst* output, const GeneralInst* input)
{
    const metadata::RtMethodInfo* method = input->get_method();
    const metadata::RtClass* klass = method->parent;

    if (!klass->image->is_corlib())
    {
        RET_OK(false);
    }

    output->set_opcode(OpCodeEnum::Illegal);

    const vm::CorLibTypes& corlib_types = vm::Class::get_corlib_types();
    const char* klass_name = klass->name;
    const char* method_name = method->name;
    size_t param_count = static_cast<size_t>(method->parameter_count);

    const Variable* const* params = output->get_params();

    if (klass == corlib_types.cls_object)
    {
        if (std::strcmp(method_name, STR_CTOR) == 0)
        {
            output->set_opcode(OpCodeEnum::Nop);
        }
    }
    else if (klass == corlib_types.cls_array)
    {
        if (std::strcmp(method_name, "UnsafeMov") == 0)
        {
            output->set_opcode(OpCodeEnum::Nop);
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
                    output->set_opcode(utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8I4));
                    output->update_var_src(arg_value);
                    output->update_var_dst(arg_this);
                    break;
                case metadata::RtElementType::I8:
                case metadata::RtElementType::U8:
                case metadata::RtElementType::I:
                case metadata::RtElementType::U:
                case metadata::RtElementType::Ptr:
                    output->set_opcode(utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8));
                    output->update_var_src(arg_value);
                    output->update_var_dst(arg_this);
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
                    output->set_opcode(utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8U4));
                    output->update_var_src(arg_value);
                    output->update_var_dst(arg_this);
                    break;
                case metadata::RtElementType::I8:
                case metadata::RtElementType::U8:
                case metadata::RtElementType::I:
                case metadata::RtElementType::U:
                case metadata::RtElementType::Ptr:
                    output->set_opcode(utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8));
                    output->update_var_src(arg_value);
                    output->update_var_dst(arg_this);
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
            output->set_opcode(offset <= INT16_MAX ? OpCodeEnum::LdcI4I2 : OpCodeEnum::LdcI4I4);
            output->set_i4(offset);
            output->update_var_dst(output->get_var_ret());
        }
    }
    else if (klass == corlib_types.cls_appdomain)
    {
        if (std::strcmp(method_name, "IsAppXModel") == 0)
        {
            output->set_opcode(OpCodeEnum::LdcI4I2);
            output->set_i4(0); // false
            output->update_var_dst(output->get_var_ret());
        }
    }
    else if (std::strcmp(klass_name, "ByReference`1") == 0)
    {
        if (std::strcmp(method_name, STR_CTOR) == 0)
        {
            output->set_opcode(OpCodeEnum::Nop);
        }
        else if (std::strcmp(method_name, "get_Value") == 0)
        {
            const Variable* arg_this = params[0];
            const Variable* dst = output->get_var_ret();
            output->set_opcode(utils::Platform::select_arch(OpCodeEnum::LdIndI4, OpCodeEnum::LdIndI8));
            output->update_var_src(arg_this);
            output->update_var_dst(dst);
        }
    }
    else if (std::strcmp(klass_name, "JitHelpers") == 0)
    {
        if (std::strcmp(method_name, "UnsafeCast") == 0 || std::strcmp(method_name, "UnsafeEnumCast") == 0 || std::strcmp(method_name, "UnsafeEnumCastLong"))
        {
            output->set_opcode(OpCodeEnum::Nop);
        }
    }

    RET_OK(output->get_opcode() != OpCodeEnum::Illegal);
}

RtResult<bool> Transformer::transform_special_newobj_methods(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::Illegal);
    const metadata::RtMethodInfo* method = input->get_method();
    const metadata::RtClass* klass = method->parent;
    if (!klass->image->is_corlib())
    {
        RET_OK(false);
    }

    const vm::CorLibTypes& corlib_types = vm::Class::get_corlib_types();
    const char* klass_name = klass->name;
    size_t param_count = static_cast<size_t>(method->parameter_count);

    const Variable* const* params = output->get_params();

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
                output->set_opcode(utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvI8I4));
                output->update_var_src(arg_value);
                output->update_var_dst(arg_value);
                break;
            case metadata::RtElementType::I8:
            case metadata::RtElementType::U8:
            case metadata::RtElementType::I:
            case metadata::RtElementType::U:
            case metadata::RtElementType::Ptr:
                output->set_opcode(OpCodeEnum::Nop);
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
                output->set_opcode(utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvI8U4));
                output->update_var_src(arg_value);
                output->update_var_dst(arg_value);
                break;
            case metadata::RtElementType::I8:
            case metadata::RtElementType::U8:
            case metadata::RtElementType::I:
            case metadata::RtElementType::U:
            case metadata::RtElementType::Ptr:
                output->set_opcode(OpCodeEnum::Nop);
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
        output->set_opcode(OpCodeEnum::Nop);
    }

    RET_OK(output->get_opcode() != OpCodeEnum::Illegal);
}

RtResultVoid Transformer::emit_Nop(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::Nop);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ProfileAddCost(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::ProfileAddCost);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Arglist(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::Arglist);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_InitLocals(GeneralInst* output, const GeneralInst* input)
{
    if (input->get_size() == 0)
        output->set_opcode(OpCodeEnum::Nop);
    else
        output->set_opcode(OpCodeEnum::InitLocals);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdArg(GeneralInst* output, const GeneralInst* input)
{
    const Variable* src = input->get_var_arg1();
    switch (src->reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
        output->set_opcode(OpCodeEnum::LdLocI1);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        output->set_opcode(OpCodeEnum::LdLocU1);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I2:
        output->set_opcode(OpCodeEnum::LdLocI2);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        output->set_opcode(OpCodeEnum::LdLocU2);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I4:
    case metadata::RtArgOrLocOrFieldReduceType::R4:
        output->set_opcode(OpCodeEnum::LdLocI4);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I8:
    case metadata::RtArgOrLocOrFieldReduceType::R8:
        output->set_opcode(OpCodeEnum::LdLocI8);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I:
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
        output->set_opcode(utils::Platform::select_arch(OpCodeEnum::LdLocI4, OpCodeEnum::LdLocI8));
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Other:
        output->set_opcode(OpCodeEnum::LdLocAny);
        output->set_stack_object_size(src->stack_object_size);
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdLoc(GeneralInst* output, const GeneralInst* input)
{
    const Variable* src = input->get_var_arg1();
    switch (src->reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
        output->set_opcode(OpCodeEnum::LdLocI1);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        output->set_opcode(OpCodeEnum::LdLocU1);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I2:
        output->set_opcode(OpCodeEnum::LdLocI2);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        output->set_opcode(OpCodeEnum::LdLocU2);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I4:
    case metadata::RtArgOrLocOrFieldReduceType::R4:
        output->set_opcode(OpCodeEnum::LdLocI4);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I8:
    case metadata::RtArgOrLocOrFieldReduceType::R8:
        output->set_opcode(OpCodeEnum::LdLocI8);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I:
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
        output->set_opcode(utils::Platform::select_arch(OpCodeEnum::LdLocI4, OpCodeEnum::LdLocI8));
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Other:
        output->set_opcode(OpCodeEnum::LdLocAny);
        output->set_stack_object_size(src->stack_object_size);
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdArga(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdLoca);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdLoca(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdLoca);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StArg(GeneralInst* output, const GeneralInst* input)
{
    const Variable* dst = input->get_var_dst();
    switch (dst->reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        output->set_opcode(OpCodeEnum::StLocI1);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I2:
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        output->set_opcode(OpCodeEnum::StLocI2);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I4:
    case metadata::RtArgOrLocOrFieldReduceType::R4:
        output->set_opcode(OpCodeEnum::StLocI4);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I8:
    case metadata::RtArgOrLocOrFieldReduceType::R8:
        output->set_opcode(OpCodeEnum::StLocI8);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I:
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
        output->set_opcode(utils::Platform::select_arch(OpCodeEnum::StLocI4, OpCodeEnum::StLocI8));
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Other:
        output->set_opcode(OpCodeEnum::StLocAny);
        output->set_stack_object_size(dst->stack_object_size);
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StLoc(GeneralInst* output, const GeneralInst* input)
{
    const Variable* dst = input->get_var_dst();
    switch (dst->reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        output->set_opcode(OpCodeEnum::StLocI1);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I2:
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        output->set_opcode(OpCodeEnum::StLocI2);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I4:
    case metadata::RtArgOrLocOrFieldReduceType::R4:
        output->set_opcode(OpCodeEnum::StLocI4);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I8:
    case metadata::RtArgOrLocOrFieldReduceType::R8:
        output->set_opcode(OpCodeEnum::StLocI8);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I:
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
        output->set_opcode(utils::Platform::select_arch(OpCodeEnum::StLocI4, OpCodeEnum::StLocI8));
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Other:
        output->set_opcode(OpCodeEnum::StLocAny);
        output->set_stack_object_size(dst->stack_object_size);
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdNull(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdNull);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdcI4(GeneralInst* output, const GeneralInst* input)
{
    int32_t value = input->get_i4();
    if (value >= INT16_MIN && value <= INT16_MAX)
    {
        output->set_opcode(OpCodeEnum::LdcI4I2);
    }
    else
    {
        output->set_opcode(OpCodeEnum::LdcI4I4);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdcI8(GeneralInst* output, const GeneralInst* input)
{
    int64_t value = input->get_i8();
    if (value >= INT16_MIN && value <= INT16_MAX)
    {
        output->set_opcode(OpCodeEnum::LdcI8I2);
    }
    else if (value >= INT32_MIN && value <= INT32_MAX)
    {
        output->set_opcode(OpCodeEnum::LdcI8I4);
    }
    else
    {
        output->set_opcode(OpCodeEnum::LdcI8I8);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdcR4(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdcI4I4);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdcR8(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdcI8I8);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdStr(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdStr);
    setup_inst_resolved_data(output, input->get_user_string());
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Dup(GeneralInst* output, const GeneralInst* input)
{
    const Variable* src = input->get_var_src();
    switch (src->data_type)
    {
    case RtEvalStackDataType::I4:
    case RtEvalStackDataType::R4:
        output->set_opcode(OpCodeEnum::LdLocI4);
        break;
    case RtEvalStackDataType::I8:
    case RtEvalStackDataType::R8:
        output->set_opcode(OpCodeEnum::LdLocI8);
        break;
    case RtEvalStackDataType::RefOrPtr:
        output->set_opcode(utils::Platform::select_arch(OpCodeEnum::LdLocI4, OpCodeEnum::LdLocI8));
        break;
    case RtEvalStackDataType::Other:
        output->set_opcode(OpCodeEnum::LdLocAny);
        output->set_stack_object_size(src->stack_object_size);
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Ret(GeneralInst* output, const GeneralInst* input)
{
    const Variable* ret = get_ret_var();
    if (ret == nullptr)
    {
        output->set_opcode(OpCodeEnum::RetVoid);
    }
    else if (input->get_var_src_eval_stack_idx() == 0)
    {
        output->set_opcode(OpCodeEnum::RetNop);
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
            output->set_size(ret->stack_object_size);
            op = OpCodeEnum::RetAny;
            break;
        default:
            RET_ERR(core::RtErr::ExecutionEngine);
        }
        output->set_opcode(op);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Br(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::Br);
    output->update_branch_target(input->get_branch_target());
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_BrTrue(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_brtruefalse(output, input, OpCodeEnum::BrTrueI4, OpCodeEnum::BrTrueI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_BrFalse(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_brtruefalse(output, input, OpCodeEnum::BrFalseI4, OpCodeEnum::BrFalseI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Beq(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_condition_branch(output, input, OpCodeEnum::BeqI4, OpCodeEnum::BeqI8, OpCodeEnum::BeqR4, OpCodeEnum::BeqR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Bge(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_condition_branch(output, input, OpCodeEnum::BgeI4, OpCodeEnum::BgeI8, OpCodeEnum::BgeR4, OpCodeEnum::BgeR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Bgt(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_condition_branch(output, input, OpCodeEnum::BgtI4, OpCodeEnum::BgtI8, OpCodeEnum::BgtR4, OpCodeEnum::BgtR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Ble(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_condition_branch(output, input, OpCodeEnum::BleI4, OpCodeEnum::BleI8, OpCodeEnum::BleR4, OpCodeEnum::BleR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Blt(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_condition_branch(output, input, OpCodeEnum::BltI4, OpCodeEnum::BltI8, OpCodeEnum::BltR4, OpCodeEnum::BltR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_BneUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_condition_branch(output, input, OpCodeEnum::BneUnI4, OpCodeEnum::BneUnI8, OpCodeEnum::BneUnR4, OpCodeEnum::BneUnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_BgeUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_condition_branch(output, input, OpCodeEnum::BgeUnI4, OpCodeEnum::BgeUnI8, OpCodeEnum::BgeUnR4, OpCodeEnum::BgeUnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_BgtUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_condition_branch(output, input, OpCodeEnum::BgtUnI4, OpCodeEnum::BgtUnI8, OpCodeEnum::BgtUnR4, OpCodeEnum::BgtUnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_BleUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_condition_branch(output, input, OpCodeEnum::BleUnI4, OpCodeEnum::BleUnI8, OpCodeEnum::BleUnR4, OpCodeEnum::BleUnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_BltUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_condition_branch(output, input, OpCodeEnum::BltUnI4, OpCodeEnum::BltUnI8, OpCodeEnum::BltUnR4, OpCodeEnum::BltUnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Switch(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::Switch);
    const auto targets = input->get_switch_targets();
    output->update_switch_targets(targets.targets, targets.count);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdIndI1(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdIndI1);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdIndU1(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdIndU1);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdIndI2(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? OpCodeEnum::LdIndI2Unaligned : OpCodeEnum::LdIndI2);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdIndU2(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? OpCodeEnum::LdIndU2Unaligned : OpCodeEnum::LdIndU2);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdIndI4(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? OpCodeEnum::LdIndI4Unaligned : OpCodeEnum::LdIndI4);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdIndR4(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? OpCodeEnum::LdIndI4Unaligned : OpCodeEnum::LdIndI4);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdIndI8(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? OpCodeEnum::LdIndI8Unaligned : OpCodeEnum::LdIndI8);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdIndR8(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? OpCodeEnum::LdIndI8Unaligned : OpCodeEnum::LdIndI8);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdIndRef(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? utils::Platform::select_arch(OpCodeEnum::LdIndI4Unaligned, OpCodeEnum::LdIndI8Unaligned)
                                                          : utils::Platform::select_arch(OpCodeEnum::LdIndI4, OpCodeEnum::LdIndI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StIndI1(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::StIndI1);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StIndI2(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? OpCodeEnum::StIndI2Unaligned : OpCodeEnum::StIndI2);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StIndI4(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? OpCodeEnum::StIndI4Unaligned : OpCodeEnum::StIndI4);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StIndR4(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? OpCodeEnum::StIndI4Unaligned : OpCodeEnum::StIndI4);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StIndI8(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? OpCodeEnum::StIndI8Unaligned : OpCodeEnum::StIndI8);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StIndR8(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? OpCodeEnum::StIndI8Unaligned : OpCodeEnum::StIndI8);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StIndRef(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_unaligned() ? utils::Platform::select_arch(OpCodeEnum::StIndI4Unaligned, OpCodeEnum::StIndI8Unaligned)
                                                          : utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Add(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_op(output, input, OpCodeEnum::AddI4, OpCodeEnum::AddI8, OpCodeEnum::AddR4, OpCodeEnum::AddR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Sub(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_op(output, input, OpCodeEnum::SubI4, OpCodeEnum::SubI8, OpCodeEnum::SubR4, OpCodeEnum::SubR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Mul(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_op(output, input, OpCodeEnum::MulI4, OpCodeEnum::MulI8, OpCodeEnum::MulR4, OpCodeEnum::MulR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Div(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_op(output, input, OpCodeEnum::DivI4, OpCodeEnum::DivI8, OpCodeEnum::DivR4, OpCodeEnum::DivR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Rem(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_op(output, input, OpCodeEnum::RemI4, OpCodeEnum::RemI8, OpCodeEnum::RemR4, OpCodeEnum::RemR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_DivUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_op(output, input, OpCodeEnum::DivUnI4, OpCodeEnum::DivUnI8, OpCodeEnum::DivR4, OpCodeEnum::DivR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_RemUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_op(output, input, OpCodeEnum::RemUnI4, OpCodeEnum::RemUnI8, OpCodeEnum::RemR4, OpCodeEnum::RemR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_And(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_bit_op(output, input, OpCodeEnum::AndI4, OpCodeEnum::AndI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Or(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_bit_op(output, input, OpCodeEnum::OrI4, OpCodeEnum::OrI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Xor(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_bit_op(output, input, OpCodeEnum::XorI4, OpCodeEnum::XorI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Shl(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_bit_shift_op(output, input, OpCodeEnum::ShlI4, OpCodeEnum::ShlI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Shr(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_bit_shift_op(output, input, OpCodeEnum::ShrI4, OpCodeEnum::ShrI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ShrUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_bit_shift_op(output, input, OpCodeEnum::ShrUnI4, OpCodeEnum::ShrUnI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Neg(GeneralInst* output, const GeneralInst* input)
{
    const Variable* dst = input->get_var_dst();
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
    output->set_opcode(op);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Not(GeneralInst* output, const GeneralInst* input)
{
    const Variable* dst = input->get_var_dst();
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
    output->set_opcode(op);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_AddOvf(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_ovf_op(output, input, OpCodeEnum::AddOvfI4, OpCodeEnum::AddOvfI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_AddOvfUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_ovf_op(output, input, OpCodeEnum::AddOvfUnI4, OpCodeEnum::AddOvfUnI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_SubOvf(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_ovf_op(output, input, OpCodeEnum::SubOvfI4, OpCodeEnum::SubOvfI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_SubOvfUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_ovf_op(output, input, OpCodeEnum::SubOvfUnI4, OpCodeEnum::SubOvfUnI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_MulOvf(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_ovf_op(output, input, OpCodeEnum::MulOvfI4, OpCodeEnum::MulOvfI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_MulOvfUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_bin_arith_ovf_op(output, input, OpCodeEnum::MulOvfUnI4, OpCodeEnum::MulOvfUnI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvI1(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvI1I4, OpCodeEnum::ConvI1I8, OpCodeEnum::ConvI1R4, OpCodeEnum::ConvI1R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvU1(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvU1I4, OpCodeEnum::ConvU1I8, OpCodeEnum::ConvU1R4, OpCodeEnum::ConvU1R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvI2(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvI2I4, OpCodeEnum::ConvI2I8, OpCodeEnum::ConvI2R4, OpCodeEnum::ConvI2R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvU2(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvU2I4, OpCodeEnum::ConvU2I8, OpCodeEnum::ConvU2R4, OpCodeEnum::ConvU2R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvI4(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::Nop, OpCodeEnum::ConvI4I8, OpCodeEnum::ConvI4R4, OpCodeEnum::ConvI4R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvU4(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::Nop, OpCodeEnum::ConvU4I8, OpCodeEnum::ConvU4R4, OpCodeEnum::ConvU4R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvI8(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvI8I4, OpCodeEnum::Nop, OpCodeEnum::ConvI8R4, OpCodeEnum::ConvI8R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvU8(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvU8I4, OpCodeEnum::Nop, OpCodeEnum::ConvU8R4, OpCodeEnum::ConvU8R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvI(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(
        output, input, utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvI8I4), utils::Platform::select_arch(OpCodeEnum::ConvI4I8, OpCodeEnum::Nop),
        utils::Platform::select_arch(OpCodeEnum::ConvI4R4, OpCodeEnum::ConvI8R4), utils::Platform::select_arch(OpCodeEnum::ConvI4R8, OpCodeEnum::ConvI8R8)));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvU(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(
        output, input, utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvU8I4), utils::Platform::select_arch(OpCodeEnum::ConvU4I8, OpCodeEnum::Nop),
        utils::Platform::select_arch(OpCodeEnum::ConvU4R4, OpCodeEnum::ConvU8R4), utils::Platform::select_arch(OpCodeEnum::ConvU4R8, OpCodeEnum::ConvU8R8)));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvR4(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvR4I4, OpCodeEnum::ConvR4I8, OpCodeEnum::Nop, OpCodeEnum::ConvR4R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvR8(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvR8I4, OpCodeEnum::ConvR8I8, OpCodeEnum::ConvR8R4, OpCodeEnum::Nop));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfI1(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvOvfI1I4, OpCodeEnum::ConvOvfI1I8, OpCodeEnum::ConvOvfI1R4, OpCodeEnum::ConvOvfI1R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfU1(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvOvfU1I4, OpCodeEnum::ConvOvfU1I8, OpCodeEnum::ConvOvfU1R4, OpCodeEnum::ConvOvfU1R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfI2(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvOvfI2I4, OpCodeEnum::ConvOvfI2I8, OpCodeEnum::ConvOvfI2R4, OpCodeEnum::ConvOvfI2R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfU2(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvOvfU2I4, OpCodeEnum::ConvOvfU2I8, OpCodeEnum::ConvOvfU2R4, OpCodeEnum::ConvOvfU2R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfI4(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::Nop, OpCodeEnum::ConvOvfI4I8, OpCodeEnum::ConvOvfI4R4, OpCodeEnum::ConvOvfI4R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfU4(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvOvfU4I4, OpCodeEnum::ConvOvfU4I8, OpCodeEnum::ConvOvfU4R4, OpCodeEnum::ConvOvfU4R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfI8(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvI8I4, OpCodeEnum::Nop, OpCodeEnum::ConvOvfI8R4, OpCodeEnum::ConvOvfI8R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfU8(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvOvfU8I4, OpCodeEnum::ConvOvfU8I8, OpCodeEnum::ConvOvfU8R4, OpCodeEnum::ConvOvfU8R8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfI(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvI8I4),
                                   utils::Platform::select_arch(OpCodeEnum::ConvOvfI4I8, OpCodeEnum::Nop),
                                   utils::Platform::select_arch(OpCodeEnum::ConvOvfI4R4, OpCodeEnum::ConvOvfI8R4),
                                   utils::Platform::select_arch(OpCodeEnum::ConvOvfI4R8, OpCodeEnum::ConvOvfI8R8)));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfU(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, utils::Platform::select_arch(OpCodeEnum::ConvOvfU4I4, OpCodeEnum::ConvOvfU8I4),
                                   utils::Platform::select_arch(OpCodeEnum::ConvOvfU4I8, OpCodeEnum::ConvOvfU8I8),
                                   utils::Platform::select_arch(OpCodeEnum::ConvOvfU4R4, OpCodeEnum::ConvOvfU8R4),
                                   utils::Platform::select_arch(OpCodeEnum::ConvOvfU4R8, OpCodeEnum::ConvOvfU8R8)));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfI1Un(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvOvfI1UnI4, OpCodeEnum::ConvOvfI1UnI8, OpCodeEnum::ConvOvfI1UnR4, OpCodeEnum::ConvOvfI1UnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfU1Un(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvOvfU1UnI4, OpCodeEnum::ConvOvfU1UnI8, OpCodeEnum::ConvOvfU1UnR4, OpCodeEnum::ConvOvfU1UnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfI2Un(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvOvfI2UnI4, OpCodeEnum::ConvOvfI2UnI8, OpCodeEnum::ConvOvfI2UnR4, OpCodeEnum::ConvOvfI2UnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfU2Un(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvOvfU2UnI4, OpCodeEnum::ConvOvfU2UnI8, OpCodeEnum::ConvOvfU2UnR4, OpCodeEnum::ConvOvfU2UnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfI4Un(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvOvfI4UnI4, OpCodeEnum::ConvOvfI4UnI8, OpCodeEnum::ConvOvfI4UnR4, OpCodeEnum::ConvOvfI4UnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfU4Un(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::Nop, OpCodeEnum::ConvOvfU4UnI8, OpCodeEnum::ConvOvfU4UnR4, OpCodeEnum::ConvOvfU4UnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfI8Un(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvI8U4, OpCodeEnum::ConvOvfI8UnI8, OpCodeEnum::ConvOvfI8UnR4, OpCodeEnum::ConvOvfI8UnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfU8Un(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, OpCodeEnum::ConvI8U4, OpCodeEnum::Nop, OpCodeEnum::ConvOvfU8UnR4, OpCodeEnum::ConvOvfU8UnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfIUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, utils::Platform::select_arch(OpCodeEnum::ConvOvfI4UnI4, OpCodeEnum::ConvI8U4),
                                   utils::Platform::select_arch(OpCodeEnum::ConvOvfI4UnI8, OpCodeEnum::ConvOvfI8UnI8),
                                   utils::Platform::select_arch(OpCodeEnum::ConvOvfI4UnR4, OpCodeEnum::ConvOvfI8UnR4),
                                   utils::Platform::select_arch(OpCodeEnum::ConvOvfI4UnR8, OpCodeEnum::ConvOvfI8UnR8)));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_ConvOvfUUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_conv(output, input, utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvI8U4),
                                   utils::Platform::select_arch(OpCodeEnum::ConvOvfU4UnI8, OpCodeEnum::Nop),
                                   utils::Platform::select_arch(OpCodeEnum::ConvOvfU4UnR4, OpCodeEnum::ConvOvfU8UnR4),
                                   utils::Platform::select_arch(OpCodeEnum::ConvOvfU4UnR8, OpCodeEnum::ConvOvfU8UnR8)));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Ceq(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_cmp_op(output, input, OpCodeEnum::CeqI4, OpCodeEnum::CeqI8, OpCodeEnum::CeqR4, OpCodeEnum::CeqR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Cgt(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_cmp_op(output, input, OpCodeEnum::CgtI4, OpCodeEnum::CgtI8, OpCodeEnum::CgtR4, OpCodeEnum::CgtR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Clt(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_cmp_op(output, input, OpCodeEnum::CltI4, OpCodeEnum::CltI8, OpCodeEnum::CltR4, OpCodeEnum::CltR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CgtUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_cmp_op(output, input, OpCodeEnum::CgtUnI4, OpCodeEnum::CgtUnI8, OpCodeEnum::CgtUnR4, OpCodeEnum::CgtUnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CltUn(GeneralInst* output, const GeneralInst* input)
{
    RET_ERR_ON_FAIL(transform_cmp_op(output, input, OpCodeEnum::CltUnI4, OpCodeEnum::CltUnI8, OpCodeEnum::CltUnR4, OpCodeEnum::CltUnR8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_InitObjI1(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::InitObjI1);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_InitObjI2(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::InitObjI2);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_InitObjI4(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::InitObjI4);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_InitObjI8(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::InitObjI8);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_InitObjRef(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(utils::Platform::select_arch(OpCodeEnum::InitObjI4, OpCodeEnum::InitObjI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_InitObjAny(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::InitObjAny);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CpObjI1(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::CpObjI1);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CpObjI2(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::CpObjI2);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CpObjI4(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::CpObjI4);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CpObjI8(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::CpObjI8);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CpObjRef(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(utils::Platform::select_arch(OpCodeEnum::CpObjI4, OpCodeEnum::CpObjI8));
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CpObjAny(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::CpObjAny);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdObjAny(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdObjAny);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StObjAny(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::StObjAny);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CastClass(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::CastClass);
    setup_inst_klass(output, input);
    assert(output->get_var_src_eval_stack_idx() == output->get_var_dst_eval_stack_idx());
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_IsInst(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::IsInst);
    setup_inst_klass(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Box(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::Box);
    setup_inst_klass(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_BoxRefInplace(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::BoxRefInplace);
    setup_inst_klass(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Unbox(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::Unbox);
    setup_inst_klass(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_UnboxAny(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::UnboxAny);
    setup_inst_klass(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_NewArr(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::NewArr);
    metadata::RtClass* ele_klass = input->get_class();
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, arr_klass, vm::ArrayClass::get_szarray_class_from_element_class(ele_klass));
    setup_inst_resolved_data(output, arr_klass);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdLen(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdLen);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Ldelema(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(input->contains_prefix_readonly() ? OpCodeEnum::LdelemaReadOnly : OpCodeEnum::Ldelema);
    setup_inst_klass(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdelemI1(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdelemI1);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdelemU1(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdelemU1);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdelemI2(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdelemI2);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdelemU2(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdelemU2);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdelemI4(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdelemI4);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdelemI8(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdelemI8);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdelemI(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdelemI);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdelemR4(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdelemR4);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdelemR8(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdelemR8);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdelemRef(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdelemRef);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdelemAny(GeneralInst* output, const GeneralInst* input)
{
    metadata::RtClass* ele_klass = input->get_class();
    output->set_opcode(vm::Class::is_reference_type(ele_klass) ? OpCodeEnum::LdelemAnyRef : OpCodeEnum::LdelemAnyVal);
    output->set_element_size(vm::Class::get_stack_location_size(ele_klass));
    setup_inst_klass(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StelemI1(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::StelemI1);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StelemI2(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::StelemI2);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StelemI4(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::StelemI4);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StelemI8(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::StelemI8);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StelemI(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::StelemI);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StelemR4(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::StelemR4);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StelemR8(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::StelemR8);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StelemRef(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::StelemRef);
    setup_inst_klass(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_StelemAny(GeneralInst* output, const GeneralInst* input)
{
    metadata::RtClass* ele_klass = input->get_class();
    output->set_opcode(vm::Class::is_reference_type(ele_klass) ? OpCodeEnum::StelemAnyRef : OpCodeEnum::StelemAnyVal);
    output->set_element_size(vm::Class::get_stack_location_size(ele_klass));
    setup_inst_klass(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_MkRefAny(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::MkRefAny);
    setup_inst_klass(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_RefAnyVal(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::RefAnyVal);
    setup_inst_klass(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_RefAnyType(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::RefAnyType);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LdToken(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LdToken);
    setup_inst_resolved_data(output, (const void*)input->get_runtime_handle().get_handle_without_type());
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Ckfinite(GeneralInst* output, const GeneralInst* input)
{
    const Variable* dst = input->get_var_dst();
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
    output->set_opcode(op);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_LocAlloc(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::LocAlloc);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_InitBlk(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::InitBlk);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CpBlk(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::CpBlk);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Ldfld(GeneralInst* output, const GeneralInst* input)
{
    size_t field_offset = output->get_field_offset();
    bool use_large_addressing = field_offset > UINT16_MAX;
    const metadata::RtFieldInfo* field = input->get_field();
    const Variable* dst = input->get_var_dst();
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
        if (input->contains_prefix_unaligned())
        {
            op = OpCodeEnum::LdfldI2Unaligned;
        }
        else
        {
            op = use_large_addressing ? OpCodeEnum::LdfldI2Large : OpCodeEnum::LdfldI2;
        }
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        if (input->contains_prefix_unaligned())
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
        if (input->contains_prefix_unaligned())
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
        if (input->contains_prefix_unaligned())
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
        if (input->contains_prefix_unaligned())
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
    output->set_opcode(op);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Ldvfld(GeneralInst* output, const GeneralInst* input)
{
    size_t field_offset = output->get_field_offset();
    bool use_large_addressing = field_offset > UINT16_MAX;
    const metadata::RtFieldInfo* field = input->get_field();
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
        if (input->contains_prefix_unaligned())
        {
            op = OpCodeEnum::LdvfldI2Unaligned;
        }
        else
        {
            op = use_large_addressing ? OpCodeEnum::LdvfldI2Large : OpCodeEnum::LdvfldI2;
        }
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        if (input->contains_prefix_unaligned())
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
        if (input->contains_prefix_unaligned())
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
        if (input->contains_prefix_unaligned())
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
        if (input->contains_prefix_unaligned())
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

    if (output->get_field_offset() == 0 && output->get_var_src_eval_stack_idx() == output->get_var_dst_eval_stack_idx())
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

    output->set_opcode(op);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Ldflda(GeneralInst* output, const GeneralInst* input)
{
    size_t field_offset = output->get_field_offset();
    bool use_large_addressing = field_offset > UINT16_MAX;
    output->set_opcode(use_large_addressing ? OpCodeEnum::LdfldaLarge : OpCodeEnum::Ldflda);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Stfld(GeneralInst* output, const GeneralInst* input)
{
    size_t field_offset = output->get_field_offset();
    bool use_large_addressing = field_offset > UINT16_MAX;
    const metadata::RtFieldInfo* field = input->get_field();
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
        if (input->contains_prefix_unaligned())
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
        if (input->contains_prefix_unaligned())
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
        if (input->contains_prefix_unaligned())
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
        if (input->contains_prefix_unaligned())
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
    output->set_opcode(op);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Ldsfld(GeneralInst* output, const GeneralInst* input)
{
    const metadata::RtFieldInfo* field = input->get_field();
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
    output->set_opcode(op);
    setup_inst_resolved_data(output, field);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Ldsflda(GeneralInst* output, const GeneralInst* input)
{
    const metadata::RtFieldInfo* field = input->get_field();
    if (vm::Field::is_static_rva(field))
    {
        output->set_opcode(OpCodeEnum::LdsfldRvaData);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const uint8_t*, rva_data, vm::Field::get_field_rva_data(field));
        setup_inst_resolved_data(output, rva_data);
    }
    else
    {
        output->set_opcode(OpCodeEnum::Ldsflda);
        setup_inst_resolved_data(output, field);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Stsfld(GeneralInst* output, const GeneralInst* input)
{
    const metadata::RtFieldInfo* field = input->get_field();
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
    output->set_opcode(op);
    setup_inst_resolved_data(output, field);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Call(GeneralInst* output, const GeneralInst* input)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_call_methods(output, input));
    if (!handled)
    {
        output->set_opcode(OpCodeEnum::CallInterp);
        setup_inst_method(output, input);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CallVirt(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::CallVirtInterp);
    setup_inst_method(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CallInternalCall(GeneralInst* output, const GeneralInst* input)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_call_methods(output, input));
    if (!handled)
    {
        output->set_opcode(OpCodeEnum::CallInternalCall);
        setup_inst_method(output, input);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CallIntrinsic(GeneralInst* output, const GeneralInst* input)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_call_methods(output, input));
    if (!handled)
    {
        output->set_opcode(OpCodeEnum::CallIntrinsic);
        setup_inst_method(output, input);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CallPInvoke(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::CallPInvoke);
    setup_inst_method(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CallAot(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::CallAot);
    setup_inst_method(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_CallRuntimeImplemented(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::CallRuntimeImplemented);
    setup_inst_method(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Calli(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::CalliInterp);
    setup_inst_resolved_data(output, input->get_method_sig());
    // output->set_frame_base(input->get_frame_base());
    //  method_idx stores in arg3, it has been setup on start.
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_NewObj(GeneralInst* output, const GeneralInst* input)
{
    const metadata::RtMethodInfo* method = input->get_method();
    OpCodeEnum op = vm::Class::is_value_type(method->parent) ? OpCodeEnum::NewValueTypeInterp : OpCodeEnum::NewObjInterp;
    output->set_opcode(op);
    setup_inst_method(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_NewObjInternalCall(GeneralInst* output, const GeneralInst* input)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_newobj_methods(output, input));
    if (!handled)
    {
        output->set_opcode(OpCodeEnum::NewObjInternalCall);
        setup_inst_method(output, input);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_NewObjIntrinsic(GeneralInst* output, const GeneralInst* input)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_newobj_methods(output, input));
    if (!handled)
    {
        output->set_opcode(OpCodeEnum::NewObjIntrinsic);
        setup_inst_method(output, input);
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_NewObjAot(GeneralInst* output, const GeneralInst* input)
{
    const metadata::RtMethodInfo* method = input->get_method();
    OpCodeEnum op = vm::Class::is_value_type(method->parent) ? OpCodeEnum::NewValueTypeAot : OpCodeEnum::NewObjAot;
    output->set_opcode(op);
    setup_inst_method(output, input);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Ldftn(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::Ldftn);
    setup_inst_resolved_data(output, input->get_method());
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Ldvirtftn(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::Ldvirtftn);
    setup_inst_resolved_data(output, input->get_method());
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Throw(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::Throw);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Rethrow(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::Rethrow);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_Leave(GeneralInst* output, const GeneralInst* input)
{
    BasicBlock* cur_bb = _cur_bb;
    const BasicBlock* leave_target = input->get_leave_target();
    switch (get_leave_surrounding_block_type(cur_bb, leave_target))
    {
    case LeaveSurroundingBlockType::None:
        output->set_opcode(OpCodeEnum::Br);
        output->update_branch_target(leave_target);
        break;
    case LeaveSurroundingBlockType::Try:
    {
        utils::NotFreeList<size_t> finally_clause_idx = find_finally_clause_idx_of_leave_target(cur_bb, leave_target);
        if (finally_clause_idx.empty())
        {
            output->set_opcode(OpCodeEnum::Br);
            output->update_branch_target(leave_target);
        }
        else
        {
            output->set_opcode(OpCodeEnum::LeaveTryWithFinally);
            output->set_first_finally_clause_index(finally_clause_idx[0]);
            output->set_finally_clauses_count(finally_clause_idx.size());
            output->update_leave_target(leave_target);
        }
        break;
    }
    case LeaveSurroundingBlockType::CatchOrFilter:
    {
        utils::NotFreeList<size_t> finally_clause_idx = find_finally_clause_idx_of_leave_target(cur_bb, leave_target);
        if (finally_clause_idx.empty())
        {
            output->set_opcode(OpCodeEnum::LeaveCatchWithoutFinally);
            output->update_leave_target(leave_target);
        }
        else
        {
            output->set_opcode(OpCodeEnum::LeaveCatchWithFinally);
            output->set_first_finally_clause_index(finally_clause_idx[0]);
            output->set_finally_clauses_count(finally_clause_idx.size());
            output->update_leave_target(leave_target);
        }
        break;
    }
    }
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_EndFilter(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::EndFilter);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_EndFinallyOrFault(GeneralInst* output, const GeneralInst* input)
{
    BasicBlock* cur_bb = _cur_bb;
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
    output->set_opcode(op);
    RET_VOID_OK();
}

RtResultVoid Transformer::emit_GetEnumLongHashCode(GeneralInst* output, const GeneralInst* input)
{
    output->set_opcode(OpCodeEnum::GetEnumLongHashCode);
    RET_VOID_OK();
}

RtResultVoid Transformer::build_exception_clauses(RtInterpMethodInfo* interp_method)
{
    const metadata::RtMethodBody* method_body = get_method_body();
    const auto& src_clauses = method_body->exception_clauses;
    size_t exception_clause_count = src_clauses.size();

    if (exception_clause_count > UINT8_MAX)
    {
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    metadata::RtModuleDef* ass = get_module();
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
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, final_type_sig,
                                                    ass->get_typesig_by_type_def_ref_spec_token(token, get_generic_container_context(), get_generic_context()));
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
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const BasicBlock*, block, get_branch_target_bb(static_cast<size_t>(il_offset)));

    RET_OK(static_cast<uint32_t>(block->ir_offset));
}

RtResultVoid Transformer::build_codes(RtInterpMethodInfo* interp_method)
{
    metadata::RtModuleDef* mod = get_module();

    // First pass: calculate total size and set offsets
    size_t total_ir_size = 0;
    BasicBlock* cur_bb = _basic_blocks;

    size_t total_ir_count = 0;
    for (BasicBlock* cur_bb = _basic_blocks; cur_bb != nullptr; cur_bb = cur_bb->next_bb)
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
        for (BasicBlock* cur_bb = _basic_blocks; cur_bb != nullptr; cur_bb = cur_bb->next_bb)
        {
            for (const GeneralInst* inst : cur_bb->insts)
            {
                codes_cur = ll::OpCodes::write_instruction_to_data(codes_cur, *inst);
            }
        }
    }
    else
    {
        metadata::IR2ILMapEntry* ir2il_map_entries = mod->get_mem_pool().calloc_any<metadata::IR2ILMapEntry>(total_ir_count);
        size_t ir_index = 0;
        for (BasicBlock* cur_bb = _basic_blocks; cur_bb != nullptr; cur_bb = cur_bb->next_bb)
        {
            for (const GeneralInst* inst : cur_bb->insts)
            {
                codes_cur = ll::OpCodes::write_instruction_to_data(codes_cur, *inst);
                ir2il_map_entries[ir_index++] = {inst->get_il_offset(), inst->get_ir_offset()};
            }
        }
        pdb_image->add_ir2il_map_for_method(get_method_info(), {utils::Span<metadata::IR2ILMapEntry>{ir2il_map_entries, total_ir_count}});
    }

    assert(codes_cur == codes + total_ir_size);
    RET_VOID_OK();
}

RtResult<const RtInterpMethodInfo*> Transformer::build_interp_method_info()
{
    const metadata::RtMethodInfo* method = get_method_info();
    metadata::RtModuleDef* mod = get_module();
    alloc::MemPool& pool = mod->get_mem_pool();

    RtInterpMethodInfo* interp_method = pool.malloc_any_zeroed<RtInterpMethodInfo>();

    if (!_resolved_datas.empty())
    {
        const void** resolved_datas = (const void**)pool.calloc_any<const void*>(_resolved_datas.size());
        std::memcpy(resolved_datas, _resolved_datas.data(), _resolved_datas.size() * sizeof(const void*));
        interp_method->resolved_datas = resolved_datas;
    }

    interp_method->total_arg_and_local_stack_object_size = static_cast<uint16_t>(get_total_arg_and_local_stack_object_size());
    interp_method->max_stack_object_size = static_cast<uint16_t>(get_max_stack_size());
    interp_method->init_locals = need_init_locals();

    RET_ERR_ON_FAIL(build_codes(interp_method));
    RET_ERR_ON_FAIL(build_exception_clauses(interp_method));

    RET_OK(interp_method);
}

} // namespace interp
} // namespace leanclr

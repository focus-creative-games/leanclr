#include "transformer.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/reflection.h"
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

void Transformer::setup_inst_resolved_data(InstructionOperands& output, const void* data)
{
    size_t index = get_resolved_data_index(data);
    output.resolved_index = index;
}

void Transformer::setup_inst_klass(InstructionOperands& output)
{
    metadata::RtClass* klass = output.klass;
    setup_inst_resolved_data(output, klass);
}

void Transformer::setup_inst_method(InstructionOperands& output)
{
    const metadata::RtMethodInfo* method = output.method;
    setup_inst_resolved_data(output, method);
}

utils::NotFreeList<size_t> Transformer::find_finally_clause_idx_of_leave_target(const BasicBlock* leave_src, const BasicBlock* leave_target)
{
    uint32_t il_src = static_cast<uint32_t>(leave_src->il_begin_offset);
    uint32_t il_target = static_cast<uint32_t>(leave_target->il_begin_offset);
    utils::NotFreeList<size_t> result(_pool);

    auto* method_body = _method_body;
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

    auto* method_body = _method_body;
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

    auto* method_body = _method_body;
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

RtResult<bool> Transformer::transform_special_call_methods(InstructionOperands& output)
{
    const metadata::RtMethodInfo* method = output.method;
    const metadata::RtClass* klass = method->parent;

    if (!klass->image->is_corlib())
    {
        RET_OK(false);
    }

    output.opcode = OpCodeEnum::Illegal;

    const vm::CorLibTypes& corlib_types = vm::Class::get_corlib_types();
    const char* klass_name = klass->name;
    const char* method_name = method->name;
    size_t param_count = static_cast<size_t>(method->parameter_count);

    const Variable* const* params = output.parameters;

    if (klass == corlib_types.cls_object)
    {
        if (std::strcmp(method_name, STR_CTOR) == 0)
        {
            output.opcode = OpCodeEnum::Nop;
        }
    }
    else if (klass == corlib_types.cls_array)
    {
        if (std::strcmp(method_name, "UnsafeMov") == 0)
        {
            output.opcode = OpCodeEnum::Nop;
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
                    output.opcode = utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8I4);
                    output.first = arg_value;
                    output.destination = arg_this;
                    break;
                case metadata::RtElementType::I8:
                case metadata::RtElementType::U8:
                case metadata::RtElementType::I:
                case metadata::RtElementType::U:
                case metadata::RtElementType::Ptr:
                    output.opcode = utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8);
                    output.first = arg_value;
                    output.destination = arg_this;
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
                    output.opcode = utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8U4);
                    output.first = arg_value;
                    output.destination = arg_this;
                    break;
                case metadata::RtElementType::I8:
                case metadata::RtElementType::U8:
                case metadata::RtElementType::I:
                case metadata::RtElementType::U:
                case metadata::RtElementType::Ptr:
                    output.opcode = utils::Platform::select_arch(OpCodeEnum::StIndI4, OpCodeEnum::StIndI8);
                    output.first = arg_value;
                    output.destination = arg_this;
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
            output.opcode = offset <= INT16_MAX ? OpCodeEnum::LdcI4I2 : OpCodeEnum::LdcI4I4;
            output.immediate = static_cast<uint32_t>(offset);
            output.destination = output.destination;
        }
    }
    else if (klass == corlib_types.cls_appdomain)
    {
        if (std::strcmp(method_name, "IsAppXModel") == 0)
        {
            output.opcode = OpCodeEnum::LdcI4I2;
            output.immediate = static_cast<uint32_t>(0); // false
            output.destination = output.destination;
        }
    }
    else if (std::strcmp(klass_name, "ByReference`1") == 0)
    {
        if (std::strcmp(method_name, STR_CTOR) == 0)
        {
            output.opcode = OpCodeEnum::Nop;
        }
        else if (std::strcmp(method_name, "get_Value") == 0)
        {
            const Variable* arg_this = params[0];
            const Variable* dst = output.destination;
            output.opcode = utils::Platform::select_arch(OpCodeEnum::LdIndI4, OpCodeEnum::LdIndI8);
            output.first = arg_this;
            output.destination = dst;
        }
    }
    else if (std::strcmp(klass_name, "JitHelpers") == 0)
    {
        if (std::strcmp(method_name, "UnsafeCast") == 0 || std::strcmp(method_name, "UnsafeEnumCast") == 0 || std::strcmp(method_name, "UnsafeEnumCastLong"))
        {
            output.opcode = OpCodeEnum::Nop;
        }
    }

    RET_OK(output.opcode != OpCodeEnum::Illegal);
}

RtResult<bool> Transformer::transform_special_newobj_methods(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::Illegal;
    const metadata::RtMethodInfo* method = output.method;
    const metadata::RtClass* klass = method->parent;
    if (!klass->image->is_corlib())
    {
        RET_OK(false);
    }

    const vm::CorLibTypes& corlib_types = vm::Class::get_corlib_types();
    const char* klass_name = klass->name;
    size_t param_count = static_cast<size_t>(method->parameter_count);

    const Variable* const* params = output.parameters;

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
                output.opcode = utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvI8I4);
                output.first = arg_value;
                output.destination = arg_value;
                break;
            case metadata::RtElementType::I8:
            case metadata::RtElementType::U8:
            case metadata::RtElementType::I:
            case metadata::RtElementType::U:
            case metadata::RtElementType::Ptr:
                output.opcode = OpCodeEnum::Nop;
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
                output.opcode = utils::Platform::select_arch(OpCodeEnum::Nop, OpCodeEnum::ConvI8U4);
                output.first = arg_value;
                output.destination = arg_value;
                break;
            case metadata::RtElementType::I8:
            case metadata::RtElementType::U8:
            case metadata::RtElementType::I:
            case metadata::RtElementType::U:
            case metadata::RtElementType::Ptr:
                output.opcode = OpCodeEnum::Nop;
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
        output.opcode = OpCodeEnum::Nop;
    }

    RET_OK(output.opcode != OpCodeEnum::Illegal);
}

RtResultVoid Transformer::emit_InitLocals(InstructionOperands& output)
{
    if (output.size == 0)
        output.opcode = OpCodeEnum::Nop;
    else
        output.opcode = OpCodeEnum::InitLocals;
    return append_instruction(output);
}

RtResultVoid Transformer::emit_LdLoc(InstructionOperands& output)
{
    const Variable* src = output.first;
    switch (src->reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
        output.opcode = OpCodeEnum::LdLocI1;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        output.opcode = OpCodeEnum::LdLocU1;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I2:
        output.opcode = OpCodeEnum::LdLocI2;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        output.opcode = OpCodeEnum::LdLocU2;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I4:
    case metadata::RtArgOrLocOrFieldReduceType::R4:
        output.opcode = OpCodeEnum::LdLocI4;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I8:
    case metadata::RtArgOrLocOrFieldReduceType::R8:
        output.opcode = OpCodeEnum::LdLocI8;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I:
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
        output.opcode = utils::Platform::select_arch(OpCodeEnum::LdLocI4, OpCodeEnum::LdLocI8);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Other:
        output.opcode = OpCodeEnum::LdLocAny;
        output.size = src->stack_object_size;
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_StLoc(InstructionOperands& output)
{
    const Variable* dst = output.destination;
    switch (dst->reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
    case metadata::RtArgOrLocOrFieldReduceType::U1:
        output.opcode = OpCodeEnum::StLocI1;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I2:
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        output.opcode = OpCodeEnum::StLocI2;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I4:
    case metadata::RtArgOrLocOrFieldReduceType::R4:
        output.opcode = OpCodeEnum::StLocI4;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I8:
    case metadata::RtArgOrLocOrFieldReduceType::R8:
        output.opcode = OpCodeEnum::StLocI8;
        break;
    case metadata::RtArgOrLocOrFieldReduceType::I:
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
        output.opcode = utils::Platform::select_arch(OpCodeEnum::StLocI4, OpCodeEnum::StLocI8);
        break;
    case metadata::RtArgOrLocOrFieldReduceType::Other:
        output.opcode = OpCodeEnum::StLocAny;
        output.size = dst->stack_object_size;
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_LdcI4(InstructionOperands& output)
{
    int32_t value = static_cast<int32_t>(output.immediate);
    if (value >= INT16_MIN && value <= INT16_MAX)
    {
        output.opcode = OpCodeEnum::LdcI4I2;
    }
    else
    {
        output.opcode = OpCodeEnum::LdcI4I4;
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_LdcI8(InstructionOperands& output)
{
    int64_t value = static_cast<int64_t>(output.immediate);
    if (value >= INT16_MIN && value <= INT16_MAX)
    {
        output.opcode = OpCodeEnum::LdcI8I2;
    }
    else if (value >= INT32_MIN && value <= INT32_MAX)
    {
        output.opcode = OpCodeEnum::LdcI8I4;
    }
    else
    {
        output.opcode = OpCodeEnum::LdcI8I8;
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_LdStr(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::LdStr;
    setup_inst_resolved_data(output, output.string_value);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Dup(InstructionOperands& output)
{
    const Variable* src = output.first;
    switch (src->data_type)
    {
    case RtEvalStackDataType::I4:
    case RtEvalStackDataType::R4:
        output.opcode = OpCodeEnum::LdLocI4;
        break;
    case RtEvalStackDataType::I8:
    case RtEvalStackDataType::R8:
        output.opcode = OpCodeEnum::LdLocI8;
        break;
    case RtEvalStackDataType::RefOrPtr:
        output.opcode = utils::Platform::select_arch(OpCodeEnum::LdLocI4, OpCodeEnum::LdLocI8);
        break;
    case RtEvalStackDataType::Other:
        output.opcode = OpCodeEnum::LdLocAny;
        output.size = src->stack_object_size;
        break;
    default:
        RET_ERR(core::RtErr::ExecutionEngine);
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Ret(InstructionOperands& output)
{
    const Variable* ret = _ret_var;
    if (ret == nullptr)
    {
        output.opcode = OpCodeEnum::RetVoid;
    }
    else if (output.first->eval_stack_offset == 0)
    {
        output.opcode = OpCodeEnum::RetNop;
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
            output.size = ret->stack_object_size;
            op = OpCodeEnum::RetAny;
            break;
        default:
            RET_ERR(core::RtErr::ExecutionEngine);
        }
        output.opcode = op;
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Br(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::Br;
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Switch(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::Switch;
    const auto targets = output.switch_targets;
    output.switch_targets = {targets.targets, targets.count};
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Neg(InstructionOperands& output)
{
    const Variable* dst = output.destination;
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
    output.opcode = op;
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Not(InstructionOperands& output)
{
    const Variable* dst = output.destination;
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
    output.opcode = op;
    return append_instruction(output);
}

RtResultVoid Transformer::emit_CastClass(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::CastClass;
    setup_inst_klass(output);
    assert(output.first->eval_stack_offset == output.destination->eval_stack_offset);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_IsInst(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::IsInst;
    setup_inst_klass(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Box(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::Box;
    setup_inst_klass(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_BoxRefInplace(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::BoxRefInplace;
    setup_inst_klass(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Unbox(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::Unbox;
    setup_inst_klass(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_UnboxAny(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::UnboxAny;
    setup_inst_klass(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_NewArr(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::NewArr;
    metadata::RtClass* ele_klass = output.klass;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, arr_klass, vm::ArrayClass::get_szarray_class_from_element_class(ele_klass));
    setup_inst_resolved_data(output, arr_klass);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Ldelema(InstructionOperands& output)
{
    output.opcode = has_prefix(output.prefix, il::OpCodePrefix::ReadOnly) ? OpCodeEnum::LdelemaReadOnly : OpCodeEnum::Ldelema;
    setup_inst_klass(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_LdelemAny(InstructionOperands& output)
{
    metadata::RtClass* ele_klass = output.klass;
    output.opcode = vm::Class::is_reference_type(ele_klass) ? OpCodeEnum::LdelemAnyRef : OpCodeEnum::LdelemAnyVal;
    output.element_size = vm::Class::get_stack_location_size(ele_klass);
    setup_inst_klass(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_StelemAny(InstructionOperands& output)
{
    metadata::RtClass* ele_klass = output.klass;
    output.opcode = vm::Class::is_reference_type(ele_klass) ? OpCodeEnum::StelemAnyRef : OpCodeEnum::StelemAnyVal;
    output.element_size = vm::Class::get_stack_location_size(ele_klass);
    setup_inst_klass(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_MkRefAny(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::MkRefAny;
#if defined(LEANCLR_BCL_CORECLR_NET10)
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const void*, handle, vm::Reflection::get_net10_method_table(output.klass->by_val));
    setup_inst_resolved_data(output, handle);
#else
    setup_inst_klass(output);
#endif
    return append_instruction(output);
}

RtResultVoid Transformer::emit_RefAnyVal(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::RefAnyVal;
#if defined(LEANCLR_BCL_CORECLR_NET10)
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const void*, handle, vm::Reflection::get_net10_method_table(output.klass->by_val));
    setup_inst_resolved_data(output, handle);
#else
    setup_inst_klass(output);
#endif
    return append_instruction(output);
}

RtResultVoid Transformer::emit_LdToken(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::LdToken;
    auto handle = metadata::RtEncodedRuntimeHandle::decode(output.runtime_handle);
    const void* value = handle.value;
#if defined(LEANCLR_BCL_CORECLR_NET10)
    if (handle.is_type())
    {
        UNWRAP_OR_RET_ERR_ON_FAIL(value, vm::Reflection::get_net10_method_table(handle.typeSig));
    }
#endif
    setup_inst_resolved_data(output, value);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Ckfinite(InstructionOperands& output)
{
    const Variable* dst = output.destination;
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
    output.opcode = op;
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Ldfld(InstructionOperands& output)
{
    size_t field_offset = vm::Field::get_field_offset_includes_object_header_for_reference_type(output.field);
    bool use_large_addressing = field_offset > UINT16_MAX;
    const metadata::RtFieldInfo* field = output.field;
    const Variable* dst = output.destination;
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
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
        {
            op = OpCodeEnum::LdfldI2Unaligned;
        }
        else
        {
            op = use_large_addressing ? OpCodeEnum::LdfldI2Large : OpCodeEnum::LdfldI2;
        }
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
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
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
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
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
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
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
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
    output.opcode = op;
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Ldvfld(InstructionOperands& output)
{
    size_t field_offset = vm::Field::get_field_offset_includes_object_header_for_reference_type(output.field);
    bool use_large_addressing = field_offset > UINT16_MAX;
    const metadata::RtFieldInfo* field = output.field;
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
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
        {
            op = OpCodeEnum::LdvfldI2Unaligned;
        }
        else
        {
            op = use_large_addressing ? OpCodeEnum::LdvfldI2Large : OpCodeEnum::LdvfldI2;
        }
        break;
    case metadata::RtArgOrLocOrFieldReduceType::U2:
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
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
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
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
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
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
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
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

    if (vm::Field::get_field_offset_includes_object_header_for_reference_type(output.field) == 0 &&
        output.first->eval_stack_offset == output.destination->eval_stack_offset)
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

    output.opcode = op;
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Ldflda(InstructionOperands& output)
{
    size_t field_offset = vm::Field::get_field_offset_includes_object_header_for_reference_type(output.field);
    bool use_large_addressing = field_offset > UINT16_MAX;
    output.opcode = use_large_addressing ? OpCodeEnum::LdfldaLarge : OpCodeEnum::Ldflda;
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Stfld(InstructionOperands& output)
{
    size_t field_offset = vm::Field::get_field_offset_includes_object_header_for_reference_type(output.field);
    bool use_large_addressing = field_offset > UINT16_MAX;
    const metadata::RtFieldInfo* field = output.field;
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
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
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
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
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
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
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
        if (has_prefix(output.prefix, il::OpCodePrefix::Unaligned))
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
    output.opcode = op;
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Ldsfld(InstructionOperands& output)
{
    const metadata::RtFieldInfo* field = output.field;
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
    output.opcode = op;
    setup_inst_resolved_data(output, field);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Ldsflda(InstructionOperands& output)
{
    const metadata::RtFieldInfo* field = output.field;
    if (vm::Field::is_static_rva(field))
    {
        output.opcode = OpCodeEnum::LdsfldRvaData;
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const uint8_t*, rva_data, vm::Field::get_field_rva_data(field));
        setup_inst_resolved_data(output, rva_data);
    }
    else
    {
        output.opcode = OpCodeEnum::Ldsflda;
        setup_inst_resolved_data(output, field);
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Stsfld(InstructionOperands& output)
{
    const metadata::RtFieldInfo* field = output.field;
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
    output.opcode = op;
    setup_inst_resolved_data(output, field);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Call(InstructionOperands& output)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_call_methods(output));
    if (!handled)
    {
        output.opcode = OpCodeEnum::CallInterp;
        setup_inst_method(output);
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_CallVirt(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::CallVirtInterp;
    setup_inst_method(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_CallInternalCall(InstructionOperands& output)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_call_methods(output));
    if (!handled)
    {
        output.opcode = OpCodeEnum::CallInternalCall;
        setup_inst_method(output);
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_CallIntrinsic(InstructionOperands& output)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_call_methods(output));
    if (!handled)
    {
        output.opcode = OpCodeEnum::CallIntrinsic;
        setup_inst_method(output);
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_CallPInvoke(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::CallPInvoke;
    setup_inst_method(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_CallAot(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::CallAot;
    setup_inst_method(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_CallRuntimeImplemented(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::CallRuntimeImplemented;
    setup_inst_method(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Calli(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::CalliInterp;
    setup_inst_resolved_data(output, output.signature);
    // output.frame_base = output.frame_base;
    //  method_idx stores in arg3, it has been setup on start.
    return append_instruction(output);
}

RtResultVoid Transformer::emit_NewObj(InstructionOperands& output)
{
    const metadata::RtMethodInfo* method = output.method;
    OpCodeEnum op = vm::Class::is_value_type(method->parent) ? OpCodeEnum::NewValueTypeInterp : OpCodeEnum::NewObjInterp;
    output.opcode = op;
    setup_inst_method(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_NewObjInternalCall(InstructionOperands& output)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_newobj_methods(output));
    if (!handled)
    {
        output.opcode = OpCodeEnum::NewObjInternalCall;
        setup_inst_method(output);
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_NewObjIntrinsic(InstructionOperands& output)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, handled, transform_special_newobj_methods(output));
    if (!handled)
    {
        output.opcode = OpCodeEnum::NewObjIntrinsic;
        setup_inst_method(output);
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_NewObjAot(InstructionOperands& output)
{
    const metadata::RtMethodInfo* method = output.method;
    OpCodeEnum op = vm::Class::is_value_type(method->parent) ? OpCodeEnum::NewValueTypeAot : OpCodeEnum::NewObjAot;
    output.opcode = op;
    setup_inst_method(output);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Ldftn(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::Ldftn;
    setup_inst_resolved_data(output, output.method);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Ldvirtftn(InstructionOperands& output)
{
    output.opcode = OpCodeEnum::Ldvirtftn;
    setup_inst_resolved_data(output, output.method);
    return append_instruction(output);
}

RtResultVoid Transformer::emit_Leave(InstructionOperands& output)
{
    BasicBlock* cur_bb = _cur_bb;
    const BasicBlock* leave_target = output.target;
    switch (get_leave_surrounding_block_type(cur_bb, leave_target))
    {
    case LeaveSurroundingBlockType::None:
        output.opcode = OpCodeEnum::Br;
        output.target = leave_target;
        break;
    case LeaveSurroundingBlockType::Try:
    {
        utils::NotFreeList<size_t> finally_clause_idx = find_finally_clause_idx_of_leave_target(cur_bb, leave_target);
        if (finally_clause_idx.empty())
        {
            output.opcode = OpCodeEnum::Br;
            output.target = leave_target;
        }
        else
        {
            output.opcode = OpCodeEnum::LeaveTryWithFinally;
            output.first_finally = finally_clause_idx[0];
            output.finally_count = finally_clause_idx.size();
            output.target = leave_target;
        }
        break;
    }
    case LeaveSurroundingBlockType::CatchOrFilter:
    {
        utils::NotFreeList<size_t> finally_clause_idx = find_finally_clause_idx_of_leave_target(cur_bb, leave_target);
        if (finally_clause_idx.empty())
        {
            output.opcode = OpCodeEnum::LeaveCatchWithoutFinally;
            output.target = leave_target;
        }
        else
        {
            output.opcode = OpCodeEnum::LeaveCatchWithFinally;
            output.first_finally = finally_clause_idx[0];
            output.finally_count = finally_clause_idx.size();
            output.target = leave_target;
        }
        break;
    }
    }
    return append_instruction(output);
}

RtResultVoid Transformer::emit_EndFinallyOrFault(InstructionOperands& output)
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
    output.opcode = op;
    return append_instruction(output);
}

RtResultVoid Transformer::build_exception_clauses(RtInterpMethodInfo* interp_method)
{
    const metadata::RtMethodBody* method_body = _method_body;
    const auto& src_clauses = method_body->exception_clauses;
    size_t exception_clause_count = src_clauses.size();

    if (exception_clause_count > UINT8_MAX)
    {
        RET_ERR(core::RtErr::ExecutionEngine);
    }

    metadata::RtModuleDef* ass = _mod;
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
                                                    ass->get_typesig_by_type_def_ref_spec_token(token, _generic_container_context, _generic_context));
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

RtResult<const RtInterpMethodInfo*> Transformer::build_interp_method_info()
{
    const metadata::RtMethodInfo* method = _method;
    metadata::RtModuleDef* mod = _mod;
    alloc::MemPool& pool = mod->get_mem_pool();

    if (_total_arg_and_local_stack_object_size > UINT16_MAX || _max_eval_stack_size > UINT16_MAX)
        RET_ERR(RtErr::ExecutionEngine);

    RtInterpMethodInfo* interp_method = pool.malloc_any_zeroed<RtInterpMethodInfo>();

    if (!_resolved_datas.empty())
    {
        const void** resolved_datas = (const void**)pool.calloc_any<const void*>(_resolved_datas.size());
        std::memcpy(resolved_datas, _resolved_datas.data(), _resolved_datas.size() * sizeof(const void*));
        interp_method->resolved_datas = resolved_datas;
    }

    interp_method->total_arg_and_local_stack_object_size = static_cast<uint16_t>(_total_arg_and_local_stack_object_size);
    interp_method->max_stack_object_size = static_cast<uint16_t>(_max_eval_stack_size);
    interp_method->init_locals = need_init_locals();

    RET_ERR_ON_FAIL(build_codes(interp_method));
    RET_ERR_ON_FAIL(build_exception_clauses(interp_method));

    RET_OK(interp_method);
}

} // namespace interp
} // namespace leanclr

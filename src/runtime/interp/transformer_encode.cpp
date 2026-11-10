#include "transformer.h"
#include "metadata/module_def.h"
#include "utils/platform.h"
#include "vm/field.h"
#include <climits>
#include <cstring>
#include <new>

namespace leanclr
{
namespace interp
{
using ll::OpCodeEnum;

RtResultVoid Transformer::emit_numeric(InstructionOperands& operands, const NumericOpcodes& opcodes, RtEvalStackDataType type)
{
    switch (type)
    {
    case RtEvalStackDataType::I4:
        operands.opcode = opcodes.i4;
        break;
    case RtEvalStackDataType::I8:
        operands.opcode = opcodes.i8;
        break;
    case RtEvalStackDataType::R4:
        operands.opcode = opcodes.r4;
        break;
    case RtEvalStackDataType::R8:
        operands.opcode = opcodes.r8;
        break;
    case RtEvalStackDataType::RefOrPtr:
        operands.opcode = utils::Platform::select_arch(opcodes.i4, opcodes.i8);
        break;
    default:
        RET_ERR(RtErr::ExecutionEngine);
    }
    return append_instruction(operands);
}

RtResultVoid Transformer::append_instruction(InstructionOperands& operands)
{
    if (operands.opcode == OpCodeEnum::Nop)
        RET_VOID_OK();
    if (operands.opcode == OpCodeEnum::Illegal || operands.opcode >= OpCodeEnum::__Count)
        RET_ERR(RtErr::ExecutionEngine);

    if (operands.field)
    {
        operands.field_offset = vm::Field::get_field_offset_includes_object_header_for_reference_type(operands.field);
        UNWRAP_OR_RET_ERR_ON_FAIL(operands.field_size, vm::Field::get_field_size(operands.field));
    }
    if (!ll::OpCodes::validate_operands(operands.opcode, operands))
        RET_ERR(RtErr::ExecutionEngine);

    size_t byte_size = ll::OpCodes::get_instruction_size(operands.opcode);
    const bool is_switch = operands.opcode == OpCodeEnum::Switch;
    if (is_switch)
    {
        if (operands.switch_targets.count > (INT32_MAX - byte_size) / sizeof(int32_t))
            RET_ERR(RtErr::ExecutionEngine);
        byte_size += operands.switch_targets.count * sizeof(int32_t);
    }
    const size_t allocation_size = utils::MemOp::align_up(sizeof(EncodedInstruction) + byte_size, alignof(EncodedInstruction));
    void* storage = _pool->malloc_zeroed(allocation_size, alignof(EncodedInstruction));
    auto* inst = new (storage) EncodedInstruction{byte_size, _cur_il_offset, 0};
    uint8_t* end = ll::OpCodes::write_instruction_to_data(inst->data(), operands.opcode, operands);
    assert(end == inst->data() + ll::OpCodes::get_instruction_size(operands.opcode));

    const size_t branch_offset = ll::OpCodes::get_branch_patch_offset(operands.opcode);
    if (branch_offset != static_cast<size_t>(-1))
    {
        if (!operands.target)
            RET_ERR(RtErr::ExecutionEngine);
        _branch_patches.push_back({inst, operands.target, branch_offset});
    }
    if (is_switch)
    {
        for (size_t i = 0; i < operands.switch_targets.count; ++i)
        {
            const BasicBlock* target = operands.switch_targets.targets[i];
            if (!target)
                RET_ERR(RtErr::ExecutionEngine);
            _branch_patches.push_back({inst, target, sizeof(ll::Switch) + i * sizeof(int32_t)});
        }
    }
    _cur_bb->insts.push_back(inst);
    RET_VOID_OK();
}

RtResultVoid Transformer::build_codes(RtInterpMethodInfo* interp_method)
{
    size_t byte_size = 0;
    size_t instruction_count = 0;
    for (BasicBlock* block = _basic_blocks; block; block = block->next_bb)
    {
        block->ir_offset = static_cast<int32_t>(byte_size);
        instruction_count += block->insts.size();
        for (EncodedInstruction* inst : block->insts)
        {
            if (inst->byte_size > INT32_MAX - byte_size)
                RET_ERR(RtErr::ExecutionEngine);
            inst->ir_offset = static_cast<int32_t>(byte_size);
            byte_size += inst->byte_size;
        }
    }

    // All block offsets are now stable. Patch only fields registered at emission.
    for (const BranchPatch& patch : _branch_patches)
    {
        const int64_t distance = static_cast<int64_t>(patch.target->ir_offset) - patch.source->ir_offset;
        if (distance < INT32_MIN || distance > INT32_MAX)
            RET_ERR(RtErr::ExecutionEngine);
        const int32_t relative_offset = static_cast<int32_t>(distance);
        assert(patch.byte_offset + sizeof(relative_offset) <= patch.source->byte_size);
        std::memcpy(patch.source->data() + patch.byte_offset, &relative_offset, sizeof(relative_offset));
    }

    alloc::MemPool& pool = _mod->get_mem_pool();
    auto* codes = pool.calloc_any<uint8_t>(byte_size);
    interp_method->codes = codes;
    interp_method->code_size = static_cast<uint32_t>(byte_size);
    metadata::PdbImage* pdb = _mod->get_pdb_image();
    auto* mappings = pdb ? pool.calloc_any<metadata::IR2ILMapEntry>(instruction_count) : nullptr;
    // Ordinary interpreted methods do not pay for AOT deoptimization metadata.
    auto* resume_entries = !_method_body->exception_clauses.empty() && _method->invoker_type == metadata::RtInvokerType::Aot
                               ? pool.calloc_any<RtInterpResumeEntry>(instruction_count)
                               : nullptr;
    uint32_t resume_count = 0;
    size_t index = 0;
    for (BasicBlock* block = _basic_blocks; block; block = block->next_bb)
    {
        for (const EncodedInstruction* inst : block->insts)
        {
            std::memcpy(codes + inst->ir_offset, inst->data(), inst->byte_size);
            if (mappings)
                mappings[index++] = {inst->il_offset, inst->ir_offset};
            if (resume_entries && inst->il_offset >= 0)
                resume_entries[resume_count++] = {static_cast<uint32_t>(inst->il_offset), static_cast<uint32_t>(inst->ir_offset)};
        }
    }
    if (pdb)
        pdb->add_ir2il_map_for_method(_method, {{mappings, instruction_count}});
    interp_method->exception_resume_entries = resume_entries;
    interp_method->exception_resume_entry_count = resume_count;
    RET_VOID_OK();
}

} // namespace interp
} // namespace leanclr

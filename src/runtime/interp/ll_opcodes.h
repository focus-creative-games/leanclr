#pragma once

#include "vm/rt_managed_types.h"
#include "interp_defs.h"
#include "vm/field.h"

namespace leanclr
{
namespace interp
{
struct InstructionOperands;

namespace ll
{

#include "generated/ll_opcodes_defs.inc"

class OpCodes
{
  public:
    static size_t get_instruction_size(OpCodeEnum opcode)
    {
        return s_opsizes[static_cast<size_t>(opcode)];
    }
    static uint8_t* write_instruction_to_data(uint8_t* codes, OpCodeEnum opcode, const InstructionOperands& operands);
    static size_t get_branch_patch_offset(OpCodeEnum opcode);
    static bool validate_operands(OpCodeEnum opcode, const InstructionOperands& operands);

  private:
    static size_t s_opsizes[static_cast<size_t>(OpCodeEnum::__Count)];
};

} // namespace ll
} // namespace interp
} // namespace leanclr

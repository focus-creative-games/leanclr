#include "interp/transformer.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>

using namespace leanclr::interp;
using ll::OpCodeEnum;

static void require(bool value, const char* message)
{
    if (!value)
    {
        std::fprintf(stderr, "Encoding check failed: %s\n", message);
        std::abort();
    }
}

int main()
{
    static_assert(sizeof(OpCodeEnum) == 2, "The instruction ID must be 16 bits");
    Variable first{}, second{}, third{}, destination{};
    InstructionOperands operands;
    operands.first = &first;
    operands.second = &second;
    operands.third = &third;
    operands.destination = &destination;
    alignas(8) uint8_t bytes[128];

    // Every opcode must encode exactly its declared fixed size. Switch payloads
    // and branch relocation are deliberately separate from fixed-field encoding.
    for (size_t i = 0; i < static_cast<size_t>(OpCodeEnum::__Count); ++i)
    {
        const auto opcode = static_cast<OpCodeEnum>(i);
        const size_t size = ll::OpCodes::get_instruction_size(opcode);
        require(size >= 4 && size % 4 == 0 && size < sizeof(bytes), "instruction size/alignment");
        require(ll::OpCodes::validate_operands(opcode, operands), "default operands");
        std::memset(bytes, 0xA5, sizeof(bytes));
        require(ll::OpCodes::write_instruction_to_data(bytes, opcode, operands) == bytes + size, "encoded size");
        uint16_t encoded_opcode;
        std::memcpy(&encoded_opcode, bytes, sizeof(encoded_opcode));
        require(encoded_opcode == i, "encoded opcode ID");
        for (size_t offset = size; offset < sizeof(bytes); ++offset)
            require(bytes[offset] == 0xA5, "encoding writes past the instruction");
    }

    first.eval_stack_offset = UINT16_MAX;
    require(ll::OpCodes::validate_operands(OpCodeEnum::LdLocI4, operands), "maximum stack index");
    first.eval_stack_offset = static_cast<size_t>(UINT16_MAX) + 1;
    require(!ll::OpCodes::validate_operands(OpCodeEnum::LdLocI4, operands), "stack index truncation");
    first.eval_stack_offset = 0;
    operands.destination = nullptr;
    require(!ll::OpCodes::validate_operands(OpCodeEnum::AddI4, operands), "missing destination");
    operands.destination = &destination;

    operands.resolved_index = static_cast<size_t>(UINT16_MAX) + 1;
    require(!ll::OpCodes::validate_operands(OpCodeEnum::LdStr, operands), "resolved index truncation");
    operands.resolved_index = 0;
    operands.field_offset = UINT32_MAX;
    require(!ll::OpCodes::validate_operands(OpCodeEnum::LdfldI4, operands), "short field offset truncation");
    require(ll::OpCodes::validate_operands(OpCodeEnum::LdfldI4Large, operands), "large field offset");

    // Preserve immediate bits, including signed zero, NaN payloads and 64-bit values.
    for (uint32_t bits : {0x80000000u, 0x7FC12345u, 0xFFFFFFFFu})
    {
        operands.immediate = bits;
        ll::OpCodes::write_instruction_to_data(bytes, OpCodeEnum::LdcI4I4, operands);
        uint32_t actual;
        std::memcpy(&actual, &reinterpret_cast<const ll::LdcI4I4*>(bytes)->value, sizeof(actual));
        require(actual == bits, "32-bit immediate bits");
    }
    operands.immediate = UINT64_C(0xFEDCBA9876543210);
    ll::OpCodes::write_instruction_to_data(bytes, OpCodeEnum::LdcI8I8, operands);
    const auto* constant = reinterpret_cast<const ll::LdcI8I8*>(bytes);
    require(constant->value_low == 0x76543210u && constant->value_high == 0xFEDCBA98u, "64-bit immediate bits");
    require(ll::OpCodes::get_branch_patch_offset(OpCodeEnum::Br) == offsetof(ll::Br, target_offset), "branch patch offset");
    require(ll::OpCodes::get_branch_patch_offset(OpCodeEnum::AddI4) == static_cast<size_t>(-1), "non-branch patch offset");
    std::puts("All instruction encoding checks passed.");
}

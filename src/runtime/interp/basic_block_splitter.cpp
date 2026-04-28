#include "basic_block_splitter.h"

namespace leanclr
{
namespace interp
{
BasicBlockSplitter::BasicBlockSplitter(const metadata::RtMethodBody* method_body, alloc::MemPool* pool)
    : _method_body(method_body), _split_offsets(), _valid_il_offsets(nullptr), _valid_il_offsets_count(0)
{
    assert(pool != nullptr);
    const size_t code_size = method_body ? static_cast<size_t>(method_body->code_size) : 0;
    _split_offsets.reserve(code_size / 16);
    _valid_il_offsets_count = code_size / 32 + 1;
    _valid_il_offsets = pool && _valid_il_offsets_count > 0 ? pool->calloc_any<uint32_t>(_valid_il_offsets_count) : nullptr;
}

RtResultVoid BasicBlockSplitter::split()
{
    RET_ERR_ON_FAIL(split_codes());
    split_exception_clauses();
    if (!validate_offsets())
    {
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }
    RET_VOID_OK();
}

const utils::HashSet<uint32_t>& BasicBlockSplitter::get_split_offsets() const
{
    return _split_offsets;
}

void BasicBlockSplitter::mark_valid_il_offset(uint32_t offset)
{
    const uint32_t index = offset >> 5;
    const uint32_t bit = 1u << (offset & 31);
    assert(index < _valid_il_offsets_count);
    _valid_il_offsets[index] |= bit;
}

bool BasicBlockSplitter::is_valid_il_offset(uint32_t offset) const
{
    const uint32_t index = offset >> 5;
    const uint32_t bit = 1u << (offset & 31);
    assert(index < _valid_il_offsets_count);
    return (_valid_il_offsets[index] & bit) != 0;
}

RtResultVoid BasicBlockSplitter::split_codes()
{
    assert(_method_body != nullptr);
    const auto& body = *_method_body;
    const uint8_t* codes = body.code;
    const uint32_t code_size = body.code_size;
    const uint8_t* codes_end = codes + code_size;

    uint32_t offset = 0;
    while (offset < code_size)
    {
        mark_valid_il_offset(offset);

        const uint8_t* pc = codes + offset;
        const il::OpCodeInfo* opcode_info = nullptr;
        if (!il::OpCodes::try_decode_opcode_info(pc, codes_end, opcode_info))
        {
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }
        const uint32_t opcode_size = static_cast<uint32_t>(il::OpCodes::get_opcode_size(pc, opcode_info));
        const uint32_t next_offset = offset + opcode_size;
        if (next_offset > code_size)
        {
            RET_ASSERT_ERR(RtErr::ExecutionEngine);
        }

        switch (opcode_info->inline_type)
        {
        case il::ArgType::None:
        case il::ArgType::Data:
            break;
        case il::ArgType::StaticBranch:
            _split_offsets.insert(next_offset);
            break;
        case il::ArgType::BranchTarget:
        {
            uint32_t target_offset = 0;
            if (opcode_info->inline_param == 1)
            {
                target_offset = static_cast<uint32_t>(static_cast<int8_t>(*(pc + 1)));
            }
            else
            {
                uint32_t val = 0;
                std::memcpy(&val, pc + 1, sizeof(uint32_t));
                target_offset = static_cast<uint32_t>(val);
            }
            if (target_offset != 0 || opcode_info->code2 == static_cast<uint8_t>(il::OpCodeValue::LeaveS) ||
                opcode_info->code2 == static_cast<uint8_t>(il::OpCodeValue::Leave))
            {
                _split_offsets.insert(next_offset);
                _split_offsets.insert(next_offset + target_offset);
            }
            break;
        }
        case il::ArgType::Switch:
        {
            int32_t case_count = 0;
            std::memcpy(&case_count, pc + 1, sizeof(int32_t));
            const int32_t* case_start = reinterpret_cast<const int32_t*>(pc + 5);
            bool split_any = false;
            for (int32_t i = 0; i < case_count; ++i)
            {
                uint32_t case_offset_val = 0;
                std::memcpy(&case_offset_val, case_start + i, sizeof(uint32_t));
                const uint32_t case_offset = case_offset_val;
                if (case_offset != 0)
                {
                    _split_offsets.insert(next_offset + case_offset);
                    split_any = true;
                }
            }
            if (split_any)
            {
                _split_offsets.insert(next_offset);
            }
            break;
        }
        }

        offset = next_offset;
    }

    if (offset != code_size)
    {
        RET_ASSERT_ERR(RtErr::ExecutionEngine);
    }

    mark_valid_il_offset(code_size);
    RET_VOID_OK();
}

void BasicBlockSplitter::split_exception_clauses()
{
    assert(_method_body != nullptr);
    for (const auto& clause : _method_body->exception_clauses)
    {
        _split_offsets.insert(static_cast<size_t>(clause.try_offset));
        _split_offsets.insert(static_cast<size_t>(clause.try_offset + clause.try_length));
        _split_offsets.insert(static_cast<size_t>(clause.handler_offset));
        _split_offsets.insert(static_cast<size_t>(clause.handler_offset + clause.handler_length));
    }
}

bool BasicBlockSplitter::validate_offsets() const
{
    for (const auto offset : _split_offsets)
    {
        if (!is_valid_il_offset(offset))
        {
            return false;
        }
    }
    return true;
}

} // namespace interp
} // namespace leanclr

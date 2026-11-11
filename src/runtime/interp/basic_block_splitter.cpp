#include "utils/little_endian.h"
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
    if (!_method_body || (!_method_body->code && _method_body->code_size != 0))
        RET_ERR(RtErr::BadImageFormat);
    if (!_valid_il_offsets)
        RET_ERR(RtErr::OutOfMemory);
    RET_ERR_ON_FAIL(split_codes());
    RET_ERR_ON_FAIL(split_exception_clauses());
    if (!validate_offsets())
    {
        RET_ERR(RtErr::BadImageFormat);
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
    if (offset > _method_body->code_size)
        return false;
    const uint32_t index = offset >> 5;
    const uint32_t bit = 1u << (offset & 31);
    return (_valid_il_offsets[index] & bit) != 0;
}

RtResultVoid BasicBlockSplitter::split_codes()
{
    assert(_method_body != nullptr);
    const auto& body = *_method_body;
    const uint8_t* codes = body.code;
    const uint32_t code_size = body.code_size;
    const uint8_t* codes_end = code_size == 0 ? codes : codes + code_size;

    uint32_t offset = 0;
    while (offset < code_size)
    {
        mark_valid_il_offset(offset);

        const uint8_t* pc = codes + offset;
        const il::OpCodeInfo* opcode_info = nullptr;
        if (!il::OpCodes::try_decode_opcode_info(pc, codes_end, opcode_info))
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        size_t size;
        if (!il::OpCodes::try_get_opcode_size(pc, codes_end, opcode_info, size))
            RET_ERR(RtErr::BadImageFormat);
        const uint32_t opcode_size = static_cast<uint32_t>(size);
        const uint32_t next_offset = offset + opcode_size;
        if (next_offset > code_size)
        {
            RET_ERR(RtErr::BadImageFormat);
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
            const int32_t delta = opcode_info->inline_param == 1 ? static_cast<int8_t>(pc[1]) : utils::LittleEndian::read_i32(pc + 1);
            const int64_t target = static_cast<int64_t>(next_offset) + delta;
            if (target < 0 || target >= code_size)
                RET_ERR(RtErr::BadImageFormat);
            _split_offsets.insert(next_offset);
            _split_offsets.insert(static_cast<uint32_t>(target));
            break;
        }
        case il::ArgType::Switch:
        {
            const uint32_t case_count = utils::LittleEndian::read_u32(pc + 1);
            const uint8_t* case_start = pc + 5;
            bool split_any = false;
            for (uint32_t i = 0; i < case_count; ++i)
            {
                const int32_t delta = utils::LittleEndian::read_i32(case_start + static_cast<size_t>(i) * 4);
                const int64_t target = static_cast<int64_t>(next_offset) + delta;
                if (target < 0 || target >= code_size)
                    RET_ERR(RtErr::BadImageFormat);
                _split_offsets.insert(static_cast<uint32_t>(target));
                split_any = true;
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
        RET_ERR(RtErr::BadImageFormat);
    }

    mark_valid_il_offset(code_size);
    RET_VOID_OK();
}

RtResultVoid BasicBlockSplitter::split_exception_clauses()
{
    assert(_method_body != nullptr);
    for (const auto& clause : _method_body->exception_clauses)
    {
        const uint32_t size = _method_body->code_size;
        if (clause.try_offset > size || clause.try_length > size - clause.try_offset || clause.handler_offset > size ||
            clause.handler_length > size - clause.handler_offset)
            RET_ERR(RtErr::BadImageFormat);
        _split_offsets.insert(static_cast<size_t>(clause.try_offset));
        _split_offsets.insert(static_cast<size_t>(clause.try_offset + clause.try_length));
        _split_offsets.insert(static_cast<size_t>(clause.handler_offset));
        _split_offsets.insert(static_cast<size_t>(clause.handler_offset + clause.handler_length));
    }
    RET_VOID_OK();
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

#include "interp/basic_block_splitter.h"
#include "utils/little_endian.h"
#include <cstdint>
#include <cstddef>
using namespace leanclr;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* bytes, size_t count)
{
    if (count > 4096)
        return 0;
    metadata::RtMethodBody body = {};
    // Half the inputs carry arbitrary exception ranges as well as code bytes.
    if (count >= 17 && (bytes[0] & 1))
    {
        metadata::RtExceptionClause clause = {};
        clause.try_offset = utils::LittleEndian::read_u32(bytes + 1);
        clause.try_length = utils::LittleEndian::read_u32(bytes + 5);
        clause.handler_offset = utils::LittleEndian::read_u32(bytes + 9);
        clause.handler_length = utils::LittleEndian::read_u32(bytes + 13);
        body.exception_clauses.push_back(clause);
        bytes += 17;
        count -= 17;
    }
    body.code = bytes;
    body.code_size = static_cast<uint32_t>(count);
    alloc::MemPool pool(4096, 4096, 4096);
    interp::BasicBlockSplitter splitter(&body, &pool);
    auto result = splitter.split();
    (void)result.is_ok(); // Both well-formed and rejected IL are valid outcomes.
    return 0;
}

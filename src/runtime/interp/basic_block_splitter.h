#pragma once

#include "core/rt_base.h"
#include "metadata/rt_metadata.h"
#include "utils/hashset.h"
#include "alloc/mem_pool.h"
#include "il_opcodes.h"

namespace leanclr
{
namespace interp
{
class BasicBlockSplitter
{
  public:
    BasicBlockSplitter(const metadata::RtMethodBody* method_body, alloc::MemPool* pool);

    RtResultVoid split();

    const utils::HashSet<uint32_t>& get_split_offsets() const;

  private:
    void mark_valid_il_offset(uint32_t offset);
    bool is_valid_il_offset(uint32_t offset) const;

    RtResultVoid split_codes();
    void split_exception_clauses();
    bool validate_offsets() const;

    const metadata::RtMethodBody* _method_body;
    utils::HashSet<uint32_t> _split_offsets;
    uint32_t* _valid_il_offsets;
    size_t _valid_il_offsets_count;
};
} // namespace interp
} // namespace leanclr

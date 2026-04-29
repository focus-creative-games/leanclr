#pragma once

#include "rt_metadata.h"
#include "utils/rt_vector.h"

namespace leanclr
{
namespace metadata
{
struct SizeAndAlignment
{
    uint32_t size;
    uint32_t alignment;
};

class Layout
{
  public:
    // Get class layout data
    static RtResult<SizeAndAlignment> get_field_size_and_alignment(RtTypeSig* typeSig);
    static RtResult<SizeAndAlignment> compute_layout(utils::Vector<const RtFieldInfo*>& fields, int32_t first_field_index_of_cur_klass, uint8_t packing);
    static RtResult<SizeAndAlignment> compute_explicit_layout(RtModuleDef* mod, utils::Vector<const RtFieldInfo*>& fields, uint8_t packing);
};
} // namespace metadata
} // namespace leanclr

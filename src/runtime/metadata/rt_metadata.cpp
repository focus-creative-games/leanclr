#include "rt_metadata.h"

#include "module_def.h"

namespace leanclr
{
namespace metadata
{

uint32_t RtMetadata::encode_gid_by_rid(RtModuleDef& module, uint32_t rid)
{
    assert(rid > 0 && rid <= MAX_METADATA_RID);
    return (module.get_id() << MODULE_ID_SHIFT_AMOUNT) | rid;
}

} // namespace metadata
} // namespace leanclr

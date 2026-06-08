#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace gc
{
    
enum class GCMode : int32_t
{
    DISABLED = 0,
    ENABLED = 1,
    MANUAL = 2
};

enum GcBlockFlags : uint16_t
{
    GC_BLOCK_MARKED = 1 << 0,
    GC_BLOCK_HAS_REFERENCES = 1 << 1,
    GC_BLOCK_LARGE = 1 << 2,
};

struct GcBlockHeader
{
    uint32_t block_size;
    uint16_t flags;
    uint16_t size_class;
#if LEANCLR_GC_DEBUG
    uint32_t site_id;
    int64_t alloc_id;
#endif
};

inline GcBlockHeader* gc_header_from_object(vm::RtObject* obj)
{
    return reinterpret_cast<GcBlockHeader*>(reinterpret_cast<uint8_t*>(obj) - sizeof(GcBlockHeader));
}

inline const GcBlockHeader* gc_header_from_object(const vm::RtObject* obj)
{
    return reinterpret_cast<const GcBlockHeader*>(reinterpret_cast<const uint8_t*>(obj) - sizeof(GcBlockHeader));
}

inline vm::RtObject* gc_object_from_header(GcBlockHeader* header)
{
    return reinterpret_cast<vm::RtObject*>(reinterpret_cast<uint8_t*>(header) + sizeof(GcBlockHeader));
}

inline bool gc_block_is_marked(const GcBlockHeader* header)
{
    return (header->flags & GC_BLOCK_MARKED) != 0;
}

inline void gc_block_set_marked(GcBlockHeader* header)
{
    header->flags = static_cast<uint16_t>(header->flags | GC_BLOCK_MARKED);
}

inline void gc_block_clear_marked(GcBlockHeader* header)
{
    header->flags = static_cast<uint16_t>(header->flags & ~GC_BLOCK_MARKED);
}

} // namespace gc
} // namespace leanclr

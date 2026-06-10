#pragma once

#if LEANCLR_GC_MARK_SWEEP

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "alloc/general_allocation.h"
#include "build_config.h"
#include "core/rt_base.h"
#include "gc/gc_common.h"
#include "gc/gc_config.h"
#include "gc/gc_debug.h"
#include "gc/gc_roots.h"
#include "utils/rt_vector.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace gc
{

struct FreeBlockHeader
{
    FreeBlockHeader* next_free;
};

class SmallHeapArena
{
  private:
    static constexpr size_t kMaxBlockCount = 2048;
    static constexpr size_t kBitsPerUseWord = sizeof(size_t) * 8;
    static constexpr size_t kMaxUseChunkCount = kMaxBlockCount / kBitsPerUseWord;
    static_assert(kMaxBlockCount % kBitsPerUseWord == 0, "kMaxBlockCount must be divisible by kBitsPerUseWord");
    static_assert(kMaxUseChunkCount <= 64, "kMaxUseChunkCount must fit in uint64_t chunk mask");

    void* _data;
    FreeBlockHeader* _free_list;
    size_t _block_size;
    size_t _block_count;
    size_t _use_bits[kMaxUseChunkCount];
    uint64_t _chunk_in_use_mask;

    bool is_block_in_use(size_t index) const;
    bool is_block_free(size_t index) const;
    void set_block_in_use(size_t index);
    void clear_block_in_use(size_t index);
    void* block_at(size_t index) const;
    size_t slot_index(void* block_ptr) const;
    void prepend_to_free_list(FreeBlockHeader* block);
    void rebuild_free_list();
    void release_data();

  public:
    SmallHeapArena(void* arena_data, size_t arena_size, size_t block_size);
    ~SmallHeapArena();

    void* allocate_block();
    bool is_full();
    bool is_empty() const;
    size_t get_block_size() const;
    size_t sweep(const GCAliveObjectBitmap& alive_object_bitmap);
};

class SizeClassPool
{
  private:
    size_t _arena_size;
    size_t _block_size;
    size_t _block_alignment;
    SmallHeapArena* _current_arena;
    utils::Vector<SmallHeapArena*> _arenas;
    size_t _next_find_not_full_start;

    SmallHeapArena* allocate_arena();
    void free_arena(SmallHeapArena* arena);
    SmallHeapArena* find_next_not_full_arena();

  public:
    SizeClassPool(size_t arena_size, size_t block_size, size_t block_alignment);

    void* allocate_block();
    void sweep(const GCAliveObjectBitmap& alive_object_bitmap, int64_t& freed_bytes);
    size_t block_size() const
    {
        return _block_size;
    }
};

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_MARK_SWEEP

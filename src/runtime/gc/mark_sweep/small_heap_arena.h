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
    static_assert(kMaxBlockCount <= UINT16_MAX, "Arena cursors must fit uint16_t");
    uint16_t _block_count;
    uint16_t _next_unused;
    size_t _use_bits[kMaxUseChunkCount];
    uint64_t _chunk_in_use_mask;

    bool is_block_in_use(size_t index) const;
    bool is_block_free(size_t index) const;

    void set_block_in_use(size_t index)
    {
        const size_t word_index = index / kBitsPerUseWord;
        assert(word_index < kMaxUseChunkCount);
        _use_bits[word_index] |= size_t(1) << (index % kBitsPerUseWord);
        _chunk_in_use_mask |= uint64_t(1) << word_index;
    }

    void clear_block_in_use(size_t index);
    void* block_at(size_t index) const;
    size_t slot_index(void* block_ptr) const;
    void prepend_to_free_list(FreeBlockHeader* block);
    void release_data();
#if LEANCLR_GC_DEBUG
    static bool is_zeroed(void* block, size_t size);
#endif

  public:
    SmallHeapArena(void* arena_data, size_t arena_size, size_t block_size);
    ~SmallHeapArena();

    void* allocate_block()
    {
        void* block;
        size_t index;
        if (_free_list != nullptr)
        {
            auto* recycled = _free_list;
            _free_list = recycled->next_free;
            recycled->next_free = nullptr;
            block = recycled;
            index = slot_index(block);
        }
        else
        {
            if (_next_unused == _block_count)
                return nullptr;
            index = _next_unused++;
            block = reinterpret_cast<uint8_t*>(_data) + index * _block_size;
        }
#if LEANCLR_GC_DEBUG
        LEANCLR_GC_ASSERT(is_zeroed(block, _block_size), "block is not zeroed");
#endif
        set_block_in_use(index);
        return block;
    }

    bool is_full();
    bool is_empty() const;
    size_t get_block_size() const;
    size_t sweep(const GCAliveObjectBitmap& alive_object_bitmap);

  public:
#if LEANCLR_GC_MARK_SWEEP
    bool contains_allocated_block(const void* ptr) const;
    vm::RtObject* find_object_containing(const void* ptr) const;

    void* data() const
    {
        return _data;
    }
#endif
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
    void* allocate_block_slow();

  public:
    SizeClassPool(size_t arena_size, size_t block_size, size_t block_alignment);

    void* allocate_block()
    {
        if (LEANCLR_LIKELY(_current_arena != nullptr))
        {
            void* block = _current_arena->allocate_block();
            if (LEANCLR_LIKELY(block != nullptr))
                return block;
        }
        return allocate_block_slow();
    }

    void sweep(const GCAliveObjectBitmap& alive_object_bitmap, int64_t& freed_bytes);

    size_t block_size() const
    {
        return _block_size;
    }

  public:
#if LEANCLR_GC_MARK_SWEEP
    bool contains_allocated_block(const void* ptr) const;
#endif
};

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_MARK_SWEEP

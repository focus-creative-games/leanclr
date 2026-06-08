#pragma once

#include <cstdint>

#include "alloc/general_allocation.h"
#include "utils/hashmap.h"

namespace leanclr
{
namespace utils
{

// Sparse bitmap keyed by address / slot_granularity. mark() returns true on first visit.
template <size_t SLOT_GRANULARITY, size_t SEGMENT_SIZE_BYTE_COUNT, size_t SEGMENT_CACHE_SIZE>
class SegmentedAddressBitmap
{
  private:
    static_assert((SLOT_GRANULARITY & (SLOT_GRANULARITY - 1)) == 0, "SLOT_GRANULARITY must be a power of two");
    static_assert((SEGMENT_SIZE_BYTE_COUNT & (SEGMENT_SIZE_BYTE_COUNT - 1)) == 0, "SEGMENT_SIZE_BYTE_COUNT must be a power of two");
    static_assert(SEGMENT_SIZE_BYTE_COUNT % sizeof(size_t) == 0, "SEGMENT_SIZE_BYTE_COUNT must be a multiple of sizeof(size_t)");
    static_assert((SEGMENT_CACHE_SIZE & (SEGMENT_CACHE_SIZE - 1)) == 0, "SEGMENT_CACHE_SIZE must be a power of two");

    static constexpr size_t kBitsPerWord = sizeof(size_t) * 8;
    static constexpr size_t kSegmentWordCount = SEGMENT_SIZE_BYTE_COUNT / sizeof(size_t);
    static constexpr size_t kSegmentBitCount = kSegmentWordCount * kBitsPerWord;
    static constexpr size_t kInvalidSegmentIndex = static_cast<size_t>(-1);

    struct SegmentCacheEntry
    {
        size_t segment_index = kInvalidSegmentIndex;
        size_t* segment = nullptr;
    };

    HashMap<size_t, size_t*> _segment_map;
    mutable SegmentCacheEntry _segment_cache[SEGMENT_CACHE_SIZE]{};

    size_t* get_or_create_segment_slow(size_t segment_index, size_t cache_slot)
    {
        auto it = _segment_map.find(segment_index);
        if (it == _segment_map.end())
        {
            size_t* segment = alloc::GeneralAllocation::calloc_any<size_t>(kSegmentWordCount);
            it = _segment_map.emplace(segment_index, segment).first;
        }
        _segment_cache[cache_slot].segment_index = segment_index;
        _segment_cache[cache_slot].segment = it->second;
        return it->second;
    }

    size_t* get_or_create_segment(size_t segment_index)
    {
        const size_t cache_slot = segment_index & (SEGMENT_CACHE_SIZE - 1);
        const SegmentCacheEntry& entry = _segment_cache[cache_slot];
        if (entry.segment_index == segment_index)
        {
            return entry.segment;
        }
        return get_or_create_segment_slow(segment_index, cache_slot);
    }

  public:
    ~SegmentedAddressBitmap()
    {
        for (auto it = _segment_map.begin(); it != _segment_map.end(); ++it)
        {
            alloc::GeneralAllocation::free(it->second);
        }
    }

    bool mark(void* address)
    {
        const size_t slot_index = reinterpret_cast<uintptr_t>(address) / SLOT_GRANULARITY;
        const size_t segment_index = slot_index / kSegmentBitCount;
        size_t* segment = get_or_create_segment(segment_index);
        const size_t bit_index_in_segment = slot_index % kSegmentBitCount;
        const size_t word_index = bit_index_in_segment / kBitsPerWord;
        const size_t bit_in_word = bit_index_in_segment % kBitsPerWord;
        const size_t mask = static_cast<size_t>(1) << bit_in_word;
        if (segment[word_index] & mask)
        {
            return false;
        }
        segment[word_index] |= mask;
        return true;
    }

    bool is_marked(const void* address) const
    {
        const size_t slot_index = reinterpret_cast<uintptr_t>(address) / SLOT_GRANULARITY;
        const size_t segment_index = slot_index / kSegmentBitCount;
        auto it = _segment_map.find(segment_index);
        if (it == _segment_map.end())
        {
            return false;
        }
        const size_t bit_index_in_segment = slot_index % kSegmentBitCount;
        const size_t word_index = bit_index_in_segment / kBitsPerWord;
        const size_t bit_in_word = bit_index_in_segment % kBitsPerWord;
        const size_t mask = static_cast<size_t>(1) << bit_in_word;
        return (it->second[word_index] & mask) != 0;
    }
};

} // namespace utils
} // namespace leanclr

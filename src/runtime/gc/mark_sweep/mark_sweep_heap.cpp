#include "gc/mark_sweep/mark_sweep_heap.h"

#if LEANCLR_GC_MARK_SWEEP

#include <cstring>
#include <unordered_map>

#include "alloc/general_allocation.h"
#include "gc/gc_config.h"
#include "gc/gc_common.h"
#include "gc/gc_scan.h"
#include "gc/handles/gc_handle_table.h"
#include "gc/roots/gc_roots.h"
#include "utils/rt_vector.h"
#include "utils/mem_op.h"
#include "vm/class.h"

namespace leanclr
{
namespace gc
{

struct FreeBlockHeader
{
    FreeBlockHeader* next_free;
};

struct ArenaHeader
{
    void* next_arena;
    FreeBlockHeader* free_list;
    uint16_t arena_size;
    uint16_t block_size;
    uint16_t block_count;
    // uint16_t used_count;
};

class SmallHeapArena
{
  private:
    ArenaHeader _header;

    void initialize_free_list(uint16_t first_block_offset)
    {
        uint8_t* arena_data_start = (uint8_t*)this;
        FreeBlockHeader* first_block = (FreeBlockHeader*)(arena_data_start + first_block_offset);
        _header.free_list = first_block;

        // block_offset is the offset of the current block from the arena_data_start,
        // so it includes the size of the ArenaHeader.
        FreeBlockHeader* cur_block = first_block;
        for (uint16_t i = 0; i + 1 < _header.block_count; i++)
        {
            FreeBlockHeader* next_block = (FreeBlockHeader*)((uint8_t*)(cur_block) + _header.block_size);
            cur_block->next_free = next_block;
            cur_block = next_block;
        }
        cur_block->next_free = nullptr;
    }

  public:
    SmallHeapArena(uint16_t arena_size, uint16_t block_size, uint16_t block_alignment)
    {
        assert((void*)this == (void*)&_header);
        assert(arena_size % block_alignment == 0);
        assert(arena_size > sizeof(ArenaHeader));
        assert(block_size >= sizeof(FreeBlockHeader));
        assert(block_size >= sizeof(void*));
        assert(block_size % block_alignment == 0);
        _header.next_arena = nullptr;
        _header.arena_size = arena_size;
        _header.block_size = block_size;
        uint16_t fist_block_offset = static_cast<uint16_t>(utils::MemOp::align_up(sizeof(ArenaHeader), block_alignment));
        _header.block_count = (uint16_t)((size_t)arena_size - fist_block_offset) / block_size;
        // _header.used_count = 0;
        _header.free_list = nullptr;
        initialize_free_list(fist_block_offset);
    }

    void* allocate_block()
    {
        if (_header.free_list == nullptr)
        {
            return nullptr;
        }
        FreeBlockHeader* free_block = _header.free_list;
        _header.free_list = (FreeBlockHeader*)free_block->next_free;
        return free_block;
    }

    bool is_full()
    {
        return _header.free_list == nullptr;
    }
};

template <typename T>
class ArenaAllocator
{
};

template <>
class ArenaAllocator<SmallHeapArena>
{
  private:
    uint16_t _arena_size;
    uint16_t _block_size;
    uint16_t _block_alignment;

  public:
    ArenaAllocator(uint16_t arena_size, uint16_t block_size, uint16_t block_alignment)
        : _arena_size(arena_size), _block_size(block_size), _block_alignment(block_alignment)
    {
    }

    SmallHeapArena* allocate_arena()
    {
        void* arena_data_start = alloc::GeneralAllocation::aligned_malloc(_arena_size, _block_alignment);
        if (arena_data_start == nullptr)
        {
            return nullptr;
        }
        std::memset(arena_data_start, 0, _arena_size);
        return new (arena_data_start) SmallHeapArena(_arena_size, _block_size, _block_alignment);
    }

    void free_arena(SmallHeapArena* arena)
    {
        arena->~SmallHeapArena();
        alloc::GeneralAllocation::aligned_free(arena);
    }
};

template <typename T>
class ArenaCollection
{
  private:
    T* _current_arena;
    utils::Vector<T*> _not_full_arenas;
    utils::Vector<T*> _full_arenas;
    ArenaAllocator<T> _allocator;

  public:
    ArenaCollection(ArenaAllocator<T> allocator) : _allocator(allocator)
    {
        _current_arena = _allocator.allocate_arena();
    }

    void* allocate_block()
    {
        void* block = _current_arena->allocate_block();
        if (LEANCLR_LIKELY(block != nullptr))
        {
            return block;
        }

        _full_arenas.push_back(_current_arena);
        if (!_not_full_arenas.empty())
        {
            _current_arena = _not_full_arenas.back();
            _not_full_arenas.pop_back();
        }
        else
        {
            T* new_arena = _allocator.allocate_arena();
            if (new_arena == nullptr)
            {
                // Rollback the full arena
                _full_arenas.pop_back();
                return nullptr;
            }
            _current_arena = new_arena;
        }
        return _current_arena->allocate_block();
    }
};

constexpr size_t kMinSmallHeapBlockSize = 8;
constexpr size_t kMaxSmallHeapBlockSize = 256;
constexpr size_t kSmallHeapBlockSizeIncrement = 8;
constexpr size_t kSmallHeapArenaSize = 16 * 1024;
static_assert(kSmallHeapArenaSize <= (1 << 16), "kSmallHeapArenaSize must be less than or equal to 64KB");
constexpr size_t kSmallHeapArenaCount = (kMaxSmallHeapBlockSize - kMinSmallHeapBlockSize) / kSmallHeapBlockSizeIncrement + 1;

ArenaCollection<SmallHeapArena>* s_small_heap_arenas[kSmallHeapArenaCount] = {};

std::unordered_map<void*, size_t> s_fixed_blocks;

static int64_t s_used_bytes = 0;
static int64_t s_heap_bytes = 0;

void initialize_small_heap_arenas()
{
    for (size_t i = 0; i < kSmallHeapArenaCount; i++)
    {
        s_small_heap_arenas[i] = new ArenaCollection<SmallHeapArena>(
            ArenaAllocator<SmallHeapArena>(static_cast<uint16_t>(kSmallHeapArenaSize),
                                           static_cast<uint16_t>(kMinSmallHeapBlockSize + i * kSmallHeapBlockSizeIncrement), static_cast<uint16_t>(GC_ALIGN)));
    }
}

static void scan_fixed_blocks(GcVisitUnknownBlock visit, void* userdata)
{
    for (auto it = s_fixed_blocks.begin(); it != s_fixed_blocks.end(); ++it)
    {
        visit(it->first, it->second, userdata);
    }
}

void MarkSweepHeap::initialize()
{
    GcPressureConfig cfg = {GC_DEFAULT_BYTE_THRESHOLD, GC_DEFAULT_SOFT_HEAP_LIMIT};
    GcPressure::initialize(cfg);
    s_used_bytes = 0;
    s_heap_bytes = 0;
    initialize_small_heap_arenas();
    GcRoots::register_visit_unknown_blocks(scan_fixed_blocks);
}

void MarkSweepHeap::collect()
{
    GcPressure::on_collect();
}

bool MarkSweepHeap::should_collect(bool force)
{
    return GcPressure::should_collect(force);
}

bool MarkSweepHeap::maybe_collect()
{
    if (!should_collect(false))
    {
        return false;
    }
    collect();
    return true;
}

int64_t MarkSweepHeap::get_used_size()
{
    return s_used_bytes;
}

int64_t MarkSweepHeap::get_heap_size()
{
    return s_heap_bytes;
}

int32_t MarkSweepHeap::get_collection_count()
{
    return 0;
}

void MarkSweepHeap::set_pressure_config(const GcPressureConfig& config)
{
    GcPressure::set_config(config);
}

// this method is used by il2cpp. we don't use it in runtime.
// we assume fixed memory count is small so we use unordered_map instead of vector
// because we think unordered_map is more efficient than vector for small size.
void* MarkSweepHeap::allocate_fixed(size_t size)
{
    void* block = alloc::GeneralAllocation::malloc_zeroed(size);
    if (block == nullptr)
    {
        return nullptr;
    }
    s_fixed_blocks[block] = size;
    s_used_bytes += size;
    s_heap_bytes += size;
    return block;
}

void MarkSweepHeap::free_fixed(void* address)
{
    auto it = s_fixed_blocks.find(address);
    if (it == s_fixed_blocks.end())
    {
        assert(false && "Address not found in fixed blocks");
        return;
    }
    size_t size = it->second;
    s_used_bytes -= size;
    s_heap_bytes -= size;
    s_fixed_blocks.erase(it);
    alloc::GeneralAllocation::free(address);
}



vm::RtObject* MarkSweepHeap::allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
{
    return allocate_object(klass, size);
}

vm::RtObject* MarkSweepHeap::allocate_object(const metadata::RtClass* klass, size_t size)
{
    assert(size >= sizeof(vm::RtObject));
    size_t aligned_size = utils::MemOp::align_up(size, GC_ALIGN);
    vm::RtObject* obj;
    if (aligned_size <= kMaxSmallHeapBlockSize)
    {
        obj = (vm::RtObject*)s_small_heap_arenas[(aligned_size - kMinSmallHeapBlockSize) / kSmallHeapBlockSizeIncrement]->allocate_block();
    }
    else
    {
        obj = (vm::RtObject*)alloc::GeneralAllocation::malloc_zeroed(aligned_size);
    }
    obj->klass = const_cast<metadata::RtClass*>(klass);
    s_used_bytes += aligned_size;
    s_heap_bytes += aligned_size;
    GcPressure::on_alloc(aligned_size);
    return obj;
}

vm::RtObject* MarkSweepHeap::allocate_array(const metadata::RtClass* arrClass, size_t totalBytes, const GcAllocSite& site)
{
    return allocate_object(arrClass, totalBytes);
}

vm::RtObject* MarkSweepHeap::allocate_array(const metadata::RtClass* arrClass, size_t totalBytes)
{
    return allocate_object(arrClass, totalBytes);
}

bool MarkSweepHeap::is_object_marked(const vm::RtObject* /*obj*/)
{
    return true;
}

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_MARK_SWEEP

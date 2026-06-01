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
#include "utils/hashset.h"
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
    size_t arena_size;
    size_t block_size;
    size_t block_count;
    // size_t used_count;
};

class SmallHeapArena
{
  private:
    ArenaHeader _header;

    void initialize_free_list(size_t first_block_offset)
    {
        uint8_t* arena_data_start = (uint8_t*)this;
        FreeBlockHeader* first_block = (FreeBlockHeader*)(arena_data_start + first_block_offset);
        _header.free_list = first_block;

        // block_offset is the offset of the current block from the arena_data_start,
        // so it includes the size of the ArenaHeader.
        FreeBlockHeader* cur_block = first_block;
        for (size_t i = 0; i + 1 < _header.block_count; i++)
        {
            FreeBlockHeader* next_block = (FreeBlockHeader*)((uint8_t*)(cur_block) + _header.block_size);
            cur_block->next_free = next_block;
            cur_block = next_block;
        }
        cur_block->next_free = nullptr;
    }

  public:
    SmallHeapArena(size_t arena_size, size_t block_size, size_t block_alignment)
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
        size_t fist_block_offset = utils::MemOp::align_up(sizeof(ArenaHeader), block_alignment);
        _header.block_count = (arena_size - fist_block_offset) / block_size;
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
    size_t _arena_size;
    size_t _block_size;
    size_t _block_alignment;

  public:
    ArenaAllocator(size_t arena_size, size_t block_size, size_t block_alignment)
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
    ArenaCollection(ArenaAllocator<T> allocator) : _allocator(allocator), _current_arena(nullptr)
    {
    }

    void* allocate_block()
    {
        if (LEANCLR_LIKELY(_current_arena != nullptr))
        {
            void* block = _current_arena->allocate_block();
            if (LEANCLR_LIKELY(block != nullptr))
            {
                return block;
            }
            _full_arenas.push_back(_current_arena);
            _current_arena = nullptr;
        }
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
                return nullptr;
            }
            _current_arena = new_arena;
        }
        return _current_arena->allocate_block();
    }
};

struct ArenaCollectionInfo
{
    size_t max_size;
    size_t size_increment;
    size_t arena_size;
};

static constexpr ArenaCollectionInfo s_arena_collection_infos[] = {
    {256, 8, 16 * 1024}, {512, 16, 32 * 1024}, {1024, 32, 64 * 1024}, {2048, 64, 128 * 1024}, {4096, 128, 256 * 1024}, {8192, 256, 512 * 1024},
};

constexpr size_t kArenaCollectionInfoCount = sizeof(s_arena_collection_infos) / sizeof(s_arena_collection_infos[0]);
constexpr size_t kArenaCollectionCount = kArenaCollectionInfoCount * 16 + 16 /* collection0 has extra 16 arenas for small objects */;
static ArenaCollection<SmallHeapArena>* s_small_heap_arenas[kArenaCollectionCount] = {};
constexpr size_t kMaxSmallObejctSize = s_arena_collection_infos[kArenaCollectionInfoCount - 1].max_size;
static ArenaCollection<SmallHeapArena>* s_map_div8_size_to_arena[kMaxSmallObejctSize / 8] = {};
static utils::HashSet<void*> s_big_object_arenas;

// constexpr size_t kMinSmallHeapBlockSize = 8;
// constexpr size_t kMaxSmallHeapBlockSize = 256;
// constexpr size_t kSmallHeapBlockSizeIncrement = GC_ALIGN;
// constexpr size_t kSmallHeapArenaSize = 16 * 1024;
// static_assert(kSmallHeapArenaSize <= (1 << 16), "kSmallHeapArenaSize must be less than or equal to 64KB");
// constexpr size_t kSmallHeapArenaCount = (kMaxSmallHeapBlockSize - kMinSmallHeapBlockSize) / kSmallHeapBlockSizeIncrement + 1;

std::unordered_map<void*, size_t> s_fixed_blocks;

static int64_t s_used_bytes = 0;
static int64_t s_heap_bytes = 0;

static void initialize_small_heap_arenas()
{
    size_t last_arena_max_size = 0;
    size_t last_arena_index = 0;
    size_t last_map_index = 0;
    for (size_t i = 0; i < kArenaCollectionInfoCount; i++)
    {
        size_t arena_size = s_arena_collection_infos[i].arena_size;
        size_t max_size = s_arena_collection_infos[i].max_size;
        size_t size_increment = s_arena_collection_infos[i].size_increment;
        for (size_t j = 1; j <= (max_size - last_arena_max_size) / size_increment; j++)
        {
            size_t current_arena_size = last_arena_max_size + size_increment * j;
            auto new_arena = new ArenaCollection<SmallHeapArena>(ArenaAllocator<SmallHeapArena>(arena_size, current_arena_size, GC_ALIGN));
            for (; last_map_index < current_arena_size / 8; last_map_index++)
            {
                s_map_div8_size_to_arena[last_map_index] = new_arena;
            }
            s_small_heap_arenas[last_arena_index++] = new_arena;
        }
        last_arena_max_size = max_size;
    }
    assert(last_map_index == kMaxSmallObejctSize / 8);
    assert(last_arena_index == kArenaCollectionCount);
}

static inline ArenaCollection<SmallHeapArena>* get_arena_collection(size_t size)
{
    assert(size > 0 && size % 8 == 0);
    return s_map_div8_size_to_arena[size / 8 - 1];
}

static inline bool is_small_object(size_t size)
{
    return size <= kMaxSmallObejctSize;
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

vm::RtObject* allocate_object_impl(const metadata::RtClass* klass, size_t size, const GcAllocSite* site)
{
    assert(size >= sizeof(vm::RtObject));
    size_t aligned_size = utils::MemOp::align_up(size, GC_ALIGN);
    vm::RtObject* obj;
    if (is_small_object(aligned_size))
    {
        obj = (vm::RtObject*)get_arena_collection(aligned_size)->allocate_block();
        if (obj == nullptr)
        {
            return nullptr;
        }
    }
    else
    {
        obj = (vm::RtObject*)alloc::GeneralAllocation::malloc_zeroed(aligned_size);
        if (obj != nullptr)
        {
            s_big_object_arenas.insert(obj);
        }
        else
        {
            return nullptr;
        }
    }
    obj->klass = const_cast<metadata::RtClass*>(klass);
#if LEANCLR_GC_DEBUG
    obj->__sync_block = site != nullptr ? const_cast<GcAllocSite*>(site->intern_site()) : nullptr;
#endif
    s_used_bytes += aligned_size;
    s_heap_bytes += aligned_size;
    GcPressure::on_alloc(aligned_size);
    return obj;
}

vm::RtObject* MarkSweepHeap::allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
{
    return allocate_object_impl(klass, size, &site);
}

vm::RtObject* MarkSweepHeap::allocate_object(const metadata::RtClass* klass, size_t size)
{
    return allocate_object_impl(klass, size, nullptr);
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

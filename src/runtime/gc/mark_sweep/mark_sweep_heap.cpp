#include "gc/mark_sweep/mark_sweep_heap.h"

#if LEANCLR_GC_MARK_SWEEP

#include <cstring>

#include "alloc/general_allocation.h"
#include "gc/gc_config.h"
#include "gc/gc_common.h"
#include "gc/gc_debug.h"
#include "gc/gc_scan.h"
#include "gc/gc_handle_table.h"
#include "gc/gc_roots.h"
#include "utils/rt_vector.h"
#include "utils/mem_op.h"
#include "utils/hashset.h"
#include "vm/class.h"
#include "vm/gchandle.h"
#include "vm/rt_array.h"
#include "vm/rt_string.h"

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

#if LEANCLR_GC_DEBUG
    bool is_zeroed(void* block, size_t size) const
    {
        uint8_t* block_data = reinterpret_cast<uint8_t*>(block);
        for (size_t i = 0; i < size; i++)
        {
            if (block_data[i] != 0)
            {
                return false;
            }
        }
        return true;
    }
#endif

    void* allocate_block()
    {
        if (_header.free_list == nullptr)
        {
            return nullptr;
        }
        FreeBlockHeader* free_block = _header.free_list;
        _header.free_list = (FreeBlockHeader*)free_block->next_free;
        free_block->next_free = nullptr;
#if LEANCLR_GC_DEBUG
        LEANCLR_GC_ASSERT(is_zeroed(free_block, _header.block_size), "free_block is not zeroed");
#endif
        return free_block;
    }

    bool is_full()
    {
        return _header.free_list == nullptr;
    }

    size_t get_block_size() const
    {
        return _header.block_size;
    }

    size_t sweep(const GCAliveObjectBitmap& alive_object_bitmap)
    {
        utils::HashSet<void*> free_blocks;
        for (FreeBlockHeader* cur = _header.free_list; cur != nullptr; cur = cur->next_free)
        {
            free_blocks.insert(cur);
        }

        size_t first_block_offset = utils::MemOp::align_up(sizeof(ArenaHeader), GC_ALIGN);
        uint8_t* arena_data_start = (uint8_t*)this;
        FreeBlockHeader* new_free_list = nullptr;
        size_t freed_count = 0;

        for (size_t i = 0; i < _header.block_count; i++)
        {
            void* block_ptr = arena_data_start + first_block_offset + i * _header.block_size;
            if (free_blocks.find(block_ptr) != free_blocks.end())
            {
                FreeBlockHeader* free_block = (FreeBlockHeader*)block_ptr;
                free_block->next_free = new_free_list;
                new_free_list = free_block;
                continue;
            }

            vm::RtObject* obj = (vm::RtObject*)block_ptr;
            if (alive_object_bitmap.is_marked(obj))
            {
                continue;
            }

#if LEANCLR_GC_DEBUG
            if (gc_debug_is_quarantined_tombstone(obj))
            {
                continue;
            }
#endif

            FreeBlockHeader* free_block = (FreeBlockHeader*)block_ptr;
            LEANCLR_ASSUME((uintptr_t)free_block % GC_ALIGN == 0);
            LEANCLR_ASSUME(_header.block_size % GC_ALIGN == 0);
#if LEANCLR_GC_DEBUG
            gc_debug_quarantine_object((vm::RtObject*)free_block, _header.block_size);
#else
            std::memset(free_block, 0, _header.block_size);
            free_block->next_free = new_free_list;
            new_free_list = free_block;
#endif
            freed_count++;
        }

        _header.free_list = new_free_list;
        return freed_count;
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

    void sweep(const GCAliveObjectBitmap& alive_object_bitmap, int64_t& freed_bytes)
    {
        utils::Vector<SmallHeapArena*> all_arenas;
        if (_current_arena != nullptr)
        {
            all_arenas.push_back(_current_arena);
        }
        for (size_t i = 0; i < _not_full_arenas.size(); ++i)
        {
            all_arenas.push_back(_not_full_arenas[i]);
        }
        for (size_t i = 0; i < _full_arenas.size(); ++i)
        {
            all_arenas.push_back(_full_arenas[i]);
        }

        _not_full_arenas.clear();
        _full_arenas.clear();
        _current_arena = nullptr;

        for (size_t i = 0; i < all_arenas.size(); ++i)
        {
            SmallHeapArena* arena = all_arenas[i];
            size_t freed_count = arena->sweep(alive_object_bitmap);
            freed_bytes += static_cast<int64_t>(freed_count * arena->get_block_size());
            if (arena->is_full())
            {
                _full_arenas.push_back(arena);
            }
            else if (_current_arena == nullptr)
            {
                _current_arena = arena;
            }
            else
            {
                _not_full_arenas.push_back(arena);
            }
        }
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

static int64_t s_used_bytes = 0;
static int64_t s_heap_bytes = 0;
static int32_t s_collection_count = 0;
static GCMode s_gc_mode;

static bool is_object_alive(vm::RtObject* obj, void* ctx)
{
    GCAliveObjectBitmap* alive_object_bitmap = reinterpret_cast<GCAliveObjectBitmap*>(ctx);
    return alive_object_bitmap->is_marked(obj);
}

static size_t get_object_allocated_size(vm::RtObject* obj)
{
    const metadata::RtClass* klass = obj->klass;
    if (vm::Class::is_string_class(klass))
    {
        auto* str = reinterpret_cast<vm::RtString*>(obj);
        return static_cast<size_t>(vm::String::get_string_allocation_size(str->length));
    }
    if (vm::Class::is_array_or_szarray(klass))
    {
        auto* arr = reinterpret_cast<vm::RtArray*>(obj);
        return vm::Array::get_array_allocation_size(arr->klass, vm::Array::get_array_length(arr));
    }
    return vm::Class::get_instance_size_with_object_header(klass);
}

static void sweep_small_objects(const GCAliveObjectBitmap& alive_object_bitmap, int64_t& freed_bytes)
{
    for (size_t i = 0; i < kArenaCollectionCount; ++i)
    {
        ArenaCollection<SmallHeapArena>* collection = s_small_heap_arenas[i];
        if (collection != nullptr)
        {
            collection->sweep(alive_object_bitmap, freed_bytes);
        }
    }
}

static void sweep_big_objects(const GCAliveObjectBitmap& alive_object_bitmap, int64_t& freed_bytes)
{
    utils::Vector<void*> dead_objects;
    for (auto it = s_big_object_arenas.begin(); it != s_big_object_arenas.end(); ++it)
    {
        vm::RtObject* obj = reinterpret_cast<vm::RtObject*>(*it);
        if (!alive_object_bitmap.is_marked(obj))
        {
            dead_objects.push_back(*it);
        }
    }

    for (size_t i = 0; i < dead_objects.size(); ++i)
    {
        vm::RtObject* obj = reinterpret_cast<vm::RtObject*>(dead_objects[i]);
        size_t aligned_size = utils::MemOp::align_up(get_object_allocated_size(obj), GC_ALIGN);
#if LEANCLR_GC_DEBUG
        gc_debug_quarantine_object(obj, aligned_size);
#else
        alloc::GeneralAllocation::free(dead_objects[i]);
#endif
        s_big_object_arenas.erase(dead_objects[i]);
        freed_bytes += static_cast<int64_t>(aligned_size);
    }
}

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

void MarkSweepHeap::initialize(const Config& config)
{
    s_gc_mode = config.mode;
    GcPressure::initialize(config.pressure_config);
    s_used_bytes = 0;
    s_heap_bytes = 0;
    initialize_small_heap_arenas();
}

void MarkSweepHeap::collect()
{
    if (s_gc_mode == GCMode::DISABLED)
    {
        return;
    }
    printf("MarkSweepHeap::collect begin\n");
    GcPressure::on_collect();
    GCAliveObjectBitmap alive_object_bitmap;
    GcRoots::foreach_root(alive_object_bitmap);

    int64_t freed_bytes = 0;
    sweep_small_objects(alive_object_bitmap, freed_bytes);
    sweep_big_objects(alive_object_bitmap, freed_bytes);
    vm::GCHandle::sweep_weak_handles(is_object_alive, &alive_object_bitmap);
    // TODO: Run finalizers for collected objects.

    int64_t old_heap_bytes = s_heap_bytes;
    s_used_bytes -= freed_bytes;
    s_heap_bytes -= freed_bytes;
    s_collection_count++;
    printf("MarkSweepHeap::collect end, old_heap_bytes: %lld, new_heap_bytes: %lld, freed_bytes: %lld\n", old_heap_bytes, s_heap_bytes, freed_bytes);
}

bool MarkSweepHeap::should_collect(bool force)
{
    if (s_gc_mode == GCMode::DISABLED)
    {
        return false;
    }
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
    return s_collection_count;
}

void MarkSweepHeap::set_gc_mode(GCMode mode)
{
    s_gc_mode = mode;
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

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_MARK_SWEEP

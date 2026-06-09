#include "gc/mark_sweep/small_heap_arena.h"

#if LEANCLR_GC_MARK_SWEEP

namespace leanclr
{
namespace gc
{

#if LEANCLR_GC_DEBUG
static bool is_zeroed(void* block, size_t size)
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

size_t SmallHeapArena::use_bits_word_count(size_t block_count)
{
    return (block_count + kBitsPerUseWord - 1) / kBitsPerUseWord;
}

bool SmallHeapArena::is_block_in_use(size_t index) const
{
    return (_use_bits[index / kBitsPerUseWord] >> (index % kBitsPerUseWord)) & 1;
}

bool SmallHeapArena::is_block_free(size_t index) const
{
    return !is_block_in_use(index);
}

void SmallHeapArena::set_block_in_use(size_t index, bool in_use)
{
    size_t word_index = index / kBitsPerUseWord;
    size_t bit_index = index % kBitsPerUseWord;
    size_t mask = static_cast<size_t>(1) << bit_index;
    if (in_use)
    {
        _use_bits[word_index] |= mask;
    }
    else
    {
        _use_bits[word_index] &= ~mask;
    }
}

void* SmallHeapArena::block_at(size_t index) const
{
    return reinterpret_cast<uint8_t*>(_data) + index * _block_size;
}

size_t SmallHeapArena::slot_index(void* block_ptr) const
{
    uintptr_t block_addr = reinterpret_cast<uintptr_t>(block_ptr);
    uintptr_t data_addr = reinterpret_cast<uintptr_t>(_data);
    assert((block_addr - data_addr) % _block_size == 0);
    return static_cast<size_t>((block_addr - data_addr) / _block_size);
}

void SmallHeapArena::rebuild_free_list()
{
    FreeBlockHeader* new_free_list = nullptr;
    for (size_t i = 0; i < _block_count; i++)
    {
        if (!is_block_free(i))
        {
            continue;
        }
        FreeBlockHeader* free_block = reinterpret_cast<FreeBlockHeader*>(block_at(i));
        free_block->next_free = new_free_list;
        new_free_list = free_block;
    }
    _free_list = new_free_list;
}

void SmallHeapArena::release_data()
{
    alloc::GeneralAllocation::aligned_free(_data);
    _data = nullptr;
    _block_count = 0;
    _free_list = nullptr;
}

size_t SmallHeapArena::allocation_size(size_t arena_size, size_t block_size)
{
    size_t block_count = arena_size / block_size;
    return offsetof(SmallHeapArena, _use_bits) + use_bits_word_count(block_count) * sizeof(size_t);
}

SmallHeapArena::SmallHeapArena(void* arena_data, size_t arena_size, size_t block_size)
    : _data(arena_data), _free_list(nullptr), _block_size(block_size), _block_count(arena_size / block_size)
{
    assert(arena_data != nullptr);
    assert(arena_size > 0);
    assert(block_size >= sizeof(FreeBlockHeader));
    assert(block_size >= sizeof(void*));
    assert(_block_count > 0);
    std::memset(_use_bits, 0, use_bits_word_count(_block_count) * sizeof(size_t));
    rebuild_free_list();
}

SmallHeapArena::~SmallHeapArena()
{
    release_data();
}

void* SmallHeapArena::allocate_block()
{
    if (_free_list == nullptr)
    {
        return nullptr;
    }
    FreeBlockHeader* free_block = _free_list;
    _free_list = (FreeBlockHeader*)free_block->next_free;
    free_block->next_free = nullptr;
#if LEANCLR_GC_DEBUG
    LEANCLR_GC_ASSERT(is_zeroed(free_block, _block_size), "free_block is not zeroed");
#endif
    set_block_in_use(slot_index(free_block), true);
    return free_block;
}

bool SmallHeapArena::is_full()
{
    return _free_list == nullptr;
}

bool SmallHeapArena::is_empty() const
{
    const size_t word_count = use_bits_word_count(_block_count);
    for (size_t i = 0; i < word_count; ++i)
    {
        if (_use_bits[i] != 0)
        {
            return false;
        }
    }
    return true;
}

size_t SmallHeapArena::get_block_size() const
{
    return _block_size;
}

size_t SmallHeapArena::sweep(const GCAliveObjectBitmap& alive_object_bitmap)
{
    size_t freed_count = 0;

    for (size_t i = 0; i < _block_count; i++)
    {
        if (is_block_free(i))
        {
            continue;
        }

        vm::RtObject* obj = reinterpret_cast<vm::RtObject*>(block_at(i));
#if LEANCLR_GC_DEBUG
        if (gc_debug_is_quarantined_tombstone(obj))
        {
            continue;
        }
#endif
        if (alive_object_bitmap.is_marked(obj))
        {
            continue;
        }

        LEANCLR_ASSUME((uintptr_t)obj % GC_ALIGN == 0);
        LEANCLR_ASSUME(_block_size % GC_ALIGN == 0);
#if LEANCLR_GC_DEBUG
        gc_debug_quarantine_object(obj, _block_size);
#else
        std::memset(obj, 0, _block_size);
        set_block_in_use(i, false);
#endif
        freed_count++;
    }

    rebuild_free_list();
    return freed_count;
}

SizeClassPool::SizeClassPool(size_t arena_size, size_t block_size, size_t block_alignment)
    : _arena_size(arena_size), _block_size(block_size), _block_alignment(block_alignment), _current_arena(nullptr)
{
}

SmallHeapArena* SizeClassPool::allocate_arena()
{
    assert(_arena_size / _block_size > 0);
    void* arena_data = alloc::GeneralAllocation::aligned_malloc(_arena_size, _block_alignment);
    if (arena_data == nullptr)
    {
        return nullptr;
    }
    std::memset(arena_data, 0, _arena_size);

    size_t mem_size = SmallHeapArena::allocation_size(_arena_size, _block_size);
    void* mem = alloc::GeneralAllocation::malloc(mem_size);
    if (mem == nullptr)
    {
        alloc::GeneralAllocation::aligned_free(arena_data);
        return nullptr;
    }
    return new (mem) SmallHeapArena(arena_data, _arena_size, _block_size);
}

void SizeClassPool::free_arena(SmallHeapArena* arena)
{
    assert(arena != nullptr);
    arena->~SmallHeapArena();
    alloc::GeneralAllocation::free(arena);
}

void* SizeClassPool::allocate_block()
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
        SmallHeapArena* new_arena = allocate_arena();
        if (new_arena == nullptr)
        {
            return nullptr;
        }
        _current_arena = new_arena;
    }
    return _current_arena->allocate_block();
}

void SizeClassPool::sweep(const GCAliveObjectBitmap& alive_object_bitmap, int64_t& freed_bytes)
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
        if (arena->is_empty())
        {
            free_arena(arena);
            continue;
        }
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

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_MARK_SWEEP

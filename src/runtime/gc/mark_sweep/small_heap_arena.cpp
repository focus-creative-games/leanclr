#include "gc/mark_sweep/small_heap_arena.h"

#if LEANCLR_GC_MARK_SWEEP

#include "gc/gc_finalizer.h"
#include "utils/mem_op.h"

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

bool SmallHeapArena::is_block_in_use(size_t index) const
{
    return (_use_bits[index / kBitsPerUseWord] >> (index % kBitsPerUseWord)) & 1;
}

bool SmallHeapArena::is_block_free(size_t index) const
{
    return !is_block_in_use(index);
}

void SmallHeapArena::set_block_in_use(size_t index)
{
    const size_t word_index = index / kBitsPerUseWord;
    const size_t bit_index = index % kBitsPerUseWord;
    const size_t mask = static_cast<size_t>(1) << bit_index;
    assert(word_index < kMaxUseChunkCount);
    const bool word_was_empty = _use_bits[word_index] == 0;
    _use_bits[word_index] |= mask;
    _chunk_in_use_mask |= static_cast<uint64_t>(1) << word_index;
}

void SmallHeapArena::clear_block_in_use(size_t index)
{
    const size_t word_index = index / kBitsPerUseWord;
    const size_t bit_index = index % kBitsPerUseWord;
    const size_t mask = static_cast<size_t>(1) << bit_index;
    assert(word_index < kMaxUseChunkCount);
    _use_bits[word_index] &= ~mask;
    if (_use_bits[word_index] == 0)
    {
        _chunk_in_use_mask &= ~(static_cast<uint64_t>(1) << word_index);
    }
}

void SmallHeapArena::prepend_to_free_list(FreeBlockHeader* block)
{
    block->next_free = _free_list;
    _free_list = block;
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
    _chunk_in_use_mask = 0;
}

SmallHeapArena::SmallHeapArena(void* arena_data, size_t arena_size, size_t block_size)
    : _data(arena_data), _free_list(nullptr), _block_size(block_size), _block_count(arena_size / block_size), _use_bits{}, _chunk_in_use_mask(0)
{
    assert(arena_data != nullptr);
    assert(arena_size > 0);
    assert(block_size >= sizeof(FreeBlockHeader));
    assert(block_size >= sizeof(void*));
    assert(_block_count > 0);
    assert(_block_count <= kMaxBlockCount);
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
    set_block_in_use(slot_index(free_block));
    return free_block;
}

bool SmallHeapArena::is_full()
{
    return _free_list == nullptr;
}

bool SmallHeapArena::is_empty() const
{
    return _chunk_in_use_mask == 0;
}

size_t SmallHeapArena::get_block_size() const
{
    return _block_size;
}

size_t SmallHeapArena::sweep(const GCAliveObjectBitmap& alive_object_bitmap)
{
    size_t freed_count = 0;
    uint64_t chunk_mask = _chunk_in_use_mask;

    while (chunk_mask != 0)
    {
        const size_t w = utils::MemOp::count_trailing_zeros_nonzero64(chunk_mask);
        assert(w < kMaxUseChunkCount);

        size_t word = _use_bits[w];
        while (word != 0)
        {
            const size_t bit_in_word = utils::MemOp::count_trailing_zeros_nonzero(word);
            const size_t i = w * kBitsPerUseWord + bit_in_word;
            if (i >= _block_count)
            {
                break;
            }

            vm::RtObject* obj = reinterpret_cast<vm::RtObject*>(block_at(i));
#if LEANCLR_GC_DEBUG
            if (gc_debug_is_quarantined_tombstone(obj))
            {
                word &= word - 1;
                continue;
            }
#endif
            if (alive_object_bitmap.is_marked(obj))
            {
                word &= word - 1;
                continue;
            }

            LEANCLR_ASSUME((uintptr_t)obj % GC_ALIGN == 0);
            LEANCLR_ASSUME(_block_size % GC_ALIGN == 0);
            GcFinalizer::on_object_freed(obj);
#if LEANCLR_GC_DEBUG
            gc_debug_quarantine_object(obj, _block_size);
#else
            std::memset(obj, 0, _block_size);
            clear_block_in_use(i);
            prepend_to_free_list(reinterpret_cast<FreeBlockHeader*>(obj));
#endif
            freed_count++;
            word &= word - 1;
        }

        chunk_mask &= chunk_mask - 1;
    }

    return freed_count;
}

SizeClassPool::SizeClassPool(size_t arena_size, size_t block_size, size_t block_alignment)
    : _arena_size(arena_size), _block_size(block_size), _block_alignment(block_alignment), _current_arena(nullptr), _next_find_not_full_start(0)
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

    void* mem = alloc::GeneralAllocation::malloc(sizeof(SmallHeapArena));
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
        _current_arena = nullptr;
    }

    _current_arena = find_next_not_full_arena();
    if (LEANCLR_LIKELY(_current_arena != nullptr))
    {
        return _current_arena->allocate_block();
    }

    _current_arena = allocate_arena();
    if (_current_arena == nullptr)
    {
        return nullptr;
    }
    _arenas.push_back(_current_arena);
    _next_find_not_full_start = _arenas.size();
    return _current_arena->allocate_block();
}

SmallHeapArena* SizeClassPool::find_next_not_full_arena()
{
    for (size_t i = _next_find_not_full_start; i < _arenas.size(); ++i)
    {
        if (!_arenas[i]->is_full())
        {
            _next_find_not_full_start = i + 1;
            return _arenas[i];
        }
    }
    _next_find_not_full_start = _arenas.size();
    return nullptr;
}

void SizeClassPool::sweep(const GCAliveObjectBitmap& alive_object_bitmap, int64_t& freed_bytes)
{
    for (size_t i = 0; i < _arenas.size();)
    {
        SmallHeapArena* arena = _arenas[i];
        size_t freed_count = arena->sweep(alive_object_bitmap);
        freed_bytes += static_cast<int64_t>(freed_count * arena->get_block_size());

        if (arena->is_empty())
        {
            free_arena(arena);
            _arenas[i] = _arenas[_arenas.size() - 1];
            _arenas.pop_back();
            continue;
        }

        ++i;
    }

    _next_find_not_full_start = 0;
    _current_arena = find_next_not_full_arena();
}

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_MARK_SWEEP

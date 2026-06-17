#include "gc/mark_sweep/mark_sweep_heap.h"

#if LEANCLR_GC_MARK_SWEEP

#include <cstring>

#include "alloc/general_allocation.h"
#include "gc/gc_config.h"
#include "gc/gc_debug.h"
#include "gc/gc_finalizer.h"
#include "gc/gc_handle_table.h"
#include "gc/gc_roots.h"
#include "gc/mark_sweep/small_heap_arena.h"
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

struct SizeClassPoolInfo
{
    size_t max_size;
    size_t size_increment;
    size_t arena_size;
};

static constexpr SizeClassPoolInfo s_size_class_pool_infos[] = {
    {256, 8, 16 * 1024}, {512, 16, 32 * 1024}, {1024, 32, 64 * 1024}, {2048, 64, 128 * 1024}, {4096, 128, 256 * 1024}, {8192, 256, 512 * 1024},
};

constexpr size_t kSizeClassPoolInfoCount = sizeof(s_size_class_pool_infos) / sizeof(s_size_class_pool_infos[0]);
constexpr size_t kSizeClassPoolCount = kSizeClassPoolInfoCount * 16 + 16 /* pool0 has extra 16 size classes for small objects */;
static SizeClassPool* s_size_class_pools[kSizeClassPoolCount] = {};
constexpr size_t kMaxSmallObejctSize = s_size_class_pool_infos[kSizeClassPoolInfoCount - 1].max_size;
static SizeClassPool* s_map_div8_size_to_pool[kMaxSmallObejctSize / 8] = {};
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
    for (size_t i = 0; i < kSizeClassPoolCount; ++i)
    {
        SizeClassPool* pool = s_size_class_pools[i];
        if (pool != nullptr)
        {
            pool->sweep(alive_object_bitmap, freed_bytes);
        }
    }
}

static void sweep_big_objects(const GCAliveObjectBitmap& alive_object_bitmap, int64_t& freed_bytes)
{
    for (auto it = s_big_object_arenas.begin(); it != s_big_object_arenas.end();)
    {
        vm::RtObject* obj = reinterpret_cast<vm::RtObject*>(*it);
        if (alive_object_bitmap.is_marked(obj))
        {
            ++it;
            continue;
        }
        size_t aligned_size = utils::MemOp::align_up(get_object_allocated_size(obj), GC_ALIGN);
        GcFinalizer::on_object_freed(obj);
#if LEANCLR_GC_DEBUG
        gc_debug_quarantine_object(obj, aligned_size);
#else
        alloc::GeneralAllocation::free(*it);
#endif
        freed_bytes += static_cast<int64_t>(aligned_size);
        it = s_big_object_arenas.erase(it);
    }
}

static void initialize_size_class_pools()
{
    size_t last_pool_max_size = 0;
    size_t last_pool_index = 0;
    size_t last_map_index = 0;
    for (size_t i = 0; i < kSizeClassPoolInfoCount; i++)
    {
        size_t arena_size = s_size_class_pool_infos[i].arena_size;
        size_t max_size = s_size_class_pool_infos[i].max_size;
        size_t size_increment = s_size_class_pool_infos[i].size_increment;
        for (size_t j = 1; j <= (max_size - last_pool_max_size) / size_increment; j++)
        {
            size_t current_block_size = last_pool_max_size + size_increment * j;
            auto new_pool = new SizeClassPool(arena_size, current_block_size, GC_ALIGN);
            for (; last_map_index < current_block_size / 8; last_map_index++)
            {
                s_map_div8_size_to_pool[last_map_index] = new_pool;
            }
            s_size_class_pools[last_pool_index++] = new_pool;
        }
        last_pool_max_size = max_size;
    }
    assert(last_map_index == kMaxSmallObejctSize / 8);
    assert(last_pool_index == kSizeClassPoolCount);
}

static inline SizeClassPool* get_size_class_pool(size_t size)
{
    assert(size > 0 && size % 8 == 0);
    return s_map_div8_size_to_pool[size / 8 - 1];
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
    initialize_size_class_pools();
}

void MarkSweepHeap::collect()
{
    if (s_gc_mode == GCMode::DISABLED)
    {
        return;
    }
    GcPressure::on_collect();
    GcFinalizer::run_pending_finalizers();

    GCAliveObjectBitmap alive_object_bitmap;
    GcRoots::foreach_root(alive_object_bitmap);
    GcFinalizer::promote_unreachable(alive_object_bitmap);

    int64_t freed_bytes = 0;
    sweep_small_objects(alive_object_bitmap, freed_bytes);
    sweep_big_objects(alive_object_bitmap, freed_bytes);
    vm::GCHandle::sweep_weak_handles(is_object_alive, &alive_object_bitmap);

    int64_t old_heap_bytes = s_heap_bytes;
    s_used_bytes -= freed_bytes;
    s_heap_bytes -= freed_bytes;
    s_collection_count++;
    // printf("MarkSweepHeap::collect end, old_heap_bytes: %lld, new_heap_bytes: %lld, freed_bytes: %lld\n", old_heap_bytes, s_heap_bytes, freed_bytes);
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
        obj = (vm::RtObject*)get_size_class_pool(aligned_size)->allocate_block();
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
    GcFinalizer::on_object_allocated(obj);
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

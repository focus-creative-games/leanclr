#include "gc/mark_sweep/mark_sweep_heap.h"

#if LEANCLR_GC_MARK_SWEEP

#include <cstring>

#include "alloc/general_allocation.h"
#include "gc/gc_config.h"
#include "gc/gc_common.h"
#include "gc/gc_scan.h"
#include "gc/handles/gc_handle_table.h"
#include "gc/roots/gc_roots.h"
#include "utils/rt_vector.h"
#include "vm/class.h"

namespace leanclr
{
namespace gc
{

struct HeapChunk
{
    uint8_t* base;
    size_t capacity;
    size_t bump;
    HeapChunk* next;
};

#if LEANCLR_GC_DEBUG
struct QuarantineEntry
{
    GcBlockHeader* header;
    uint8_t* mirror;
    QuarantineEntry* next;
};
#endif

static HeapChunk* s_chunks = nullptr;
static HeapChunk* s_current_chunk = nullptr;
static void* s_free_lists[GC_SOH_MAX_BYTES / GC_ALIGN + 1] = {};
static utils::Vector<vm::RtObject*> s_mark_stack;
static int32_t s_collection_count = 0;
static int64_t s_used_bytes = 0;
static int64_t s_heap_bytes = 0;

#if LEANCLR_GC_DEBUG
static QuarantineEntry* s_quarantine_head = nullptr;
#endif

static uint16_t size_to_class(size_t block_size)
{
    if (block_size > GC_SOH_MAX_BYTES)
    {
        return 0;
    }
    const size_t idx = (block_size + GC_ALIGN - 1) / GC_ALIGN;
    return static_cast<uint16_t>(idx);
}

static size_t class_to_block_size(uint16_t size_class)
{
    if (size_class == 0)
    {
        return 0;
    }
    return static_cast<size_t>(size_class) * GC_ALIGN;
}

static HeapChunk* alloc_chunk(size_t capacity)
{
    HeapChunk* chunk = alloc::GeneralAllocation::malloc_any_zeroed<HeapChunk>();
    if (chunk == nullptr)
    {
        return nullptr;
    }
    chunk->base = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(capacity));
    if (chunk->base == nullptr)
    {
        alloc::GeneralAllocation::free(chunk);
        return nullptr;
    }
    chunk->capacity = capacity;
    chunk->bump = 0;
    chunk->next = s_chunks;
    s_chunks = chunk;
    s_heap_bytes += static_cast<int64_t>(capacity);
    return chunk;
}

static GcBlockHeader* alloc_from_chunk(size_t block_size)
{
    if (s_current_chunk == nullptr || s_current_chunk->bump + block_size > s_current_chunk->capacity)
    {
        s_current_chunk = alloc_chunk(GC_CHUNK_SIZE);
        if (s_current_chunk == nullptr)
        {
            return nullptr;
        }
    }
    GcBlockHeader* header = reinterpret_cast<GcBlockHeader*>(s_current_chunk->base + s_current_chunk->bump);
    s_current_chunk->bump += block_size;
    return header;
}

static GcBlockHeader* alloc_block(size_t block_size, uint16_t size_class, uint16_t flags)
{
    GcBlockHeader* header = nullptr;
    if (size_class != 0)
    {
        void*& freelist = s_free_lists[size_class - 1];
        if (freelist != nullptr)
        {
            header = reinterpret_cast<GcBlockHeader*>(freelist);
            freelist = *reinterpret_cast<void**>(freelist);
        }
    }
    if (header == nullptr)
    {
        header = alloc_from_chunk(block_size);
    }
    if (header == nullptr)
    {
        return nullptr;
    }
    header->block_size = static_cast<uint32_t>(block_size);
    header->flags = flags;
    header->size_class = size_class;
#if LEANCLR_GC_DEBUG
    header->alloc_id = 0;
    header->site_id = 0;
#endif
    std::memset(gc_object_from_header(header), 0, block_size - sizeof(GcBlockHeader));
    s_used_bytes += static_cast<int64_t>(block_size);
    GcPressure::on_alloc(block_size);
    return header;
}

static int64_t s_alloc_id_last = 0;

static int64_t alloc_id_next()
{
    return ++s_alloc_id_last;
}

static vm::RtObject* alloc_object_impl(const metadata::RtClass* klass, size_t size, const GcAllocSite& site, bool has_references)
{
    const size_t block_size = gc_align_size(sizeof(GcBlockHeader) + size);
    const uint16_t size_class = size_to_class(block_size);
    uint16_t flags = has_references ? static_cast<uint16_t>(GC_BLOCK_HAS_REFERENCES) : 0;
    if (size_class == 0)
    {
        flags = static_cast<uint16_t>(flags | GC_BLOCK_LARGE);
    }
    GcBlockHeader* header = alloc_block(block_size, size_class, flags);
    if (header == nullptr)
    {
        return nullptr;
    }
#if LEANCLR_GC_DEBUG
    header->alloc_id = alloc_id_next();
    header->site_id = site.intern_site_id();
#else
    (void)site;
#endif
    vm::RtObject* obj = gc_object_from_header(header);
    obj->klass = const_cast<metadata::RtClass*>(klass);
    obj->__sync_block = nullptr;
    return obj;
}

static void mark_object(vm::RtObject* obj, void* /*userdata*/)
{
    if (obj == nullptr)
    {
        return;
    }
    GcBlockHeader* header = gc_header_from_object(obj);
    if (gc_block_is_marked(header))
    {
        return;
    }
    gc_block_set_marked(header);
    s_mark_stack.push_back(obj);
}

static void drain_mark_stack()
{
    while (!s_mark_stack.empty())
    {
        vm::RtObject* obj = s_mark_stack.back();
        s_mark_stack.pop_back();
        gc_scan_object_refs(obj, mark_object, nullptr);
    }
}

static void mark_root_slot(vm::RtObject** slot, void* /*userdata*/)
{
    if (slot != nullptr && *slot != nullptr)
    {
        mark_object(*slot, nullptr);
    }
}

static void mark_object_root(vm::RtObject* obj, void* /*userdata*/)
{
    if (obj != nullptr)
    {
        mark_object(obj, nullptr);
    }
}

#if LEANCLR_GC_DEBUG
static void move_to_quarantine(GcBlockHeader* header)
{
    QuarantineEntry* entry = alloc::GeneralAllocation::malloc_any_zeroed<QuarantineEntry>();
    if (entry == nullptr)
    {
        return;
    }
    entry->header = header;
    entry->mirror = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(header->block_size));
    if (entry->mirror != nullptr)
    {
        std::memcpy(entry->mirror, header, header->block_size);
    }
    std::memset(header, 0xDD, header->block_size);
    entry->next = s_quarantine_head;
    s_quarantine_head = entry;
}

static void verify_quarantine()
{
    utils::Utf8StringBuilder sb(512);
    for (QuarantineEntry* entry = s_quarantine_head; entry != nullptr; entry = entry->next)
    {
        if (entry->mirror == nullptr)
        {
            continue;
        }
        if (std::memcmp(entry->header, entry->mirror, entry->header->block_size) != 0)
        {
            const GcBlockHeader* h = entry->header;
#if LEANCLR_GC_DEBUG
            const vm::RtObject* obj = gc_object_from_header(const_cast<GcBlockHeader*>(h));
            GcAllocSite::get_site_description(h->site_id, sb);
            sb.sure_null_terminator_but_not_append();
            assert(false && "Quarantine entry mismatch");
            sb.clear();
            (void)obj;
            // TODO: route to leanclr log when available
#endif
        }
    }
}
#endif

static void sweep_chunk(HeapChunk* chunk)
{
    size_t offset = 0;
    while (offset < chunk->bump)
    {
        GcBlockHeader* header = reinterpret_cast<GcBlockHeader*>(chunk->base + offset);
        const size_t block_size = header->block_size;
        if (block_size == 0)
        {
            break;
        }
        if (gc_block_is_marked(header))
        {
            gc_block_clear_marked(header);
        }
        else
        {
            s_used_bytes -= static_cast<int64_t>(block_size);
#if LEANCLR_GC_DEBUG
            move_to_quarantine(header);
#else
            const uint16_t sc = header->size_class;
            if (sc != 0)
            {
                void*& freelist = s_free_lists[sc - 1];
                *reinterpret_cast<void**>(header) = freelist;
                freelist = header;
            }
#endif
        }
        offset += block_size;
    }
}

void MarkSweepHeap::initialize()
{
    GcPressureConfig cfg = {GC_DEFAULT_BYTE_THRESHOLD, GC_DEFAULT_SOFT_HEAP_LIMIT};
    GcPressure::initialize(cfg);
    s_chunks = nullptr;
    s_current_chunk = nullptr;
    std::memset(s_free_lists, 0, sizeof(s_free_lists));
    s_mark_stack.clear();
    s_collection_count = 0;
    s_used_bytes = 0;
    s_heap_bytes = 0;
#if LEANCLR_GC_DEBUG
    s_quarantine_head = nullptr;
#endif
}

bool MarkSweepHeap::is_object_marked(const vm::RtObject* obj)
{
    if (obj == nullptr)
    {
        return false;
    }
    return gc_block_is_marked(gc_header_from_object(obj));
}

void MarkSweepHeap::collect()
{
    // TODO
    return;
    s_mark_stack.clear();
    GcRoots::foreach_root(mark_root_slot, nullptr);
    GcRoots::foreach_object_root(mark_object_root, nullptr);
    GcHandleTable::foreach_strong_and_pinned(mark_root_slot, nullptr);
    drain_mark_stack();

    for (HeapChunk* chunk = s_chunks; chunk != nullptr; chunk = chunk->next)
    {
        sweep_chunk(chunk);
    }

    GcHandleTable::sweep_weak_after_mark();
#if LEANCLR_GC_DEBUG
    verify_quarantine();
#endif
    GcPressure::on_collect();
    gc_pressure_set_used_size(s_used_bytes);
    s_collection_count++;
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

void* MarkSweepHeap::allocate_fixed(size_t size)
{
    return alloc::GeneralAllocation::malloc_zeroed(size);
}

void MarkSweepHeap::free_fixed(void* address)
{
    alloc::GeneralAllocation::free(address);
}

vm::RtObject* MarkSweepHeap::allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
{
    const bool has_refs = vm::Class::get_has_references(klass);
    return alloc_object_impl(klass, size, site, has_refs);
}

vm::RtObject* MarkSweepHeap::allocate_object(const metadata::RtClass* klass, size_t size)
{
    const bool has_refs = vm::Class::get_has_references(klass);
    return alloc_object_impl(klass, size, GcAllocSite::none(), has_refs);
}

// vm::RtObject* MarkSweepHeap::allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
// {
//     return alloc_object_impl(klass, size, site, false);
// }

// vm::RtObject* MarkSweepHeap::allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size)
// {
//     return alloc_object_impl(klass, size, GcAllocSite::none(), false);
// }

vm::RtObject* MarkSweepHeap::allocate_array(const metadata::RtClass* arrClass, size_t totalBytes, const GcAllocSite& site)
{
    const bool has_refs = arrClass->element_class != nullptr && vm::Class::get_has_references(arrClass->element_class);
    return alloc_object_impl(arrClass, totalBytes, site, has_refs);
}

vm::RtObject* MarkSweepHeap::allocate_array(const metadata::RtClass* arrClass, size_t totalBytes)
{
    const bool has_refs = arrClass->element_class != nullptr && vm::Class::get_has_references(arrClass->element_class);
    return alloc_object_impl(arrClass, totalBytes, GcAllocSite::none(), has_refs);
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

void MarkSweepHeap::set_pressure_config(const GcPressureConfig& config)
{
    GcPressure::set_config(config);
}

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_MARK_SWEEP

#if LEANCLR_GC_ZERO_GC

#include "gc/zero_gc/zero_gc_heap.h"

#include <cstring>

#include "alloc/general_allocation.h"
#include "gc/gc_common.h"
#include "gc/gc_config.h"

namespace leanclr
{
namespace gc
{

struct ZeroChunk
{
    uint8_t* base;
    size_t capacity;
    size_t bump;
    ZeroChunk* next;
};

static ZeroChunk* s_chunks = nullptr;
static ZeroChunk* s_current = nullptr;
static int64_t s_used_bytes = 0;
static int64_t s_heap_bytes = 0;

static ZeroChunk* new_chunk(size_t capacity)
{
    ZeroChunk* c = alloc::GeneralAllocation::malloc_any_zeroed<ZeroChunk>();
    if (c == nullptr)
    {
        return nullptr;
    }
    c->base = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(capacity));
    if (c->base == nullptr)
    {
        alloc::GeneralAllocation::free(c);
        return nullptr;
    }
    c->capacity = capacity;
    c->bump = 0;
    c->next = s_chunks;
    s_chunks = c;
    s_heap_bytes += static_cast<int64_t>(capacity);
    return c;
}

static GcBlockHeader* bump_alloc(size_t block_size, const GcAllocSite& site)
{
    if (s_current == nullptr || s_current->bump + block_size > s_current->capacity)
    {
        s_current = new_chunk(GC_CHUNK_SIZE);
        if (s_current == nullptr)
        {
            return nullptr;
        }
    }
    GcBlockHeader* header = reinterpret_cast<GcBlockHeader*>(s_current->base + s_current->bump);
    s_current->bump += block_size;
    s_used_bytes += static_cast<int64_t>(block_size);
    header->block_size = static_cast<uint32_t>(block_size);
    header->flags = 0;
    header->size_class = 0;
#if LEANCLR_GC_DEBUG
    header->alloc_id = gc_alloc_id_next();
    header->site_id = site.intern_site_id();
#else
    (void)site;
#endif
    std::memset(gc_object_from_header(header), 0, block_size - sizeof(GcBlockHeader));
    GcPressure::on_alloc(block_size);
    return header;
}

static vm::RtObject* alloc_impl(const metadata::RtClass* klass, size_t size, const GcAllocSite& site, uint16_t flags)
{
    const size_t block_size = gc_align_size(sizeof(GcBlockHeader) + size);
    GcBlockHeader* header = bump_alloc(block_size, site);
    if (header == nullptr)
    {
        return nullptr;
    }
    header->flags = flags;
#if LEANCLR_GC_DEBUG
    header->site_id = site.intern_site_id();
#else
    (void)site;
#endif
    vm::RtObject* obj = gc_object_from_header(header);
    obj->klass = const_cast<metadata::RtClass*>(klass);
    obj->__sync_block = nullptr;
    return obj;
}

void ZeroGcHeap::initialize()
{
    GcPressureConfig cfg = {GC_DEFAULT_BYTE_THRESHOLD, GC_DEFAULT_SOFT_HEAP_LIMIT};
    GcPressure::initialize(cfg);
    s_chunks = nullptr;
    s_current = nullptr;
    s_used_bytes = 0;
    s_heap_bytes = 0;
}

void ZeroGcHeap::collect()
{
    GcPressure::on_collect();
}

bool ZeroGcHeap::maybe_collect()
{
    return false;
}

bool ZeroGcHeap::should_collect(bool /*force*/)
{
    return false;
}

void* ZeroGcHeap::allocate_fixed(size_t size)
{
    return alloc::GeneralAllocation::malloc_zeroed(size);
}

void ZeroGcHeap::free_fixed(void* address)
{
    alloc::GeneralAllocation::free(address);
}

vm::RtObject* ZeroGcHeap::allocate_object(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
{
    uint16_t flags = vm::Class::get_has_references(klass) ? GC_BLOCK_HAS_REFERENCES : 0;
    return alloc_impl(klass, size, site, flags);
}

vm::RtObject* ZeroGcHeap::allocate_object(const metadata::RtClass* klass, size_t size)
{
    uint16_t flags = vm::Class::get_has_references(klass) ? GC_BLOCK_HAS_REFERENCES : 0;
    return alloc_impl(klass, size, GcAllocSite::none(), flags);
}

// vm::RtObject* ZeroGcHeap::allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size, const GcAllocSite& site)
// {
//     return alloc_impl(klass, size, site, 0);
// }

// vm::RtObject* ZeroGcHeap::allocate_object_not_contains_references(const metadata::RtClass* klass, size_t size)
// {
//     return alloc_impl(klass, size, GcAllocSite::none(), 0);
// }

vm::RtObject* ZeroGcHeap::allocate_array(const metadata::RtClass* arrClass, size_t totalBytes, const GcAllocSite& site)
{
    const bool has_refs = arrClass->element_class != nullptr && vm::Class::get_has_references(arrClass->element_class);
    return alloc_impl(arrClass, totalBytes, site, has_refs ? GC_BLOCK_HAS_REFERENCES : 0);
}

vm::RtObject* ZeroGcHeap::allocate_array(const metadata::RtClass* arrClass, size_t totalBytes)
{
    const bool has_refs = arrClass->element_class != nullptr && vm::Class::get_has_references(arrClass->element_class);
    return alloc_impl(arrClass, totalBytes, GcAllocSite::none(), has_refs ? GC_BLOCK_HAS_REFERENCES : 0);
}

void ZeroGcHeap::write_barrier(vm::RtObject** obj_ref_location, vm::RtObject* new_obj)
{
    if (obj_ref_location != nullptr)
    {
        *obj_ref_location = new_obj;
    }
}

bool ZeroGcHeap::has_strict_wbarriers()
{
    return false;
}

int64_t ZeroGcHeap::get_used_size()
{
    return s_used_bytes;
}

int64_t ZeroGcHeap::get_heap_size()
{
    return s_heap_bytes;
}

int32_t ZeroGcHeap::get_collection_count()
{
    return 0;
}

bool ZeroGcHeap::is_object_marked(const vm::RtObject* /*obj*/)
{
    return true;
}

void ZeroGcHeap::set_pressure_config(const GcPressureConfig& config)
{
    GcPressure::set_config(config);
}

} // namespace gc
} // namespace leanclr

#endif // LEANCLR_GC_ZERO_GC

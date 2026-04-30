#include "mem_pool.h"

namespace leanclr
{
namespace alloc
{

size_t MemPool::s_default_region_size = DEFAULT_REGION_SIZE;

void MemPool::set_default_region_size(size_t size)
{
    s_default_region_size = size;
}

size_t MemPool::get_default_region_size()
{
    return s_default_region_size;
}

MemPool::Region* MemPool::create_region(size_t capacity)
{
    const size_t aligned_capacity = std::max(align_up(capacity, page_size_), region_size_);

    auto* data = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc_zeroed(aligned_capacity));
    if (!data)
    {
        return nullptr;
    }

    auto* reg = static_cast<MemPool::Region*>(alloc::GeneralAllocation::malloc_zeroed(sizeof(MemPool::Region)));
    if (!reg)
    {
        alloc::GeneralAllocation::free(data);
        return nullptr;
    }

    reg->data = data;
    reg->size = aligned_capacity;
    reg->cur = 0;
    reg->next = nullptr;
    return reg;
}

bool MemPool::add_region(size_t capacity)
{
    MemPool::Region* new_region = create_region(capacity);
    if (!new_region)
    {
        return false;
    }
    new_region->next = region_;
    region_ = new_region;
    return true;
}

uint8_t* MemPool::malloc_zeroed(size_t size, size_t alignment)
{
    assert(alignment && (alignment & (alignment - 1)) == 0 && "Alignment must be a power of two");
    assert(size % alignment == 0 && "Size must be multiple of alignment");

    assert(region_ && "No region available in MemPool");

    size_t start_pos = align_up(region_->cur, alignment);

    if (start_pos + size > region_->size)
    {
        if (!add_region(size))
        {
            return nullptr;
        }
    }

    Region* reg = region_;
    start_pos = align_up(region_->cur, alignment);
    uint8_t* ptr = reg->data + start_pos;
    reg->cur = start_pos + size;
    return ptr;
}

uint8_t* MemPool::calloc(size_t count, size_t size, size_t alignment)
{
    assert(alignment && (alignment & (alignment - 1)) == 0 && "Alignment must be a power of two");
    assert(size % alignment == 0 && "Size must be multiple of alignment");
    assert(count == 0 || (count <= SIZE_MAX / size) && "Size overflow in calloc");
    const size_t total = count * size;
    return malloc_zeroed(total, alignment);
}

MemPool::~MemPool()
{
    Region* reg = region_;
    while (reg)
    {
        Region* next = reg->next;
#if LEANCLR_DEBUG
        std::memset(reg->data, 0xDD, reg->size);
#endif
        alloc::GeneralAllocation::free(reg->data);
#if LEANCLR_DEBUG
        std::memset(reg, 0xDD, sizeof(Region));
#endif
        alloc::GeneralAllocation::free(reg);
        reg = next;
    }
}
} // namespace alloc
} // namespace leanclr
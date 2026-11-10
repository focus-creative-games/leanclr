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
    size_t rounded_capacity;
    if (!utils::CheckedSize::align_up(capacity, page_size_, rounded_capacity))
        return nullptr;
    const size_t aligned_capacity = std::max(rounded_capacity, region_size_);

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
    if (alignment == 0 || (alignment & (alignment - 1)) != 0 || size % alignment != 0)
        return nullptr;
    size_t required_capacity;
    if (!utils::CheckedSize::add(size, alignment - 1, required_capacity))
        return nullptr;

    size_t start_pos = 0;
    // Align the absolute address: an ordinary allocator need not align region
    // bases to the caller's requested (possibly extended) alignment.
    auto fits = [&]()
    {
        if (!region_)
            return false;
        const size_t base = reinterpret_cast<uintptr_t>(region_->data);
        size_t address, aligned_address;
        if (!utils::CheckedSize::add(base, region_->cur, address) || !utils::CheckedSize::align_up(address, alignment, aligned_address))
            return false;
        start_pos = aligned_address - base;
        return start_pos <= region_->size && size <= region_->size - start_pos;
    };
    if (!fits())
    {
        if (!add_region(required_capacity) || !fits())
            return nullptr;
    }
    Region* reg = region_;
    uint8_t* ptr = reg->data + start_pos;
    reg->cur = start_pos + size;
    return ptr;
}

uint8_t* MemPool::calloc(size_t count, size_t size, size_t alignment)
{
    if (alignment == 0 || (alignment & (alignment - 1)) != 0 || size % alignment != 0)
        return nullptr;
    size_t total;
    if (!utils::CheckedSize::multiply(count, size, total))
        return nullptr;
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

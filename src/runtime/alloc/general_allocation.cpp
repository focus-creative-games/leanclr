#include "general_allocation.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#if LEANCLR_PLATFORM_WIN
#include <malloc.h>
#elif LEANCLR_USE_POSIX_ALIGNED_HEAP
#include <cstdlib>
#if defined(__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#endif
namespace leanclr
{
namespace alloc
{
namespace
{
struct AlignedAllocHeader
{
    void* raw_ptr;
    size_t requested_size;
};

static inline bool is_power_of_two(size_t v)
{
    return v != 0 && (v & (v - 1)) == 0;
}

// ---------------------------------------------------------------------------
// Generic aligned heap (header + std::malloc). Used on WASM, musl Linux,
// and any target without first-class aligned realloc / usable size.
// ---------------------------------------------------------------------------
static void* generic_aligned_malloc(size_t size, size_t alignment)
{
    if (alignment < alignof(void*))
    {
        alignment = alignof(void*);
    }
    assert(is_power_of_two(alignment) && "alignment must be a non-zero power of two");
    const size_t overhead = sizeof(AlignedAllocHeader) + (alignment - 1);
    void* raw = std::malloc(size + overhead);
    if (!raw)
    {
        return nullptr;
    }
    uintptr_t p = reinterpret_cast<uintptr_t>(raw) + sizeof(AlignedAllocHeader);
    uintptr_t aligned = (p + (alignment - 1)) & ~(static_cast<uintptr_t>(alignment - 1));
    auto* header = reinterpret_cast<AlignedAllocHeader*>(aligned - sizeof(AlignedAllocHeader));
    header->raw_ptr = raw;
    header->requested_size = size;
    return reinterpret_cast<void*>(aligned);
}

static void generic_aligned_free(void* ptr)
{
    if (!ptr)
    {
        return;
    }
    uintptr_t aligned = reinterpret_cast<uintptr_t>(ptr);
    auto* header = reinterpret_cast<AlignedAllocHeader*>(aligned - sizeof(AlignedAllocHeader));
    std::free(header->raw_ptr);
}

static void* generic_aligned_realloc(void* ptr, size_t size, size_t alignment)
{
    if (ptr == nullptr)
    {
        return generic_aligned_malloc(size, alignment);
    }
    if (size == 0)
    {
        generic_aligned_free(ptr);
        return nullptr;
    }
    uintptr_t aligned = reinterpret_cast<uintptr_t>(ptr);
    auto* old_header = reinterpret_cast<AlignedAllocHeader*>(aligned - sizeof(AlignedAllocHeader));
    const size_t old_size = old_header->requested_size;
    void* new_ptr = generic_aligned_malloc(size, alignment);
    if (!new_ptr)
    {
        return nullptr;
    }
    std::memcpy(new_ptr, ptr, std::min(old_size, size));
    generic_aligned_free(ptr);
    return new_ptr;
}

static void* default_malloc(size_t size)
{
    return std::malloc(size);
}

static void default_free(void* ptr)
{
    std::free(ptr);
}

static void* default_calloc(size_t nmemb, size_t size)
{
    return std::calloc(nmemb, size);
}

static void* default_realloc(void* ptr, size_t size)
{
    return std::realloc(ptr, size);
}

#if LEANCLR_PLATFORM_WIN
static void* default_aligned_malloc(size_t size, size_t alignment)
{
    if (alignment < alignof(void*))
    {
        alignment = alignof(void*);
    }
    assert(is_power_of_two(alignment) && "alignment must be a non-zero power of two");
    return _aligned_malloc(size, alignment);
}

static void default_aligned_free(void* ptr)
{
    if (!ptr)
    {
        return;
    }
    _aligned_free(ptr);
}

static void* default_aligned_realloc(void* ptr, size_t size, size_t alignment)
{
    if (alignment < alignof(void*))
    {
        alignment = alignof(void*);
    }
    assert(is_power_of_two(alignment) && "alignment must be a non-zero power of two");
    return _aligned_realloc(ptr, size, alignment);
}
#elif LEANCLR_USE_POSIX_ALIGNED_HEAP
static void* default_aligned_malloc(size_t size, size_t alignment)
{
    if (alignment < alignof(void*))
    {
        alignment = alignof(void*);
    }
    assert(is_power_of_two(alignment) && "alignment must be a non-zero power of two");
    void* p = nullptr;
    if (posix_memalign(&p, alignment, size) != 0)
    {
        return nullptr;
    }
    return p;
}

static void default_aligned_free(void* ptr)
{
    std::free(ptr);
}

static size_t posix_allocated_byte_length(const void* ptr)
{
#if defined(__APPLE__)
    return malloc_size(ptr);
#else
    return static_cast<size_t>(malloc_usable_size(const_cast<void*>(ptr)));
#endif
}

static void* default_aligned_realloc(void* ptr, size_t size, size_t alignment)
{
    if (ptr == nullptr)
    {
        return default_aligned_malloc(size, alignment);
    }
    if (size == 0)
    {
        default_aligned_free(ptr);
        return nullptr;
    }
    if (alignment < alignof(void*))
    {
        alignment = alignof(void*);
    }
    assert(is_power_of_two(alignment) && "alignment must be a non-zero power of two");
    void* new_ptr = default_aligned_malloc(size, alignment);
    if (!new_ptr)
    {
        return nullptr;
    }
    const size_t old_len = posix_allocated_byte_length(ptr);
    std::memcpy(new_ptr, ptr, std::min(old_len, size));
    default_aligned_free(ptr);
    return new_ptr;
}
#else
static void* default_aligned_malloc(size_t size, size_t alignment)
{
    return generic_aligned_malloc(size, alignment);
}

static void default_aligned_free(void* ptr)
{
    generic_aligned_free(ptr);
}

static void* default_aligned_realloc(void* ptr, size_t size, size_t alignment)
{
    return generic_aligned_realloc(ptr, size, alignment);
}
#endif
} // namespace

MemoryCallbacks GeneralAllocation::s_memory_callbacks = {default_malloc, default_aligned_malloc, default_free,           default_aligned_free,
                                                         default_calloc, default_realloc,        default_aligned_realloc};
} // namespace alloc
} // namespace leanclr

#pragma once

#include "core/rt_base.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace metadata
{
struct RtClass;
}

namespace gc
{

// Default heap chunk size (bytes).
constexpr size_t GC_CHUNK_SIZE = 1024 * 1024;

// Objects larger than this use the large-object path (exact size blocks).
constexpr size_t GC_SOH_MAX_BYTES = 256;

// Default allocation pressure before maybe_collect() triggers a collection.
constexpr uint64_t GC_DEFAULT_BYTE_THRESHOLD = 512 * 1024;

// 0 disables soft heap limit check.
constexpr uint64_t GC_DEFAULT_SOFT_HEAP_LIMIT = 0;

constexpr size_t GC_ALIGN = 8; //PTR_SIZE * 2;

inline size_t gc_align_size(size_t size)
{
    return (size + GC_ALIGN - 1) & ~(GC_ALIGN - 1);
}

inline size_t gc_object_size(const metadata::RtClass* klass)
{
    return sizeof(vm::RtObject) + klass->instance_size_without_header;
}

} // namespace gc
} // namespace leanclr

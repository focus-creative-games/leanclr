#pragma once

#include "build_config.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace gc
{

#if LEANCLR_GC_DEBUG
// True if a previous sweep already quarantined this object (klass cleared, memory retained).
bool gc_debug_is_quarantined_tombstone(const vm::RtObject* obj);

// Copy object bytes to a snapshot, zero the original slot, point __sync_block at the snapshot.
void gc_debug_quarantine_object(vm::RtObject* obj, size_t size);
#endif

} // namespace gc
} // namespace leanclr

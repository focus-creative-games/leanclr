#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace gc
{

typedef void (*GcMarkFn)(vm::RtObject* obj, void* userdata);

void gc_scan_object_refs(vm::RtObject* obj, GcMarkFn mark_fn, void* userdata);

} // namespace gc
} // namespace leanclr

#pragma once

#include "metadata/rt_metadata.h"

namespace leanclr
{
namespace gc
{

void setup_class_gc_bitmap(metadata::RtClass* klass);

} // namespace gc
} // namespace leanclr

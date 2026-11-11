#pragma once
#include "metadata/rt_metadata.h"
void verify_generic_registry(const leanclr::metadata::RtMethodInfo* primitive, const leanclr::metadata::RtMethodInfo* external,
                             const leanclr::metadata::RtMethodInfo* missing, bool benchmark, unsigned iterations);

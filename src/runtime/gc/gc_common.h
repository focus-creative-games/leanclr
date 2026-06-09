#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace gc
{

#if LEANCLR_GC_DEBUG
#define LEANCLR_GC_ASSERT(expr, msg) \
    do                               \
    {                                \
        assert(expr);         \
        if (!(expr))                 \
            panic(msg);              \
    } while (0)
#else
#define LEANCLR_GC_ASSERT(expr, msg) assert(expr)
#endif

enum class GCMode : int32_t
{
    DISABLED = 0,
    ENABLED = 1,
    MANUAL = 2
};

} // namespace gc
} // namespace leanclr

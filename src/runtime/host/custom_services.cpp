#include "default_services.h"
#include "default_allocation.h"

namespace leanclr
{
namespace host
{
ServiceCallbacks make_default_services()
{
    return {};
}

alloc::MemoryCallbacks make_default_memory_callbacks()
{
    return {};
}
} // namespace host
} // namespace leanclr

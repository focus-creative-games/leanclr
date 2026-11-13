#pragma once
#include "core/rt_base.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace bcl
{
// .NET 10 adapter contract. The token is opaque; it is never a managed/native object pointer.
class AssemblyLoadContext
{
  public:
    static RtResult<void*> initialize(void* managed_handle, bool is_default, bool collectible) noexcept;
    static RtResult<metadata::RtAssembly*> load(void* context, const uint8_t* image, int32_t image_size, const uint8_t* symbols, int32_t symbols_size) noexcept;
};
} // namespace bcl
} // namespace leanclr

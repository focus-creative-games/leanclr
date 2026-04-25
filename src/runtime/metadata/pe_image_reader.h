#pragma once

#include "cli_image.h"
#include "core/rt_base.h"

#include <cstddef>
#include <cstdint>

namespace leanclr
{
namespace utils
{
class MemPool;
}

namespace metadata
{

class PeImageReader
{
  private:
    uint8_t* _image_base;
    size_t _image_size;

  public:
    PeImageReader(uint8_t* image_base, size_t image_size) : _image_base(image_base), _image_size(image_size)
    {
    }

    RtResult<CliImage*> ReadCliImage(alloc::MemPool& imagePrivatePool);
};
} // namespace metadata
} // namespace leanclr
